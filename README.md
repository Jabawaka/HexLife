# Description
This is a hexagonal grid version of Conway's game of life. It is a zero player
game consisting of a grid of cells that can be in one of two states, alive or
dead. The number of neighbours at each step determines whether a live cell
survives or dies or whether a dead cell can come to life.

![image]([https://github.com/Jabawaka/Hexlife.git/example.gif](https://github.com/Jabawaka/HexLife/blob/main/example.gif))

# Dependencies
You need the following libraries to be able to compile and run the project:
* SDL2
* SDL2 image
* SDL2 ttf

# Controls
You can control the grid slightly like this:
* Space: toggle between running and paused modes.
* Point and click: change a cell's state (works woth while running and paused
  modes).
* 'R': reset the grid to a random state.
* 'C': reset the grid to a clear state.
