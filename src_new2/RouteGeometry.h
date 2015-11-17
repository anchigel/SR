/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */


#ifndef ROUTEGEOMETRY_H
#define	ROUTEGEOMETRY_H
#include "oaDesignDB.h"
#include "Grid.h"
#include "Definitions.h"
#include "ProjectDesignRules.h"
using namespace std;
using namespace oa;
class RouteGeometry {
public:
    RouteGeometry();
    void mazeToGeometry(Grid* grid, oaDesign* design,ProjectDesignRules dr);
    virtual ~RouteGeometry();
//    bool CreateViaIfNeeded(Cell* currCell, Grid* grid, oaDesign* design,
//                           ProjectDesignRules dr);
    void CreatePinTextLabel(Cell* cell, oaDesign* design,
                            ProjectDesignRules& dr);
    bool hasViaBelow(Cell* currCell, Grid* grid);
    bool isViaExtNeeded(Cell* currCell, Grid* grid);
    oaRect* createVia(Cell* currCell, ProjectDesignRules dr,
                              oaDesign* design);
private:
    //ideally this fn should be in Grid, but I'm leaving it here for now
    Cell* GetCell(Grid* grid,METAL_LAYER_INFO layerInfo,int firstDimIndex, 
                                                        int secDimIndex,
                                                        int layerIndex);
   
    bool isBackTraceOfCellAdjLayer(Cell* cell,Grid* grid,
                                   int lowerAboveBoth,
                                   Cell*& lowerAboveCell);
    bool isEndpointOfSegment(Cell* cell, Grid* grid);
    
};

#endif	/* ROUTEGEOMETRY_H */

