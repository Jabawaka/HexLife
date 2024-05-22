#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "grid.h"

#define SCREEN_WIDTH_PX   (1000)
#define SCREEN_HEIGHT_PX  (1000)

#define TRUE  (1)
#define FALSE (0)

#define GRID_WIDTH_CELLS   (100)
#define GRID_HEIGHT_CELLS  (100)

#define GRID_MIN_NEIGHBOURS_SURVIVE  (2)
#define GRID_MAX_NEIGHBOURS_SURVIVE  (4)
#define GRID_MIN_NEIGHBOURS_CREATE   (3)
#define GRID_MAX_NEIGHBOURS_CREATE   (3)

#define GRID_UPDATE_RATE_MS  (100)


SDL_Texture *loadTexture(char *pathToSprite, SDL_Renderer *p_renderer)
{
    SDL_Surface *p_miscSurf = IMG_Load(pathToSprite);
    if (p_miscSurf == NULL)
    {
        printf("Could not load %s\n", pathToSprite);
        return NULL;
    }

    SDL_Texture *p_tex = SDL_CreateTextureFromSurface(p_renderer, p_miscSurf);
    if (p_tex == NULL)
    {
        printf("Could not convert %s into texture\n", pathToSprite);
        return NULL;
    }

    SDL_FreeSurface(p_miscSurf);

    return p_tex;
}

int main(int argc, char *argv[])
{
    /* ------ DECLARATION ------ */
    /* Detect difference in pixels vs window size */
    int win_width_pnt, win_height_pnt;
    int gl_width_px, gl_height_px;
    float scaleFactor_width_pntToPx = 1.0;
    float scaleFactor_height_pntToPx = 1.0;

    /* Variables for rendering */
    SDL_Window *p_window = NULL;
    SDL_Renderer *p_renderer = NULL;

    SDL_Texture *p_hexTex = NULL;
    SDL_Texture *p_sikTex = NULL;
    SDL_Texture *p_fixTex = NULL;
    SDL_Surface *p_miscSurf = NULL;

    SDL_Rect renderRect;
    renderRect.w = GRID_CELL_WIDTH;
    renderRect.h = GRID_CELL_HEIGHT;

    TTF_Font *p_font = NULL;
    SDL_Texture *p_pausedTex = NULL;
    SDL_Texture *p_rulesTex = NULL;
    SDL_Rect pausedRect = { GRID_X_POSITION_PX, SCREEN_HEIGHT_PX - 40, 0, 0 };
    SDL_Rect rulesRect =  { 0, SCREEN_HEIGHT_PX - 40, 0, 0 };

    SDL_Color textColor = { 0xF7, 0xF7, 0xF7, 0xFF };

    char rulesString[32];

    /* App control */
    SDL_Event event;
    Uint64 currTime_ms;
    Uint64 ellapsedTime_ms;
    Uint64 lastRenderTime_ms;

    int quit = FALSE;
    int paused = TRUE;
    int mousePressed = FALSE;
    uint8_t mouseCurrCellState = GRID_DEAD;
    uint8_t mouseNewCellState = GRID_DEAD;
    int mouse_xpos_pnt, mouse_ypos_pnt;

    int gridUpdate = FALSE;

    int shiftDown = FALSE;
    int ctrlDown = FALSE;

    /* Grid */
    Grid grid;
    uint8_t savedGrid[GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS];

    int iRow, iCol;

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
        SCREEN_WIDTH_PX, SCREEN_HEIGHT_PX,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (p_window == NULL)
    {
        printf("Could not create window\n");
        return 1;
    }

    SDL_GL_GetDrawableSize(p_window, &gl_width_px, &gl_height_px);
    if (gl_width_px != SCREEN_WIDTH_PX || gl_height_px != SCREEN_HEIGHT_PX)
    {
        printf("Resizing window\n");
        scaleFactor_width_pntToPx = (float) gl_width_px / (float) SCREEN_WIDTH_PX;
        scaleFactor_height_pntToPx = (float) gl_height_px / (float) SCREEN_HEIGHT_PX;
        win_width_pnt = SCREEN_WIDTH_PX / scaleFactor_width_pntToPx;
        win_height_pnt = SCREEN_HEIGHT_PX / scaleFactor_height_pntToPx;

        SDL_SetWindowSize(p_window, win_width_pnt, win_height_pnt);
    }

    p_renderer = SDL_CreateRenderer(p_window, -1, SDL_RENDERER_ACCELERATED);
    if (p_renderer == NULL)
    {
        printf("Could not create renderer\n");
        return 1;
    }
    SDL_SetRenderDrawColor(p_renderer, 0x0D, 0x0D, 0x0D, 0xFF);

    /* Load Hex */
    p_hexTex = loadTexture("assets/hex.png", p_renderer);
    if (p_hexTex == NULL)
    {
        printf("Could not load hex\n");
        return 1;
    }

    p_sikTex = loadTexture("assets/hex_sick.png", p_renderer);
    if (p_sikTex == NULL)
    {
        printf("Could not load sick hex\n");
        return 1;
    }

    p_fixTex = loadTexture("assets/hex_fix.png", p_renderer);
    if (p_fixTex == NULL)
    {
        printf("Could not load fix hex\n");
        return 1;
    }

    /* Start grid */
    srand(time(NULL));

    grid = Grid_create(GRID_WIDTH_CELLS, GRID_HEIGHT_CELLS);
    Grid_resetGrid(&grid);
    memcpy(savedGrid, grid.p_disp, GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS * sizeof(uint8_t));

    /* Load font */
    p_font = TTF_OpenFont("assets/monaco.ttf", 18);
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
                        paused = !paused;
                        break;

                    case SDLK_LSHIFT:
                    case SDLK_RSHIFT:
                        shiftDown = TRUE;
                        break;

                    case SDLK_LCTRL:
                    case SDLK_RCTRL:
                        ctrlDown = TRUE;
                        break;

                    case SDLK_r:
                        Grid_resetGrid(&grid);
                        paused = TRUE;
                        break;

                    case SDLK_c:
                        Grid_clearGrid(&grid);
                        paused = TRUE;
                        break;

                    case SDLK_f:
                        Grid_fillGrid(&grid);
                        paused = TRUE;
                        break;

                    case SDLK_s:
                        if (shiftDown == TRUE)
                        {
                            memcpy(grid.p_disp, savedGrid, GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS * sizeof(uint8_t));
                            paused = TRUE;
                        }
                        else
                        {
                            memcpy(savedGrid, grid.p_disp, GRID_WIDTH_CELLS * GRID_HEIGHT_CELLS * sizeof(uint8_t));
                        }
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_LSHIFT:
                    case SDLK_RSHIFT:
                        shiftDown = FALSE;
                        break;

                    case SDLK_LCTRL:
                    case SDLK_RCTRL:
                        ctrlDown = FALSE;
                        break;
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                SDL_GetMouseState(&mouse_xpos_pnt, &mouse_ypos_pnt);
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                mousePressed = TRUE;
                mouseCurrCellState = Grid_getDispValueFromMouse
                   (&grid,
                    scaleFactor_width_pntToPx * mouse_xpos_pnt,
                    scaleFactor_height_pntToPx * mouse_ypos_pnt);

                switch (mouseCurrCellState)
                {
                    case GRID_DEAD:
                        if (shiftDown == TRUE)
                        {
                            mouseNewCellState = GRID_SICK;
                        }
                        else if (ctrlDown == TRUE)
                        {
                            mouseNewCellState = GRID_FIXED;
                        }
                        else
                        {
                            mouseNewCellState = GRID_ALIVE;
                        }
                        break;

                    case GRID_ALIVE:
                        if (shiftDown == TRUE)
                        {
                            mouseNewCellState = GRID_SICK;
                        }
                        else if (ctrlDown == TRUE)
                        {
                            mouseNewCellState = GRID_FIXED;
                        }
                        else
                        {
                            mouseNewCellState = GRID_DEAD;
                        }
                        break;

                    case GRID_FIXED:
                    case GRID_SICK:
                        mouseNewCellState = GRID_DEAD;
                        break;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                mousePressed = FALSE;
            }
        }

        if (mousePressed == TRUE)
        {
            Grid_changeCell
                (&grid,
                scaleFactor_width_pntToPx * mouse_xpos_pnt,
                scaleFactor_height_pntToPx * mouse_ypos_pnt,
                mouseNewCellState);
        }

        /* Handle pause */
        if (paused == TRUE)
        {
            gridUpdate = FALSE;
        }

        /* Update grid */
        if (gridUpdate == TRUE)
        {
            Grid_hexGridNextWithRange
               (&grid,
                GRID_MIN_NEIGHBOURS_SURVIVE, GRID_MAX_NEIGHBOURS_SURVIVE,
                GRID_MIN_NEIGHBOURS_CREATE, GRID_MAX_NEIGHBOURS_CREATE);
        }

        /* ------ RENDER ------ */
        /* Render grid */
        SDL_RenderClear(p_renderer);

        /* Render texture to screen */
        renderRect.x = GRID_X_POSITION_PX;
        renderRect.y = GRID_Y_POSITION_PX;
        for (iRow = GRID_Y_RENDER_OFFSET_CELLS; iRow < (GRID_Y_RENDER_OFFSET_CELLS + GRID_Y_RENDER_NUM_CELLS); iRow++)
        {
            for (iCol = GRID_X_RENDER_OFFSET_CELLS; iCol < (GRID_X_RENDER_OFFSET_CELLS + GRID_X_RENDER_NUM_CELLS); iCol++)
            {
                switch (Grid_getDispValue(&grid, iRow, iCol))
                {
                    case GRID_ALIVE:
                        SDL_RenderCopy(p_renderer, p_hexTex, NULL, &renderRect);
                        break;
                    case GRID_SICK:
                        SDL_RenderCopy(p_renderer, p_sikTex, NULL, &renderRect);
                        break;
                    case GRID_FIXED:
                        SDL_RenderCopy(p_renderer, p_fixTex, NULL, &renderRect);
                        break;
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
        if (paused)
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

        snprintf(rulesString, 32, "Live: %d-%d, Birth: %d-%d",
                 GRID_MIN_NEIGHBOURS_SURVIVE, GRID_MAX_NEIGHBOURS_SURVIVE,
                 GRID_MIN_NEIGHBOURS_CREATE, GRID_MAX_NEIGHBOURS_CREATE);
        p_miscSurf = TTF_RenderText_Solid(p_font, rulesString, textColor);
        if (p_miscSurf != NULL)
        {
            p_rulesTex = SDL_CreateTextureFromSurface(p_renderer, p_miscSurf);
            if (p_rulesTex != NULL)
            {
                rulesRect.w = p_miscSurf->w;
                rulesRect.h = p_miscSurf->h;
                rulesRect.x = SCREEN_WIDTH_PX - GRID_X_POSITION_PX - rulesRect.w;
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
    /* Destroy grid */
    Grid_destroy(&grid);

    /* Destroy window */
    SDL_DestroyWindow(p_window);
    p_window = NULL;

    SDL_DestroyTexture(p_hexTex);

    /* Quit SDL subsystems */
    SDL_Quit();

    return 0;
}