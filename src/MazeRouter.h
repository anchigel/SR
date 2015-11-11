/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef MAZEROUTER_H
#define	MAZEROUTER_H

#include "Router.h"
#include "oaDesignDB.h"

using namespace oa;
using namespace std;

class MazeRouter : public Router 
{
public:
    MazeRouter();
    MazeRouter(vector<Connection>* netlist, ProjectDesignRules* rules, oaInt4 VDD_y, oaInt4 VSS_y, oaBox* design_bbox);
    ~MazeRouter();
    
    bool route();
private:
    
    void buildGrid(); //generate grid dimensions, cell sizing, cell0 origin, allocate memory for grid of cells
    oaUInt4 cellDim_coarse(); //compute cell dimensions in DBU assuming coarse grid
    oaUInt4 cellDim_fine(); //compute cell dimensions in DBU assuming fine grid
    void initializeGrid(); //enter contact info, power rails
    void routePowerNet(oaInt4 nid); //do special routing of VDD or VSS on Metal1
	void mazeRoute(oaUInt4 netID, oaInt4 contactIndex0, oaInt4 contactIndex1, bool setPin); //do maze routing of a single net
    void multitermRoute(oaUInt4 netID, setPin);
	void generateKeepouts(oaUInt4 netID); //generate keepout regions for w.r.t. a given net
    void generateKeepout(Cell* c); //Generate keepout region in a radius around a given cell
    void doBacktrace(Cell* source, Cell* sink, bool setPin); //Trace back from sink to source, filling cells as needed. Also do pin selection.
    oaUInt4 __keepoutRadius_lateral_dbu;
    oaUInt4 __keepoutRadius_lateral;
    oaUInt4 __keepoutRadius_longitudinal_dbu;
    oaUInt4 __keepoutRadius_longitudinal;
    oaUInt4 __keepoutRadius_powerRail_dbu;
    oaUInt4 __keepoutRadius_powerRail;
    oaUInt4 __keepoutRadius_bbox_m1_dbu;
    oaUInt4 __keepoutRadius_bbox_m1;
    oaUInt4 __keepoutRadius_bbox_m2_dbu;
    oaUInt4 __keepoutRadius_bbox_m2;
};

#endif	/* MAZEROUTER_H */

