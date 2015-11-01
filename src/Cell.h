/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef CELL_H
#define	CELL_H

#include "oaDesignDB.h"
using namespace oa;
using namespace std;

typedef enum {
    CellFilled, //Cell is filled by a net
    CellContact, //Cell is filled by a net that is over its contact. Only applies to Metal1
    CellVDDRail, //Cell is filled by the VDD power rail (assumed to be top of cell)
    CellVSSRail, //Cell is filled by the VSS power rail (assumed to be bottom of cell)
    CellKeepout, //No net is routed here, but it is not available for routing due to spacing rule of an adjacent feature
    CellFree //Cell is empty space, available for routing
} CellStatus;

class Cell {
public:
    Cell(); //barebones constructor, but not too useful
    Cell(oaUInt4 m, oaUInt4 n, oaUInt4 k, oaInt4 x, oaInt4 y, oaUInt4 w, oaUInt4 h); //use this constructor from the Grid. m,n,k,x,y,w,h are always fixed so no other way to change them
    virtual ~Cell();
  
    //getters
    void getPosition(oaUInt4* m, oaUInt4* n, oaUInt4 *k); //Gets the horizontal (m), vertical (n), and layer (k) position of this cell in the grid
    void getAbsolutePosition(oaInt4* x, oaInt4* y); //Gets the (x,y) Cartesian position of this cell's origin (bottom-left), relative to the origin of the design in nm
    void getAbsoluteWH(oaUInt4* w, oaUInt4* h); //Gets the absolute width and height of this cell in nm    
//    oaInt4 getWeight(); //Returns the routing weight of this cell
    bool isSource(); //Returns true if this cell is the source for maze router
    bool isSink(); //Returns true if this cell is the sink for the maze router
    CellStatus getStatus(); //Returns a CellStatus enumerator indicating status for maze routing purposes
    oaUInt4 getDistance(); //Returns the distance from the source Cell
    Cell* getBacktrace(); //Returns a pointer to the Cell who is on the shortest path back to the source in maze routing
    oaString getNetType();
    bool isPin(); //returns true if this cell is an IO pin
    oaString getPinName();//return pin name, which only makes sense if it's IO pin 
    oaInt4 getNetID();
    bool touched();
    bool needsVia(); //returns true if this cell needs a via on it. Should only be used for M1 (layer 0)
    
    //setters
 //   void setWeight(oaInt4 weight); //Sets the routing weight of this Cell
    void setSource(bool b); //Set true if this Cell is maze routing source, false otherwise
    void setSink(bool b); //Set true if this Cell is maze routing sink, false otherwise
    void setStatus(CellStatus status); //Set the status enumeration for maze routing purposes
    void setDistance(oaUInt4 dist); //Set the distance from the source node in a maze route
    void setBacktrace(Cell* prev); //Set the Cell on the shortest path back to the source Cell in a maze route
    void setNetType(oaString netType); //Set the type of the net
    void setNetID(oaInt4 nid); //numerical identifier
    void setPin(bool isPin); //set true if this cell is a pin.
    void setPinName(oaString pinName);//YB added pin name. Only meaningful if IO pin
    void reset(); //Reset all routing-relevant metrics of this Cell. Use after an iteration of maze routing an entire net
    void softReset(); //Reset some routing-relevant metrics of this Cell. Use after a segment of maze routing an entire net
    void setNeedsVia(); //cannot be unset! be careful
    void touch();
    void setIsRouted();
    bool isRouted();
private:
    //Cell position
    oaUInt4 __m; //Cell # in horizontal (X) direction
    oaUInt4 __n; //Cell # in vertical (Y) direction
    oaUInt4 __k; //Layer #
    
    //Cell position and size in DBU
    oaInt4 __x; //X coordinate of cell origin (center) in w.r.t. design origin
    oaInt4 __y; //Y coordinate in cell origin (center) in w.r.t. design origin
    oaUInt4 __w; //Width of cell
    oaUInt4 __h; //Height of cell
    
    //Distance
    oaInt4 __distance; //distance in cell #s from source (m,n,k)
    
    //Cell metadata
    CellStatus __status; //status of this cell for routing
  //  oaInt4 __weight; //routing weight
    bool __isSource; //source for this maze routing phase
    bool __isSink; //sink for this maze routing phase
    bool __isPin;
    oaString __netType; //can be VDD, VSS, IO, or S
    oaString __pinName;//name of pin, if IO
    oaInt4 __netID; //to distinguish unique nets
    Cell* __backtrace; //pointer to cell that led maze router to this one
    bool __touched;
    bool __needsVia; //should only be used for M1 cells (layer 0)
    bool __isRouted;// has already been routed.. only set if passed as source to route())
}; 

#endif	/* CELL_H */

