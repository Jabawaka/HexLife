#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#define SCREEN_WIDTH_PX  (1024)
#define SCREEN_HEIGHT_PX  (768)

/* Screen is 64x48 cells */
#define GRID_WIDTH_CELLS    (72)
#define GRID_HEIGHT_CELLS   (40)
#define GRID_X_POSITION_PX (160)
#define GRID_Y_POSITION_PX (160)
#define GRID_ALIVE           (1)
#define GRID_DEAD            (0)

#define GRID_MIN_NEIGHBOURS_SURVIVE  (2)
#define GRID_MAX_NEIGHBOURS_SURVIVE  (3)
#define GRID_NEIGHBOURS_BORN         (3)

void resetGrid(uint8_t *p_grid1, uint8_t *p_grid2)
{
    int iRow, iCol;

    printf("Resetting grid\n");
    for (iRow = 0; iRow < GRID_HEIGHT_CELLS; iRow++)
    {
        for (iCol = 0; iCol < GRID_WIDTH_CELLS; iCol++)
        {
            p_grid1[iRow * GRID_WIDTH_CELLS + iCol] = rand() % 2;
            p_grid2[iRow * GRID_WIDTH_CELLS + iCol] = rand() % 2;
        }
    }
}

int main(int argc, char *argv[])
{
    /* ------ DECLARATION ------ */
    /* Variables for rendering */
    SDL_Window *p_window = NULL;
    SDL_Surface *p_surface = NULL;
    SDL_Renderer *p_renderer = NULL;

    SDL_Texture *p_hexTex = NULL;
    SDL_Surface *p_hexSurf = NULL;

    SDL_Rect renderRect;
    renderRect.w = 32;
    renderRect.h = 31;

    /* App control */
    bool quit = false;
    bool pause = true;
    SDL_Event event;
    Uint64 currTime_ms;
    Uint64 ellapsedTime_ms;
    Uint64 lastRenderTime_ms;

    /* Grid */
    uint8_t grid1[GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS];
    uint8_t grid2[GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS];

    uint8_t *p_displayGrid;
    uint8_t *p_nextGrid;

    int iRow;
    int iCol;

    int prevRow, nextRow;
    int prevCol, nextCol;

    bool randAlive = false;
    uint8_t aliveNeighbours = 0;

    /* ------ INITIALISATION ------ */
    /* Initialise systems */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initialise SDL\n");
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) < 0)
    {
        printf("Could not initialise SDL_Image\n");
        return 1;
    }
    if (TTF_Init() < 0)
    {
        printf("Could not initialise SDL_TTF\n");
        return 1;
    }

    /* Open window */
    p_window = SDL_CreateWindow
       ("HexLife",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (p_window == NULL)
    {
        printf("Could not create window\n");
        return 1;
    }

    p_renderer = SDL_CreateRenderer(p_window, -1, SDL_RENDERER_ACCELERATED);
    if (p_renderer == NULL)
    {
        printf("Could not create renderer\n");
        return 1;
    }
    SDL_SetRenderDrawColor(p_renderer, 0x09, 0x09, 0x09, 0xFF);

    p_surface = SDL_GetWindowSurface(p_window);

    /* Load Hex */
    p_hexSurf = IMG_Load("assets/hex.png");
    if (p_hexSurf == NULL)
    {
        printf("Could not load hex image\n");
        return 1;
    }

    p_hexTex = SDL_CreateTextureFromSurface(p_renderer, p_hexSurf);
    if (p_hexTex == NULL)
    {
        printf("Could not convert hex into texture\n");
        return 1;
    }
    SDL_FreeSurface(p_hexSurf);

    /* Start grid */
    srand(time(NULL));

    resetGrid(grid1, grid2);
    p_displayGrid = grid1;
    p_nextGrid = grid2;

    /* ------ MAIN LOOP ------ */
    lastRenderTime_ms = SDL_GetTicks();
    while (!quit)
    {
        /* Read input */
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_SPACE:
                        pause = !pause;
                        break;
                    case SDLK_r:
                        resetGrid(grid1, grid2);
                        p_displayGrid = grid1;
                        p_nextGrid = grid2;
                        pause = true;
                    default:
                        printf("Key untreated\n");
                }
            }
        }

        /* Update grid */
        for (iRow = 0; iRow < GRID_HEIGHT_CELLS; iRow++)
        {
            prevRow = iRow - 1;
            if (prevRow < 0)
            {
                prevRow += GRID_HEIGHT_CELLS;
            }
            nextRow = iRow + 1;
            if (nextRow >= GRID_HEIGHT_CELLS)
            {
                nextRow -= GRID_HEIGHT_CELLS;
            }

            for (iCol = 0; iCol < GRID_WIDTH_CELLS; iCol += 2)
            {
                prevCol = iCol - 1;
                if (prevCol < 0)
                {
                    prevCol += GRID_WIDTH_CELLS;
                }
                nextCol = iCol + 1;
                if (nextCol >= GRID_WIDTH_CELLS)
                {
                    nextCol -= GRID_WIDTH_CELLS;
                }

                /* Count alive neighbours */
                aliveNeighbours =
                    p_displayGrid[iRow    * GRID_WIDTH_CELLS + prevCol] +
                    p_displayGrid[iRow    * GRID_WIDTH_CELLS + iCol]    +
                    p_displayGrid[iRow    * GRID_WIDTH_CELLS + nextCol] +
                    p_displayGrid[nextRow * GRID_WIDTH_CELLS + prevCol] +
                    p_displayGrid[nextRow * GRID_WIDTH_CELLS + iCol]    +
                    p_displayGrid[nextRow * GRID_WIDTH_CELLS + nextCol];

                if (p_displayGrid[iRow * GRID_WIDTH_CELLS + iCol] == GRID_ALIVE)
                {
                    if ( aliveNeighbours < GRID_MIN_NEIGHBOURS_SURVIVE
                      || aliveNeighbours > GRID_MAX_NEIGHBOURS_SURVIVE)
                    {
                        p_nextGrid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_DEAD;
                    }
                    else
                    {
                        p_nextGrid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_ALIVE;
                    }
                }
                else
                {
                    if (aliveNeighbours == GRID_NEIGHBOURS_BORN)
                    {
                        p_nextGrid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_ALIVE;
                    }
                }
            }

            for (iCol = 1; iCol < GRID_WIDTH_CELLS; iCol += 2)
            {
                prevCol = iCol - 1;
                if (prevCol < 0)
                {
                    prevCol += GRID_WIDTH_CELLS;
                }
                nextCol = iCol + 1;
                if (nextCol >= GRID_WIDTH_CELLS)
                {
                    nextCol -= GRID_WIDTH_CELLS;
                }

                /* Count alive neighbours */
                aliveNeighbours =
                    p_displayGrid[prevRow * GRID_WIDTH_CELLS + prevCol] +
                    p_displayGrid[prevRow * GRID_WIDTH_CELLS + iCol]    +
                    p_displayGrid[prevRow * GRID_WIDTH_CELLS + nextCol] +
                    p_displayGrid[iRow    * GRID_WIDTH_CELLS + prevCol] +
                    p_displayGrid[iRow    * GRID_WIDTH_CELLS + iCol]    +
                    p_displayGrid[iRow    * GRID_WIDTH_CELLS + nextCol];

                if (p_displayGrid[iRow * GRID_WIDTH_CELLS + iCol] == GRID_ALIVE)
                {
                    if ( aliveNeighbours < GRID_MIN_NEIGHBOURS_SURVIVE
                      || aliveNeighbours > GRID_MAX_NEIGHBOURS_SURVIVE)
                    {
                        p_nextGrid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_DEAD;
                    }
                    else
                    {
                        p_nextGrid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_ALIVE;
                    }
                }
                else
                {
                    if (aliveNeighbours == GRID_NEIGHBOURS_BORN)
                    {
                        p_nextGrid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_ALIVE;
                    }
                }
            }
        }

        /* Swap grids */
        if (p_displayGrid == grid1)
        {
            p_displayGrid = grid2;
            p_nextGrid = grid1;
        }
        else
        {
            p_displayGrid = grid1;
            p_nextGrid = grid2;
        }

        /* Render grid */
        SDL_RenderClear(p_renderer);

        //Render texture to screen
        renderRect.x = GRID_X_POSITION_PX;
        renderRect.y = GRID_Y_POSITION_PX;
        for (iRow = 0; iRow < GRID_HEIGHT_CELLS; iRow++)
        {
            for (iCol = 0; iCol < GRID_WIDTH_CELLS; iCol++)
            {
                if (p_displayGrid[iRow * GRID_WIDTH_CELLS + iCol] == GRID_ALIVE)
                {
                    SDL_RenderCopy(p_renderer, p_hexTex, NULL, &renderRect);
                }

                renderRect.x += 24;
                if (iCol % 2 == 0)
                {
                    renderRect.y -= 16;
                }
                else
                {
                    renderRect.y += 16;
                }
            }

            renderRect.x = GRID_X_POSITION_PX;
            renderRect.y += 32;
        }

        /* Update screen */
        SDL_RenderPresent(p_renderer);

        /* Time */
        currTime_ms = SDL_GetTicks();
        ellapsedTime_ms = currTime_ms - lastRenderTime_ms;
        lastRenderTime_ms = currTime_ms;

        if (ellapsedTime_ms < 33)
        {
            SDL_Delay(33 - ellapsedTime_ms);
        }
    }

    /* ------ CLEAN UP ----- */
    /* Destroy window */
    SDL_DestroyWindow(p_window);
    p_window = NULL;

    SDL_DestroyTexture(p_hexTex);

    /* Quit SDL subsystems */
    SDL_Quit();

    return 0;
}