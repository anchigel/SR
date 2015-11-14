/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */


#ifndef ROUTER_H
#define	ROUTER_H

#include "oaDesignDB.h"
#include "Grid.h"
#include "Connection.h"
#include "ProjectDesignRules.h"

using namespace oa;
using namespace std;

/*
 * This is the base class router. Specific implementations of routers should inherit
 * this class
 */
class Router {
public:
    Router();
    Router(vector<Connection>* netlist, ProjectDesignRules* rules, oaInt4 VDD_y, oaInt4 VSS_y, oaBox* design_bbox);
    ~Router();
    
    virtual bool route();
    Grid* getGrid();
protected:
    Grid* __grid;
    vector<Connection>* __netlist;
    vector<vector<Cell*> > __contactCells;
    ProjectDesignRules* __rules;
    oaInt4 __VDD_y;
    oaInt4 __VSS_y;
    oaBox* __design_bbox;
	 bool __foundRoute;
    
    virtual void buildGrid();
};

#endif	/* ROUTER_H */

