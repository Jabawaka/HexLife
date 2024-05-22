#include <stdio.h>
#include <stdlib.h>

#include "grid.h"


Grid Grid_create(int width_cells, int height_cells)
{
    Grid grid;

    grid.p_data1 = NULL;
    grid.p_data2 = NULL;

    grid.p_data1 = malloc(width_cells * height_cells * sizeof(uint8_t));
    grid.p_data2 = malloc(width_cells * height_cells * sizeof(uint8_t));
    if (grid.p_data1 == NULL || grid.p_data2 == NULL)
    {
        printf("[ERR] Could not create grid\n");
    }

    grid.width_cells = width_cells;
    grid.height_cells = height_cells;

    grid.p_disp = grid.p_data1;
    grid.p_next = grid.p_data2;

    return grid;
}


void Grid_resetGrid(Grid *p_grid)
{
    int iRow, iCol;

    for (iRow = 0; iRow < p_grid->height_cells; iRow++)
    {
        for (iCol = 0; iCol < p_grid->width_cells; iCol++)
        {
            if (rand() % 4 == 0)
            {
                p_grid->p_data1[iRow * p_grid->width_cells + iCol] = GRID_ALIVE;
            }
            else
            {
                p_grid->p_data1[iRow * p_grid->width_cells + iCol] = GRID_DEAD;
            }
        }
    }

    p_grid->p_disp = p_grid->p_data1;
    p_grid->p_next = p_grid->p_data2;
}


void Grid_clearGrid(Grid *p_grid)
{
    int iCell;

    for (iCell = 0; iCell < p_grid->width_cells * p_grid->height_cells; iCell++)
    {
        p_grid->p_data1[iCell] = GRID_DEAD;
    }

    p_grid->p_disp = p_grid->p_data1;
    p_grid->p_next = p_grid->p_data2;
}

void Grid_fillGrid(Grid *p_grid)
{
    int iCell;

    for (iCell = 0; iCell < p_grid->width_cells * p_grid->height_cells; iCell++)
    {
        p_grid->p_data1[iCell] = GRID_ALIVE;
    }

    p_grid->p_disp = p_grid->p_data1;
    p_grid->p_next = p_grid->p_data2;
}


int Grid_hexGridNextWithRange(Grid *p_grid, int minAlive, int maxAlive, int minCreate, int maxCreate)
{
    int iRow, iCol, iNeigh;
    int prevRow, nextRow;
    int prevCol, nextCol;

    int neighbourLocations[GRID_HEX_NUM_NEIGHBOURS];
    int aliveNeighbours;
    int anySick;

    int isStationary = TRUE;

    for (iRow = 0; iRow < p_grid->height_cells; iRow++)
    {
        prevRow = iRow - 1;
        if (prevRow < 0)
        {
            prevRow += p_grid->height_cells;
        }
        nextRow = iRow + 1;
        if (nextRow >= p_grid->height_cells)
        {
            nextRow -= p_grid->height_cells;
        }

        for (iCol = 0; iCol < p_grid->width_cells; iCol++)
        {
            prevCol = iCol - 1;
            if (prevCol < 0)
            {
                prevCol += p_grid->width_cells;
            }
            nextCol = iCol + 1;
            if (nextCol >= p_grid->width_cells)
            {
                nextCol -= p_grid->width_cells;
            }

            /* Count alive neighbours */
            if (iCol % 2 == 0)
            {
                neighbourLocations[0] = iRow    * p_grid->width_cells + prevCol;
                neighbourLocations[1] = prevRow * p_grid->width_cells + iCol;
                neighbourLocations[2] = iRow    * p_grid->width_cells + nextCol;
                neighbourLocations[3] = nextRow * p_grid->width_cells + prevCol;
                neighbourLocations[4] = nextRow * p_grid->width_cells + iCol;
                neighbourLocations[5] = nextRow * p_grid->width_cells + nextCol;
            }
            else
            {
                neighbourLocations[0] = prevRow * p_grid->width_cells + prevCol;
                neighbourLocations[1] = prevRow * p_grid->width_cells + iCol;
                neighbourLocations[2] = prevRow * p_grid->width_cells + nextCol;
                neighbourLocations[3] = iRow    * p_grid->width_cells + prevCol;
                neighbourLocations[4] = nextRow * p_grid->width_cells + iCol;
                neighbourLocations[5] = iRow    * p_grid->width_cells + nextCol;
            }

            aliveNeighbours = 0;
            anySick = FALSE;
            for (iNeigh = 0; iNeigh < GRID_HEX_NUM_NEIGHBOURS; iNeigh++)
            {
                if (p_grid->p_disp[neighbourLocations[iNeigh]] != GRID_DEAD)
                {
                    aliveNeighbours++;
                    if (p_grid->p_disp[neighbourLocations[iNeigh]] == GRID_SICK)
                    {
                        anySick = TRUE;
                    }
                }
            }

            /* Update next grid */
            Grid_setNextValue(p_grid, iRow, iCol, Grid_getDispValue(p_grid, iRow, iCol));
            if (Grid_getDispValue(p_grid, iRow, iCol) == GRID_ALIVE
             || Grid_getDispValue(p_grid, iRow, iCol) == GRID_SICK)
            {
                if (aliveNeighbours < minAlive || aliveNeighbours > maxAlive)
                {
                    Grid_setNextValue(p_grid, iRow, iCol, GRID_DEAD);
                    isStationary = FALSE;
                }
            }
            else if (Grid_getDispValue(p_grid, iRow, iCol) == GRID_DEAD)
            {
                if (aliveNeighbours >= minCreate && aliveNeighbours <= maxCreate)
                {
                    isStationary = FALSE;
                    if (anySick == TRUE)
                    {
                        Grid_setNextValue(p_grid, iRow, iCol, GRID_SICK);
                    }
                    else
                    {
                        Grid_setNextValue(p_grid, iRow, iCol, GRID_ALIVE);
                    }
                }
            }
        }
    }

    if (p_grid->p_disp == p_grid->p_data1)
    {
        p_grid->p_disp = p_grid->p_data2;
        p_grid->p_next = p_grid->p_data1;
    }
    else
    {
        p_grid->p_disp = p_grid->p_data1;
        p_grid->p_next = p_grid->p_data2;
    }

    return isStationary;
}


void Grid_changeCell(Grid *p_grid, int mouse_xpos_px, int mouse_ypos_px, int cellState)
{
    int rowCell, colCell;
    colCell = (mouse_xpos_px - (GRID_CELL_WIDTH - GRID_X_STEP_PX) / 2  - GRID_X_POSITION_PX) / GRID_X_STEP_PX + GRID_X_RENDER_OFFSET_CELLS;

    if (colCell % 2 == 0)
    {
        rowCell = (mouse_ypos_px - GRID_CELL_HEIGHT / 2 - GRID_Y_POSITION_PX) / GRID_Y_STEP_PX + GRID_Y_RENDER_OFFSET_CELLS;
    }
    else
    {
        rowCell = (mouse_ypos_px + GRID_Y_OFFSET_ROW_PX - GRID_CELL_HEIGHT / 2 - GRID_Y_POSITION_PX) / GRID_Y_STEP_PX + GRID_Y_RENDER_OFFSET_CELLS;
    }

    Grid_setDispValue(p_grid, rowCell, colCell, cellState);
}


uint8_t Grid_getDispValueFromMouse(Grid *p_grid, int mouse_xpos_px, int mouse_ypos_px)
{
    int rowCell, colCell;
    colCell = (mouse_xpos_px - (GRID_CELL_WIDTH - GRID_X_STEP_PX) / 2  - GRID_X_POSITION_PX) / GRID_X_STEP_PX + GRID_X_RENDER_OFFSET_CELLS;

    if (colCell % 2 == 0)
    {
        rowCell = (mouse_ypos_px - GRID_CELL_HEIGHT / 2 - GRID_Y_POSITION_PX) / GRID_Y_STEP_PX + GRID_Y_RENDER_OFFSET_CELLS;
    }
    else
    {
        rowCell = (mouse_ypos_px + GRID_Y_OFFSET_ROW_PX - GRID_CELL_HEIGHT / 2 - GRID_Y_POSITION_PX) / GRID_Y_STEP_PX + GRID_Y_RENDER_OFFSET_CELLS;
    }

    return Grid_getDispValue(p_grid, rowCell, colCell);
}


uint8_t Grid_getDispValue(Grid *p_grid, int row, int col)
{
    return p_grid->p_disp[row * p_grid->width_cells + col];
}


uint8_t Grid_getNextValue(Grid *p_grid, int row, int col)
{
    return p_grid->p_next[row * p_grid->width_cells + col];
}


void Grid_setDispValue(Grid *p_grid, int row, int col, uint8_t value)
{
    p_grid->p_disp[row * p_grid->width_cells + col] = value;
}


void Grid_setNextValue(Grid *p_grid, int row, int col, uint8_t value)
{
    p_grid->p_next[row * p_grid->width_cells + col] = value;
}


void Grid_destroy(Grid *p_grid)
{
    free(p_grid->p_data1);
    free(p_grid->p_data2);

    p_grid->p_disp = NULL;
    p_grid->p_next = NULL;

    p_grid->width_cells = 0;
    p_grid->height_cells = 0;
}