/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef GRID_H
#define	GRID_H

#include "oaDesignDB.h"
#include "Cell.h"
using namespace oa;
using namespace std;

class Grid {
public:
    Grid();
    Grid(oaUInt4 dim_m, oaUInt4 dim_n, oaUInt4 dim_k, oaUInt4 cell_width, oaUInt4 cell_height, oaInt4 cell0_x, oaInt4 cell0_y);
    virtual ~Grid();
    
    Cell* at(oaUInt4 m, oaUInt4 n, oaUInt4 k); //Returns a reference to the cell at position (m,n,k) in grid
    Cell* atXY(oaInt4 x, oaInt4 y, oaInt4 layer); //Returns a reference to the Cell containing position (x,y) in DBU
    
    oaUInt4 coordsToIndex(oaUInt4 m, oaUInt4 n, oaUInt4 k); //converts (m,n,k) coordinates to an index
    void indexToCoords(oaUInt4 index, oaUInt4* m, oaUInt4* n, oaUInt4* k); //converts an index to (m,n,k) coordinates
    
    void getDims(oaUInt4* dim_m, oaUInt4 *dim_n, oaUInt4 *dim_k); //gets the dimensions of the grid
    void getCellDims(oaUInt4* cell_w, oaUInt4* cell_h); //gets the dimensions of each cell in DBU
    void getCellZeroOrigin(oaInt4* cell0_x, oaInt4* cell0_y); //gets the origin of cell0 in DBU
    oaUInt4 getNumCells(); //gets the total number of cells
    
    void reset(); //reset all relevant routing info of all cells
    void softReset(); //reset only source/sink and distance of all cells
    void printStats(); //print grid and cell dimensions, cell0 position
    void print(); //print out grid as ASCII art
private:
    /* m is X direction (horizontal) n is Y direction (vertical), k is Z direction (layer) */
    oaUInt4 __dim_m;
    oaUInt4 __dim_n;
    oaUInt4 __dim_k;
    oaUInt4 __cell_width;
    oaUInt4 __cell_height;
    oaInt4 __cell0_x; //center of cell
    oaInt4 __cell0_y; //center of cell
    oaInt4 __design_origin_x;
    oaInt4 __design_origin_y;
    oaUInt4 __numVias;
    
    vector<Cell*> __cells; //linear vector for speed, we emulate a 3D vector through our methods
    
    void printLayer(oaUInt4 k); //prints layer K as ASCII art
};

#endif	/* GRID_H */
