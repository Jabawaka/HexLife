#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#define SCREEN_WIDTH_PX  (1024)
#define SCREEN_HEIGHT_PX  (768)

#define TRUE  (1)
#define FALSE (0)

/* Screen is 64x48 cells */
#define GRID_WIDTH_CELLS    (72)
#define GRID_HEIGHT_CELLS   (40)
#define GRID_ALIVE           (1)
#define GRID_DEAD            (0)

#define GRID_X_POSITION_PX   (160)
#define GRID_Y_POSITION_PX   (200)
#define GRID_X_STEP_PX        (24)
#define GRID_Y_STEP_PX        (32)
#define GRID_Y_OFFSET_ROW_PX  (16)

#define GRID_MIN_NEIGHBOURS_SURVIVE  (2)
#define GRID_MAX_NEIGHBOURS_SURVIVE  (4)
#define GRID_NEIGHBOURS_BORN         (3)

#define GRID_UPDATE_RATE_MS  (100)

void resetGrid(uint8_t *p_grid)
{
    int iRow, iCol;

    for (iRow = 0; iRow < GRID_HEIGHT_CELLS; iRow++)
    {
        for (iCol = 0; iCol < GRID_WIDTH_CELLS; iCol++)
        {
            if (rand() % 3 == 0)
            {
                p_grid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_ALIVE;
            }
            else
            {
                p_grid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_DEAD;
            }
        }
    }
}

void clearGrid(uint8_t *p_grid)
{
    int iCell;

    for (iCell = 0; iCell < GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS; iCell++)
    {
        p_grid[iCell] = GRID_DEAD;
    }
}

void changeCell(int mouse_xpos_px, int mouse_ypos_px, uint8_t *p_grid)
{
    mouse_xpos_px = 2 * mouse_xpos_px;
    mouse_ypos_px = 2 * mouse_ypos_px;

    int colCell = (mouse_xpos_px  - GRID_X_POSITION_PX) / GRID_X_STEP_PX;
    int rowCell = (mouse_ypos_px - GRID_Y_POSITION_PX) / GRID_Y_STEP_PX;

    if (p_grid[rowCell * GRID_WIDTH_CELLS + colCell] == GRID_ALIVE)
    {
        p_grid[rowCell * GRID_WIDTH_CELLS + colCell] = GRID_DEAD;
    }
    else
    {
        p_grid[rowCell * GRID_WIDTH_CELLS + colCell] = GRID_ALIVE;
    }
}

int main(int argc, char *argv[])
{
    /* ------ DECLARATION ------ */
    /* Variables for rendering */
    SDL_Window *p_window = NULL;
    SDL_Renderer *p_renderer = NULL;

    SDL_Texture *p_hexTex = NULL;
    SDL_Surface *p_miscSurf = NULL;

    SDL_Rect renderRect;
    renderRect.w = 32;
    renderRect.h = 31;

    TTF_Font *p_font = NULL;
    SDL_Texture *p_pausedTex = NULL;
    SDL_Texture *p_rulesTex = NULL;
    SDL_Rect pausedRect = { 180, 96, 0, 0 };
    SDL_Rect rulesRect = { 850, 96, 0, 0 };

    SDL_Color textColor = { 0xF7, 0xF7, 0xF7, 0xFF };

    char rulesString[32];

    /* App control */
    SDL_Event event;
    Uint64 currTime_ms;
    Uint64 ellapsedTime_ms;
    Uint64 lastRenderTime_ms;

    int quit = FALSE;
    int pause = TRUE;
    int mouse_xpos_px, mouse_ypos_px;

    int gridUpdate = FALSE;

    /* Grid */
    uint8_t grid1[GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS];
    uint8_t grid2[GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS];

    uint8_t *p_displayGrid;
    uint8_t *p_nextGrid;

    int iRow;
    int iCol;

    int prevRow, nextRow;
    int prevCol, nextCol;

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
    SDL_SetRenderDrawColor(p_renderer, 0x0D, 0x0D, 0x0D, 0xFF);

    /* Load Hex */
    p_miscSurf = IMG_Load("assets/hex.png");
    if (p_miscSurf == NULL)
    {
        printf("Could not load hex image\n");
        return 1;
    }

    p_hexTex = SDL_CreateTextureFromSurface(p_renderer, p_miscSurf);
    if (p_hexTex == NULL)
    {
        printf("Could not convert hex into texture\n");
        return 1;
    }
    SDL_FreeSurface(p_miscSurf);

    /* Start grid */
    srand(time(NULL));

    p_displayGrid = grid1;
    p_nextGrid = grid2;
    resetGrid(p_displayGrid);

    /* Load font */
    p_font = TTF_OpenFont("assets/monaco.ttf", 32);
    if (p_font == NULL)
    {
        printf("Could not load font\n");
        return 1;
    }

    /* ------ MAIN LOOP ------ */
    lastRenderTime_ms = SDL_GetTicks();
    ellapsedTime_ms = 0;
    while (quit != TRUE)
    {
        /* Read input */
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = TRUE;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = TRUE;
                        break;

                    case SDLK_SPACE:
                        pause = !pause;
                        break;

                    case SDLK_r:
                        p_displayGrid = grid1;
                        p_nextGrid = grid2;
                        resetGrid(p_displayGrid);
                        pause = TRUE;
                        break;

                    case SDLK_c:
                        p_displayGrid = grid1;
                        p_nextGrid = grid2;
                        clearGrid(p_displayGrid);
                        pause = TRUE;
                        break;
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                SDL_GetMouseState(&mouse_xpos_px, &mouse_ypos_px);
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                changeCell(mouse_xpos_px, mouse_ypos_px, p_displayGrid);
            }
        }

        /* Handle pause */
        if (pause == TRUE)
        {
            gridUpdate = FALSE;
        }

        /* Update grid */
        if (gridUpdate == TRUE)
        {
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

                for (iCol = 0; iCol < GRID_WIDTH_CELLS; iCol++)
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
                    if (iCol % 2 == 0)
                    {
                        aliveNeighbours =
                            p_displayGrid[iRow    * GRID_WIDTH_CELLS + prevCol] +
                            p_displayGrid[prevRow * GRID_WIDTH_CELLS + iCol]    +
                            p_displayGrid[iRow    * GRID_WIDTH_CELLS + nextCol] +
                            p_displayGrid[nextRow * GRID_WIDTH_CELLS + prevCol] +
                            p_displayGrid[nextRow * GRID_WIDTH_CELLS + iCol]    +
                            p_displayGrid[nextRow * GRID_WIDTH_CELLS + nextCol];
                    }
                    else
                    {
                        aliveNeighbours =
                            p_displayGrid[prevRow * GRID_WIDTH_CELLS + prevCol] +
                            p_displayGrid[prevRow * GRID_WIDTH_CELLS + iCol]    +
                            p_displayGrid[prevRow * GRID_WIDTH_CELLS + nextCol] +
                            p_displayGrid[iRow    * GRID_WIDTH_CELLS + prevCol] +
                            p_displayGrid[nextRow * GRID_WIDTH_CELLS + iCol]    +
                            p_displayGrid[iRow    * GRID_WIDTH_CELLS + nextCol];
                    }

                    /* Update next grid */
                    p_nextGrid[iRow * GRID_WIDTH_CELLS + iCol] = p_displayGrid[iRow * GRID_WIDTH_CELLS + iCol];
                    if (p_displayGrid[iRow * GRID_WIDTH_CELLS + iCol] == GRID_ALIVE)
                    {
                        if ( aliveNeighbours < GRID_MIN_NEIGHBOURS_SURVIVE
                          || aliveNeighbours > GRID_MAX_NEIGHBOURS_SURVIVE)
                        {
                            p_nextGrid[iRow * GRID_WIDTH_CELLS + iCol] = GRID_DEAD;
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
        }

        /* ------ RENDER ------ */
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

                renderRect.x += GRID_X_STEP_PX;
                if (iCol % 2 == 0)
                {
                    renderRect.y -= GRID_Y_OFFSET_ROW_PX;
                }
                else
                {
                    renderRect.y += GRID_Y_OFFSET_ROW_PX;
                }
            }

            renderRect.x = GRID_X_POSITION_PX;
            renderRect.y += GRID_Y_STEP_PX;
        }

        /* Render text */
        p_miscSurf = NULL;
        if (pause)
        {
            p_miscSurf = TTF_RenderText_Solid(p_font, "Paused", textColor);
        }
        else
        {
            p_miscSurf = TTF_RenderText_Solid(p_font, "Running", textColor);
        }

        if (p_miscSurf != NULL)
        {
            p_pausedTex = SDL_CreateTextureFromSurface(p_renderer, p_miscSurf);
            if (p_pausedTex != NULL)
            {
                pausedRect.w = p_miscSurf->w;
                pausedRect.h = p_miscSurf->h;
                SDL_RenderCopy(p_renderer, p_pausedTex, NULL, &pausedRect);
            }
        }
        SDL_FreeSurface(p_miscSurf);

        snprintf(rulesString, 32, "Live: %d-%d, Birth: %d", GRID_MIN_NEIGHBOURS_SURVIVE, GRID_MAX_NEIGHBOURS_SURVIVE, GRID_NEIGHBOURS_BORN);
        p_miscSurf = TTF_RenderText_Solid(p_font, rulesString, textColor);
        if (p_miscSurf != NULL)
        {
            p_rulesTex = SDL_CreateTextureFromSurface(p_renderer, p_miscSurf);
            if (p_rulesTex != NULL)
            {
                rulesRect.w = p_miscSurf->w;
                rulesRect.h = p_miscSurf->h;
                SDL_RenderCopy(p_renderer, p_rulesTex, NULL, &rulesRect);
            }
        }
        SDL_FreeSurface(p_miscSurf);

        /* Update screen */
        SDL_RenderPresent(p_renderer);

        /* Time */
        currTime_ms = SDL_GetTicks();
        ellapsedTime_ms = ellapsedTime_ms + currTime_ms - lastRenderTime_ms;
        lastRenderTime_ms = currTime_ms;

        if (ellapsedTime_ms > GRID_UPDATE_RATE_MS)
        {
            ellapsedTime_ms -= GRID_UPDATE_RATE_MS;
            gridUpdate = TRUE;
        }
        else
        {
            gridUpdate = FALSE;
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