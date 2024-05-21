#ifndef H_HEXLIFE_GRID_H
#define H_HEXLIFE_GRID_H


#include <stdint.h>


#define GRID_DEAD  (0)
#define GRID_ALIVE (1)
#define GRID_SICK  (2)
#define GRID_FIXED (3)

#define GRID_HEX_NUM_NEIGHBOURS (6)



typedef struct Grid_struct {
    /* Main memory buffers */
    uint8_t *p_data1;
    uint8_t *p_data2;

    /* Pointers to the data to be displayed */
    uint8_t *p_disp;
    uint8_t *p_next;

    /* Size for the grid */
    int width_cells;
    int height_cells;
} Grid;


extern Grid Grid_create(int width_cells, int height_cells);

extern void Grid_resetGrid(Grid *p_grid);

extern void Grid_clearGrid(Grid *p_grid);

extern void Grid_fillGrid(Grid *p_grid);

extern void Grid_hexGridNextWithRange(Grid *p_grid, int minAlive, int maxAlive, int minCreate, int maxCreate);

extern uint8_t Grid_getDispValue(Grid *p_grid, int row, int col);

extern uint8_t Grid_getNextValue(Grid *p_grid, int row, int col);

extern void Grid_setDispValue(Grid *p_grid, int row, int col, uint8_t value);

extern void Grid_setNextValue(Grid *p_grid, int row, int col, uint8_t value);

extern void Grid_destroy(Grid *p_grid);


#endif /* H_HEXLIFE_GRID_H */