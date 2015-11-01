/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include "Router.h"

Router::Router() {
}

Router::Router(vector<Connection>* netlist, ProjectDesignRules* rules, oaInt4 VDD_y, oaInt4 VSS_y, oaBox* design_bbox) {
    __netlist = netlist;
    __rules = rules;
    __VDD_y = VDD_y;
    __VSS_y = VSS_y;
    __design_bbox = design_bbox;
	 __foundRoute = false;    
    __grid = NULL;
}

Router::~Router()
{
}

void Router::buildGrid() {
}

bool Router::route() {
}
Grid* Router::getGrid()
{
    return __grid;
}
