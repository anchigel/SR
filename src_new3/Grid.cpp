/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include "oaDesignDB.h"
#include "Grid.h"
#include "Cell.h"
#include <iostream>
using namespace oa;
using namespace std;

Grid::Grid() {
    __dim_m = 0;
    __dim_n = 0;
    __dim_k = 0;
    __cell_width = 0;
    __cell_height = 0;
    __cell0_x = 0;
    __cell0_y = 0;
    __numVias = 0;
    __design_origin_x = 0;
    __design_origin_y = 0;
}

Grid::Grid(oaUInt4 dim_m, oaUInt4 dim_n, oaUInt4 dim_k, oaUInt4 cell_width, oaUInt4 cell_height, oaInt4 cell0_x, oaInt4 cell0_y) {
    __dim_m = dim_m;
    __dim_n = dim_n;
    __dim_k = dim_k;
    __cell_width = cell_width;
    __cell_height = cell_height;
    __cell0_x = cell0_x;
    __cell0_y = cell0_y;
    __numVias = 0;
    __design_origin_x = __cell0_x - __cell_width/2;
    __design_origin_y = __cell0_y - __cell_height/2;
    
    __cells.resize(__dim_m * __dim_n * __dim_k);
    
    for (oaUInt4 m = 0; m < __dim_m; m++) {
        for (oaUInt4 n = 0; n < __dim_n; n++) {
            for (oaUInt4 k = 0; k < __dim_k; k++) {
                __cells.at(coordsToIndex(m,n,k)) = new Cell(m,n,k,__cell0_x+m*__cell_width,__cell0_y+n*__cell_height,__cell_width,__cell_height);
            }
        }
    }
}

Grid::~Grid() {
    for (oaUInt4 i = 0; i < getNumCells(); i++) {
        delete __cells.at(i);
    }
}

Cell* Grid::at(oaUInt4 m, oaUInt4 n, oaUInt4 k) {
    return __cells.at(coordsToIndex(m,n,k));
}

Cell* Grid::atXY(oaInt4 x, oaInt4 y, oaInt4 layer) {
    //cout << "x="<<x<<" __design_origin_x="<<__design_origin_x<<" __design_origin_y="<<__design_origin_y << endl; //Weiche
    oaInt4 m = (x - __design_origin_x) / __cell_width; //Weiche
    oaInt4 n = (y - __design_origin_y) / __cell_height; //Weiche
    //oaInt4 m = x  / __cell_width;
    //oaInt4 n = y  / __cell_height;
    oaUInt4 k = layer;
    
    return __cells.at(coordsToIndex(m,n,k));
}
    
oaUInt4 Grid::coordsToIndex(oaUInt4 m, oaUInt4 n, oaUInt4 k) {
    return k * (__dim_m * __dim_n) + n * (__dim_m) + m;
}

void Grid::indexToCoords(oaUInt4 index, oaUInt4 *m, oaUInt4 *n, oaUInt4 *k) {
    oaUInt4 mtmp = index % __dim_m;
    oaUInt4 ntmp = (index / mtmp) % __dim_n;
    oaUInt4 ktmp = (index / (mtmp*ntmp)) % __dim_k;
            
    *m = mtmp;
    *n = ntmp;
    *k = ktmp;
}

void Grid::getDims(oaUInt4* dim_m, oaUInt4 *dim_n, oaUInt4 *dim_k) {
    *dim_m = __dim_m;
    *dim_n = __dim_n;
    *dim_k = __dim_k;
}

void Grid::getCellDims(oaUInt4* cell_w, oaUInt4* cell_h) {
    *cell_w = __cell_width;
    *cell_h = __cell_height;
}

void Grid::getCellZeroOrigin(oaInt4* cell0_x, oaInt4* cell0_y) {
    *cell0_x = __cell0_x;
    *cell0_y = __cell0_y;
}

oaUInt4 Grid::getNumCells() {
    return __dim_m * __dim_n * __dim_k;
}

void Grid::reset() {
    for (oaUInt4 i = 0; i < __dim_m * __dim_n * __dim_k; i++)
        __cells.at(i)->reset();
}

void Grid::softReset() {
    for (oaUInt4 i = 0; i < __dim_m * __dim_n * __dim_k; i++)
        __cells.at(i)->softReset();
}

void Grid::printStats() {
    cout << "Grid statistics:" << endl;
    cout << "...Dimensions (# Cells): " << __dim_m << " x " << __dim_n << " x " << __dim_k << "" << endl;
    cout << "...Cell Dimensions (DBU): " << __cell_width << " x " << __cell_height << endl;
    cout << "...Origin of Cell (0,0,k) in DBU: (" << __cell0_x << "," << __cell0_y << ")" << endl;
}

void Grid::print() {
    __numVias = 0;
    printLayer(0);
    printLayer(1);
	if(__dim_k == 3)
		printLayer(2);
    cout << "Total # vias: " << __numVias << endl; 
}

void Grid::printLayer(oaUInt4 k) {
    //print top boundary
    cout << endl << endl << "----LAYER " << k << "----" << endl;
    for (oaInt4 m = 0; m < __dim_m+2; m++)
        cout << "_";
    cout << endl;
    
    //print each row.
    for (oaInt4 n = __dim_n-1; n >= 0; n--) {
        cout << "|";
        for (oaInt4 m = 0; m < __dim_m; m++) {
            Cell *cell = at(m,n,k);
            CellStatus status = cell->getStatus();
            switch (status) {
                case CellFilled:
                    if (cell->isPin())
                        cout << "P";
                    else if (cell->needsVia()) {
                        cout << "^";
                        __numVias++;
                    }
                    else
                        cout << "X";
                    break;
                case CellContact:
                    if (cell->isPin() && cell->needsVia()) {
                        cout << "*";
                        __numVias++;
                    }
                    else if (cell->isPin())
                        cout << "%";
                    else if (cell->needsVia()) {
                        cout << "$";
                        __numVias++;
                    }
                    else
                        cout << "C";
                    break;
                case CellVDDRail:
                    cout << "V";
                    break;
                case CellVSSRail:
                    cout << "G";
                    break;
                case CellKeepout:
                    cout << ".";
                    break;
                case CellFree:
                    cout << " ";
                    break;
                default:
                    cout << "?";
                    break;
            }
        }
        cout << "|" << endl;
    }
    
    //print bottom boundary
    for (oaInt4 m = 0; m < __dim_m+2; m++)
        cout << "_";
    cout << endl << endl;
}

/*void Grid::printLayer(oaUInt4 k) { //Weiche
    //print top boundary
    cout << endl << endl << "----LAYER " << k << "----" << endl;
    for (oaInt4 m = 0; m < __dim_m+2; m++)
        cout << "_";
    cout << endl;
    
    //print each row.
    for (oaInt4 n = __dim_n-1; n >= 0; n--) {
        cout << "|";
        for (oaInt4 m = 0; m < __dim_m; m++) {
            Cell *cell = at(m,n,k);
            CellStatus status = cell->getStatus();
            switch (status) {
                case CellFilled:
                    if (cell->isPin())
                        cout << cell->getNetID();
                    else if (cell->needsVia()) {
                        cout << cell->getNetID();
                        __numVias++;
                    }
                    else
                        cout << cell->getNetID();
                    break;
                case CellContact:
                    if (cell->isPin() && cell->needsVia()) {
                        cout << cell->getNetID();
                        __numVias++;
                    }
                    else if (cell->isPin())
                        cout << cell->getNetID();
                    else if (cell->needsVia()) {
                        cout << cell->getNetID();
                        __numVias++;
                    }
                    else
                        cout << cell->getNetID();
                    break;
                case CellVDDRail:
                    cout << "V";
                    break;
                case CellVSSRail:
                    cout << "G";
                    break;
                case CellKeepout:
                    cout << ".";
                    break;
                case CellFree:
                    cout << " ";
                    break;
                default:
                    cout << "?";
                    break;
            }
        }
        cout << "|" << endl;
    }
    
    //print bottom boundary
    for (oaInt4 m = 0; m < __dim_m+2; m++)
        cout << "_";
    cout << endl << endl;
}*/
