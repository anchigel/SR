/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include "MazeRouter.h"
#include "oaDesignDB.h"
#include "Definitions.h"
#include "ClosestPairIndices.h"
#include <iostream>

using namespace oa;
using namespace std;

MazeRouter::MazeRouter()
{
    __keepoutRadius_lateral_dbu = 0;
    __keepoutRadius_lateral = 0;
    __keepoutRadius_longitudinal_dbu = 0;
    __keepoutRadius_longitudinal = 0;
    __keepoutRadius_powerRail_dbu = 0;
    __keepoutRadius_powerRail = 0;
    __keepoutRadius_bbox_m1_dbu = 0;
    __keepoutRadius_bbox_m1 = 0;
    __keepoutRadius_bbox_m2_dbu = 0;
    __keepoutRadius_bbox_m2 = 0;
}

MazeRouter::MazeRouter(vector<Connection>* netlist, ProjectDesignRules* rules, oaInt4 VDD_y, oaInt4 VSS_y, oaBox* design_bbox, oaUInt4 n_Layers) : Router(netlist,rules,VDD_y,VSS_y,design_bbox,n_Layers) {
    
    buildGrid(n_Layers);
    
    //Lateral keepout radius (for parallel wire segments on same layer): Min metal width + min metal spacing = metal pitch
    __keepoutRadius_lateral_dbu = __rules->getMetalWidthRule() + __rules->getMetalSpaceRule();
    oaUInt4 cell_w, cell_h = 0; //assume width=height
    __grid->getCellDims(&cell_w, &cell_h);
    __keepoutRadius_lateral = __keepoutRadius_lateral_dbu / cell_w;
   
    //Longitudinal keepout radius (for line ends on same layer): Min metal spacing + 2*extension + contact size to margin for worst case where two contacted line ends are next to each other.
    __keepoutRadius_longitudinal_dbu = __rules->getMetalSpaceRule() + 2*__rules->getContactViaExtensionRule()+__rules->getViaDimensionRule();
    __keepoutRadius_longitudinal = __keepoutRadius_longitudinal_dbu / cell_w;
    
    //power rails don't have contacts nor extensions so we can be more liberal with their keepouts. Should be S+E+V/2
    __keepoutRadius_powerRail_dbu = __rules->getMetalSpaceRule() + __rules->getContactViaExtensionRule() + __rules->getViaDimensionRule()/2;
    __keepoutRadius_powerRail = __keepoutRadius_powerRail_dbu / cell_w;
    
    //create bounding box keepout on Metal1 and Metal2
    __keepoutRadius_bbox_m1_dbu = __rules->getMetalWidthRule() / 2;
    __keepoutRadius_bbox_m1 = __keepoutRadius_bbox_m1_dbu / cell_w;
    __keepoutRadius_bbox_m2_dbu = __rules->getViaDimensionRule() / 2 + __rules->getContactViaExtensionRule();
    __keepoutRadius_bbox_m2 = __keepoutRadius_bbox_m2_dbu / cell_w;
	
	num_of_layers = n_Layers;
    
    cout << "Maze routing lateral keepout radius (DBU): " << __keepoutRadius_lateral_dbu << endl;
    cout << "Maze routing lateral keepout radius (cells): " << __keepoutRadius_lateral << endl;
    cout << "Maze routing longitudinal keepout radius (DBU): " << __keepoutRadius_longitudinal_dbu << endl;
    cout << "Maze routing longitudinal keepout radius (cells): " << __keepoutRadius_longitudinal << endl;
    cout << "Maze routing power rail keepout radius (DBU): " << __keepoutRadius_powerRail_dbu << endl;
    cout << "Maze routing power rail keepout radius (cells): " << __keepoutRadius_powerRail << endl;
    cout << "Maze routing bounding box keepout radius for Metal1 (DBU): " << __keepoutRadius_bbox_m1_dbu << endl;
    cout << "Maze routing bounding box keepout radius for Metal1: " << __keepoutRadius_bbox_m1 << endl;
    cout << "Maze routing bounding box keepout radius for Metal2 (DBU): " << __keepoutRadius_bbox_m2_dbu << endl;
    cout << "Maze routing bounding box keepout radius for Metal2: " << __keepoutRadius_bbox_m2 << endl;
}

MazeRouter::~MazeRouter()
{
}

bool MazeRouter::route() {
	 __foundRoute = true; //assume true until we find an issue

    cout << "Special routing power nets..." << endl;
    //cout << "Special routing power nets: " << VDD_NET_ID << " for VDD and " << VSS_NET_ID << " for VSS." << endl; //Weiche
    routePowerNet(VDD_NET_ID);
    routePowerNet(VSS_NET_ID);
    
    //Single-contact IO nets: special case. No routing, only pin needed, but it gets high priority.
    cout << "Generating pins for single-contact IO nets..." << endl;
    __grid->reset();
    for (oaInt4 net = 2; net < __contactCells.size(); net++) {
        if (__contactCells[net][0]->getNetType() == "IO" && __contactCells[net].size() == 1) { 
            oaUInt4 m,n,k = 0;
            Cell* c = __contactCells[net][0];
            c->getPosition(&m,&n,&k);
            
            //We want to do this at the contact M1, since M2 pins are not required.
            //Propagate pin name (used later in creating text label))
            __grid->at(m,n,k)->setPin(true);
            __grid->at(m,n,k)->setPinName(c->getPinName());
        }
    }
    
    //Multi-contact IO nets. Need routing. Choose pin location after routing is done
    cout << "Maze routing multi-contact IO nets..." << endl;
    for (oaInt4 net = 2; net < __contactCells.size(); net++) {
        if (__contactCells[net][0]->getNetType() == "IO" && __contactCells[net].size() > 1) {
            //Clean reset for each net
            __grid->reset();
            generateKeepouts(net);
            
            //Basic approach: route from contact to contact in order of appearance.
            for (oaInt4 contactIndex = 0; contactIndex < __contactCells[net].size()-1; contactIndex++) {
                __grid->softReset(); //Preserve keepouts during soft reset so we don't waste time recomputing them
                if (contactIndex == 0) //set pin during first segment
                    mazeRoute(net, contactIndex+1, contactIndex, true);
                else   
                    mazeRoute(net, contactIndex+1, contactIndex, false);
            }

            /*
            //Yasmine's "better" approach: route between closest pairs of contacts first. You are welcome to try this. Disabled by default.
            //Get closest pairs
            vector<ClosestPairIndices> closestPairs;
            closestPairs=ClosestPairIndices::getClosestPairs(__contactCells[net]);
            int nPairs=closestPairs.size();
            for (oaInt4 pairIndex = 0; pairIndex <nPairs; pairIndex++) {
                __grid->softReset(); //Preserve keepouts during soft reset so we don't waste time recomputing them
                if (pairIndex == 0) //set pin during first segment
                    mazeRoute(net, closestPairs[pairIndex].getIndex1(),
                                   closestPairs[pairIndex].getIndex2(), true);
                else   
                   mazeRoute(net, closestPairs[pairIndex].getIndex1(),
                                   closestPairs[pairIndex].getIndex2(), false);
            }
            */
        }
    }
    
    //Multi-contact S nets, no pins needed.
    cout << "Maze routing S nets..." << endl;
    for (oaInt4 net = 2; net < __contactCells.size(); net++) { //Mark: perhaps change this to order by HPWL bounding box of nets? Smallest first?
          //Reset the grid weights, distances, sources, and sinks. Clear any keepout cells that were leftover from a previous maze route.    
        if (__contactCells[net][0]->getNetType() == "S") {
            //Clean reset for each net
            __grid->reset();
            generateKeepouts(net); //Generate keepouts for all nets OTHER than this one!
           
            for (oaInt4 contactIndex = 0; contactIndex < __contactCells[net].size()-1; contactIndex++) {
                __grid->softReset();
                mazeRoute(net, contactIndex+1, contactIndex, false); //Assume we always have >=2 pins for this net type, otherwise it wouldn't even make sense.
            }

            /*
            //Yasmine's "better" approach: route between closest pairs of contacts first. You are welcome to try this. Disabled by default.
            vector<ClosestPairIndices> closestPairs;
            closestPairs=ClosestPairIndices::getClosestPairs(__contactCells[net]);
            int nPairs=closestPairs.size();
            for (oaInt4 pairIndex = 0; pairIndex <nPairs; pairIndex++) {
                __grid->softReset(); //Preserve keepouts during soft reset so we don't waste time recomputing them
                mazeRoute(net, closestPairs[pairIndex].getIndex1(),
                                   closestPairs[pairIndex].getIndex2(), false);
            }
            */
        }
    }
   
    __grid->reset();
    //__grid->print();
	
	 return __foundRoute;
}

void MazeRouter::buildGrid(oaUInt4 n_Layers) {
    cout << "MazeRouter is building the Grid..." << endl;
    
    //First, determine the dimension (width, height) of each cell in DBU. For MazeRouter, width and height should be equal.
    oaUInt4 cell_dim_x = cellDim_fine();
    oaUInt4 cell_dim_y = cellDim_fine();
    //oaUInt4 cell_dim_x = 16; //Weiche
    //oaUInt4 cell_dim_y = 18; //Weiche
    
    //Next, determine the number of cells in each dimension (m,n,k). The Grid should fit entirely INSIDE the design bounding box so we don't accidentally route outside.
    oaUInt4 design_width = __design_bbox->getWidth();
    oaUInt4 design_height = __design_bbox->getHeight();
    oaUInt4 m = 1;
    oaUInt4 n = 1;
    oaUInt4 k = 1;
   
    m = design_width / cell_dim_x;
    n = design_height / cell_dim_y;
    k = n_Layers; //Variable, depending on input in main.cpp
    
    //Next, determine the origin (x0,y0) of the bottom-left-most cell on layer 0, relative to origin of design in DBU
    oaInt4 cell0_x = 0;
    oaInt4 cell0_y = 0;
    
    cell0_x = __design_bbox->lowerLeft().x() + cell_dim_x/2;
    cell0_y = __design_bbox->lowerLeft().y() + cell_dim_y/2;
	//cout<<"Orig y: "<<__design_bbox->lowerLeft().y()<<endl; //Weiche
    __grid = new Grid(m,n,k,cell_dim_x,cell_dim_y,cell0_x,cell0_y);
    cout << "Initializing grid..." << endl;
    initializeGrid();
    __grid->printStats();
}

oaUInt4 MazeRouter::cellDim_coarse() {
    //Cell dim should be max(metalwidth, metalspace) + 2*viaextensionrule. Note we assume via/contact dimensions <= metal width always.
    oaUInt4 cell_dim = 0;
    
    if (__rules->getMetalSpaceRule() > __rules->getMetalWidthRule())
        cell_dim = __rules->getMetalSpaceRule();
    else
        cell_dim = __rules->getMetalWidthRule();
    cell_dim += 2*__rules->getContactViaExtensionRule();
    if (cell_dim % 2 == 1) //odd, round up to even number
        cell_dim++;      
    
    return cell_dim;
}

oaUInt4 MazeRouter::cellDim_fine() {
    //Cell dim should be 5nm x 5nm = 50 DBU x 50 DBU.
    return 50;     
}

void MazeRouter::initializeGrid() {
    __grid->reset();
    
    //Map each net's contacts to grid cells     
    cout << "Mapping net contacts to grid cells, netlist size = "<< __netlist->size() << "..." << endl;
    for (int i = 0; i < __netlist->size(); i++) {
        Connection c = __netlist->at(i);
        cout << "...Setting up net " << i << " which has type " << c.netType << "..." << endl;
        vector<Cell*> temp; //vector of Cells containing contacts for this net
        for (int j = 0; j < c.contactCoords.size(); j++) {
            oaPoint p = c.contactCoords.at(j);
            oaInt4 x = p.x();
            oaInt4 y = p.y();
            //cout << "before __grid->atXY x="<<x<<" y="<<y << endl;
            Cell* contactCell = __grid->atXY(x,y,0);
            //cout << "after __grid->atXY" << endl;
            oaUInt4 m,n,k = 0;
            contactCell->getPosition(&m,&n,&k);
            cout << "...... contact origin at (" << x << "," << y << ") DBU, which corresponds to Grid Cell (" << m << "," << n << "," << k << ")" << endl;
            contactCell->setNetID(i);
            contactCell->setNetType(oaString(c.netType.c_str()));
            contactCell->setPinName(c.pinName.c_str()); 
            contactCell->setStatus(CellContact);
         
            temp.insert(temp.end(),contactCell);
        }
        __contactCells.insert(__contactCells.end(), temp);
    }
    
    //Map VDD/VSS rails to cells
    cout << "Mapping power rails to grid cells..." << endl;
    oaUInt4 dim_m, dim_n, dim_k = 0;
    oaInt4 x,y = 0;  
    __grid->getDims(&dim_m, &dim_n, &dim_k);
    
    for (oaUInt4 m = 0; m < dim_m; m++) {
        for (oaUInt4 n = 0; n < dim_n; n++) {
            __grid->at(m,n,0)->getAbsolutePosition(&x,&y);
			//cout<<"x="<<x<<" y="<<y<<" __VDD_y="<<__VDD_y<<" __VSS_y="<<__VSS_y<<"\n"; //Weiche
            if (y >= __VDD_y) {
                __grid->at(m,n,0)->setNetType("VDD");
                __grid->at(m,n,0)->setNetID(VDD_NET_ID);
                __grid->at(m,n,0)->setStatus(CellVDDRail);
            } else if (y <= __VSS_y) {
                __grid->at(m,n,0)->setNetType("VSS");
                __grid->at(m,n,0)->setNetID(VSS_NET_ID);
                __grid->at(m,n,0)->setStatus(CellVSSRail);
            } 
        }
    }
    
    cout << "Finished initializing the Grid." << endl;
}

void MazeRouter::routePowerNet(oaInt4 nid) {
    //We assume that each contact that needs to connect to VDD can route directly up in Metal1.
    //We assume that each contact that needs to connect to VSS can route directly down in Metal1.
    //This appears reasonable with the given test cases, and the knowledge of P/N diffusion regions and normal CMOS logic.
    oaUInt4 dim_m, dim_n, dim_k = 0;
    __grid->getDims(&dim_m, &dim_n, &dim_k);
	
	//Metal 1 cannot be used to route vertically to VDD/VSS if it's direction is not V or B
	//Use metal 2 instead if that's the case
	oaUInt4 layer = 0;
	if(__rules->getMetal1Direction() == 'H') {
		layer = 1;
	}

    //cout << "In routePowerNet! dim_m = " << dim_m << " dim_n = " << dim_n << endl; //Weiche
    //Let's loop through all contacts that are on the net, and generate their routes one-by-one.
    for (oaInt4 i = 0; i < __contactCells[nid].size(); i++) {
        Cell* contact = __contactCells[nid][i];
        oaUInt4 m,n,k = 0;
        contact->getPosition(&m, &n, &k);
        oaInt4 j = n;
        bool done = false;
		
		if(layer == 1) {
			contact->setNeedsVia();
		}
		
		//int nCellsInRail=11; //Weiche
		//int nCellsInRail=0; //Weiche	
			while (!done) { //Iterate through cells until we hit a rail
            //cout<<"before at m="<<m<<" j="<<j<<" k="<<k<<endl; //Weiche
			Cell* curr = __grid->at(m,j,layer);
			Cell* vdd_vss = __grid->at(m,j,k);
            //cout<<"after at m="<<m<<" j="<<j<<" k="<<k; //Weiche
			CellStatus status = curr->getStatus();
			CellStatus status2 = vdd_vss->getStatus();
            oaUInt4 net_id = curr->getNetID();
            oaUInt4 net_id2 = vdd_vss->getNetID();
			
			//Check cell underneath to see it it's vdd or vss
			if(layer == 1) {
				if(status2 == CellVDDRail && nid == VDD_NET_ID) {
					done = true;
					/*__grid->at(m,j+1,layer)->setBacktrace(__grid->at(m,j,layer));
					__grid->at(m,j+2,layer)->setBacktrace(__grid->at(m,j+1,layer));
					__grid->at(m,j+3,layer)->setBacktrace(__grid->at(m,j+2,layer));
					__grid->at(m,j+4,layer)->setBacktrace(__grid->at(m,j+3,layer));
					__grid->at(m,j+5,layer)->setBacktrace(__grid->at(m,j+4,layer));
					__grid->at(m,j+5,0)->setBacktrace(__grid->at(m,j+5,layer));
					__grid->at(m,j+5,0)->setNeedsVia();*/
					vdd_vss->setNeedsVia();
					vdd_vss->setBacktrace(curr);
				}
				else if(status2 == CellVSSRail && nid == VSS_NET_ID) {
					done = true;
					/*__grid->at(m,j-1,layer)->setBacktrace(__grid->at(m,j,layer));
					__grid->at(m,j-2,layer)->setBacktrace(__grid->at(m,j-1,layer));
					__grid->at(m,j-3,layer)->setBacktrace(__grid->at(m,j-2,layer));
					__grid->at(m,j-4,layer)->setBacktrace(__grid->at(m,j-3,layer));
					__grid->at(m,j-5,layer)->setBacktrace(__grid->at(m,j-4,layer));
					__grid->at(m,j-5,0)->setBacktrace(__grid->at(m,j-5,layer));
					__grid->at(m,j-5,0)->setNeedsVia();*/
					vdd_vss->setNeedsVia();
					vdd_vss->setBacktrace(curr);
				}
			}

            switch (status) {
                case CellVDDRail:
				//cout<<"  case CellVDDRail"<<endl; //Weiche
                    if (nid == VDD_NET_ID)
					{
						/*if(nCellsInRail>0) //Weiche
						{
							curr->setStatus(CellFilled);
			                curr->setNetType(contact->getNetType());
				            curr->setNetID(nid);
							nCellsInRail--;
							if(nCellsInRail==5)
								curr->setNeedsVia();
						}*/
						/*if(nCellsInRail==0){ //Weiche
							curr->setStatus(CellFilled);
			                curr->setNetType(contact->getNetType());
				            curr->setNetID(nid);
							curr->setNeedsVia();
							done = true;
						}*/
                        done = true;
					}
                    else {
                        cout << "Somehow hit the VDD rail while routing VSS!" << endl;
						__foundRoute = false;
                        //__grid->print();
                        exit(1);
                    }
                    break;
                case CellVSSRail:
                    //cout<<"  case CellVSSRail"<<endl; //Weiche
                    if (nid == VSS_NET_ID){
						/*if(nCellsInRail>0) //Weiche
						{
							curr->setStatus(CellFilled);
			                curr->setNetType(contact->getNetType());
				            curr->setNetID(nid);
							nCellsInRail--;
							if(nCellsInRail==5)
								curr->setNeedsVia();
						}*/
						/* if(nCellsInRail==0){ //Weiche
							curr->setStatus(CellFilled);
			                curr->setNetType(contact->getNetType());
				            curr->setNetID(nid);
							curr->setNeedsVia();
							done = true;
						}*/
                        done = true;
					}
                    else {
                        cout << "Somehow hit the VSS rail while routing VDD!" << endl;
						__foundRoute = false;
                        //__grid->print();
                        exit(1);
                    }
                    break;
                case CellFilled:
                    //cout<<"  case CellFilled"<<endl; //Weiche
                case CellContact:
				    //cout<<"  case CellContact"<<endl; //Weiche
                    if (net_id != nid) {
						//cout << "net_id = " << net_id << " nid = " << nid << endl; //Weiche
                        cout << "We have a problem routing power net " << contact->getNetType() << "! We hit an occupied cell that wasn't ours. Here's the Grid." << endl;
						__foundRoute = false;
                        //cout<<"m: "<<m<<", j: "<<j<<", k: "<<k<<endl; //Weiche
						//__grid->print();
                        exit(1);
                    }
                    break;
                case CellKeepout:
				    //cout<<"  case CellKeepout"<<endl; //Weiche
                    cout << "We have a problem routing power net " << contact->getNetType() << "! We hit a keepout region. Here's the Grid." << endl;
                    __foundRoute = false;
                    __grid->print();
                    exit(1);
                    break;
                case CellFree:
				    //cout<<"  case CellFree"<<endl; //Weiche
                    curr->setStatus(CellFilled);
                    curr->setNetType(contact->getNetType());
                    curr->setNetID(nid);
                    break;
            }
                       
            if (nid == VDD_NET_ID) {
                //create backtrace
				if(i == 1)
					curr->setBacktrace(__grid->at(m,j-1,k));
                else if (i > 0)
                    curr->setBacktrace(__grid->at(m,j-1,layer));
                j++;
            } else {
                //create backtrace
				if(i == 1)
					curr->setBacktrace(__grid->at(m,j+1,k));
                else if (i > 0)
                    curr->setBacktrace(__grid->at(m,j+1,layer));
                j--;
            }
        }       
    }
} 

void MazeRouter::mazeRoute(oaUInt4 netID, oaInt4 contactIndex0, oaInt4 contactIndex1, bool setPin) { //Lee's algorithm
    //First, choose two endpoints to connect, and set them as source and sink.
    Cell* source = __contactCells[netID][contactIndex0]; 
    Cell* sink = __contactCells[netID][contactIndex1];
 
    source->setIsRouted();
    
    source->setSource(true);
    sink->setSink(true);
    
    oaUInt4 sourcem, sourcen, sourcek = 0;
    oaInt4 sourceNetID = source->getNetID();
    oaUInt4 sinkm, sinkn, sinkk = 0;
    source->getPosition(&sourcem,&sourcen,&sourcek);
    sink->getPosition(&sinkm,&sinkn,&sinkk);
    //cout<<"source m: "<<(int)sourcem<<" sourcen "<<(int)sourcen<<endl;  //Weiche
    //Next, initialize our plist and nlist (two adjacent BFS wavefronts)
    vector<Cell*> plist;
    vector<Cell*> nlist;
    vector<Cell*> neighbors;
    bool path_exists = false;
    oaUInt4 dist = 0;
    Cell* curr = NULL;
    oaUInt4 m,n,k = 0;    
    oaUInt4 dim_m, dim_n, dim_k = 0;
    __grid->getDims(&dim_m,&dim_n,&dim_k);
   
    plist.push_back(source); //we start with the source only.
    while (plist.size() > 0 && !path_exists) {
        //One wave
        for (int p = 0; p < plist.size(); p++) {
            curr = plist[p];
            //Iterate over neighbors of curr
            curr->getPosition(&m,&n,&k);
            neighbors.clear();

           if (k == 0) { //bottom layer, M1, route vertically only
                if(__rules->getMetal1Direction() == 'V') { //vertical only
					if (n-1 >= 0)
						neighbors.push_back(__grid->at(m,n-1,k));
					if (n+1 < dim_n)
						neighbors.push_back(__grid->at(m,n+1,k));
				}
                else if(__rules->getMetal1Direction() == 'H') { //horizontal only
					if (m-1 >= 0)
						neighbors.push_back(__grid->at(m-1,n,k));
					if (m+1 < dim_m)
						neighbors.push_back(__grid->at(m+1,n,k));
				}
				else if(__rules->getMetal1Direction() == 'B') { //bidirectional
					if (m-1 >= 0)
						neighbors.push_back(__grid->at(m-1,n,k));
					if (m+1 < dim_m)
						neighbors.push_back(__grid->at(m+1,n,k));
					if (n-1 >= 0)
						neighbors.push_back(__grid->at(m,n-1,k));
					if (n+1 < dim_n)
						neighbors.push_back(__grid->at(m,n+1,k));
				}
				neighbors.push_back(__grid->at(m,n,1));
				if(num_of_layers == 3)
					neighbors.push_back(__grid->at(m,n,2));
            }
            else if(k == 1){ //M2
				if(__rules->getMetal2Direction() == 'V') { //vertial only
					if (n-1 >= 0)
						neighbors.push_back(__grid->at(m,n-1,k));
					if (n+1 < dim_n)
						neighbors.push_back(__grid->at(m,n+1,k));
				}
                else if(__rules->getMetal2Direction() == 'H') { //horizontal only
					if (m-1 >= 0)
						neighbors.push_back(__grid->at(m-1,n,k));
					if (m+1 < dim_m)
						neighbors.push_back(__grid->at(m+1,n,k));
				}
				else if(__rules->getMetal2Direction() == 'B') { //bidirectional
					if (n-1 >= 0)
						neighbors.push_back(__grid->at(m,n-1,k));
					if (n+1 < dim_n)
						neighbors.push_back(__grid->at(m,n+1,k));
					if (m-1 >= 0)
						neighbors.push_back(__grid->at(m-1,n,k));
					if (m+1 < dim_m)
						neighbors.push_back(__grid->at(m+1,n,k));
				}
				neighbors.push_back(__grid->at(m,n,0));
				if(num_of_layers == 3)
					neighbors.push_back(__grid->at(m,n,2));
            }
			else if(k == 2){ //M3
				if(__rules->getMetal3Direction() == 'V') { //vertial only
					if (n-1 >= 0)
						neighbors.push_back(__grid->at(m,n-1,k));
					if (n+1 < dim_n)
						neighbors.push_back(__grid->at(m,n+1,k));
				}
                else if(__rules->getMetal3Direction() == 'H') { //horizontal only
					if (m-1 >= 0)
						neighbors.push_back(__grid->at(m-1,n,k));
					if (m+1 < dim_m)
						neighbors.push_back(__grid->at(m+1,n,k));
				}
				else if(__rules->getMetal3Direction() == 'B') { //bidirectional
					if (n-1 >= 0)
						neighbors.push_back(__grid->at(m,n-1,k));
					if (n+1 < dim_n)
						neighbors.push_back(__grid->at(m,n+1,k));
					if (m-1 >= 0)
						neighbors.push_back(__grid->at(m-1,n,k));
					if (m+1 < dim_m)
						neighbors.push_back(__grid->at(m+1,n,k));
				}
				 neighbors.push_back(__grid->at(m,n,1)); 
				 neighbors.push_back(__grid->at(m,n,0)); 
			}
    
            Cell* next = NULL;
            CellStatus nextstatus;
            oaInt4 netID = -1;
            for (int d = 0; d < neighbors.size(); d++) {
                next = neighbors[d];   
                nextstatus = next->getStatus();
                netID = next->getNetID();
                if (!next->touched() && //cell must NOT be touched and...
                        (nextstatus == CellFree || //free, or
                                ((nextstatus == CellFilled || nextstatus == CellContact) && //filled or contact, and...
                                        netID == sourceNetID))) { //same net ID as source.
                    next->touch();
                    
                    next->setDistance(dist+1);
                    next->setBacktrace(curr);
                    nlist.push_back(next);
                    if (next->isSink()) { //HURRAY WE FOUND IT
                        path_exists = true;
                    }
                    
                    /*
                    //test by Yasmine: enough to touch any of cells of this net
                    unsigned int hSrc,vSrc,lSrc, hNext,vNext,lNext, hCurr,vCurr,lCurr;
                    source->getPosition(&hSrc,&vSrc,&lSrc);
                    curr->getPosition(&hCurr,&vCurr,&lCurr);
                    next->getPosition(&hNext,&vNext, &lNext);
                    if(hNext==hSrc//vertically aligned
                    && lNext==lSrc//same layer
                    && lSrc==lCurr//same layer (no via just before the reached cell)
                       && lNext==0)//heuristic to try to favor keeping on M1 layer
                    {
                        if(((nextstatus==CellFilled)//filled or contact but already routed
                            ||(nextstatus==CellContact && (next->isRouted())))                             
                            && netID==sourceNetID && next!=source)
                        {
                            path_exists=true;
                            sink=next;
                        }
                    }
                    //end test
                    */
                }
            }
        }
        dist++;
        plist = nlist;
        nlist.clear();
    }
    
    if (path_exists) {
#ifdef DEBUG
        cout << "Found a path from Cell (" << sourcem << "," << sourcen << "," << sourcek << ") to (" << sinkm << "," << sinkn << "," << sinkk << ")" << endl;
#endif
        doBacktrace(source, sink, setPin);
    } else {
        cout << "DID NOT find a path from Cell (" << sourcem << "," << sourcen << "," << sourcek << ") to (" << sinkm << "," << sinkn << "," << sinkk << ")" << endl;
		  __foundRoute = false;
    }   
}

void MazeRouter::doBacktrace(Cell* source, Cell* sink, bool setPin) {
    Cell* curr = sink;
    Cell* tmp = NULL;
    oaUInt4 currm,currn,currk = 0;
    oaUInt4 tmpm,tmpn,tmpk = 0;
    
    //Handle pin
    if (setPin) {
        curr->getPosition(&currm,&currn,&currk);
        tmp = curr->getBacktrace();
        tmp->getPosition(&tmpm,&tmpn,&tmpk);
        
        if (currm == tmpm && currn == tmpn && currk == (tmpk+1)%2) { //backtrace is above the contact, set pin on M2 (the backtraced cell).
            tmp->setPin(true);
            tmp->setPinName(source->getPinName());
        }
        else { //set pin on the contact (sink)
            curr->setPin(true);
        }
    }
    
    // backtrace sink to source
    curr = sink;
    tmp = curr;
    while (tmp != source) {       
        if (curr->getStatus() == CellFree) {
            curr->setStatus(CellFilled);
            curr->setNetID(source->getNetID());
            curr->setNetType(source->getNetType());
        }
        
        //check if we need a via.
		  oaInt4 currm_dbu, currn_dbu = 0;
		  oaInt4 tmpm_dbu, tmpn_dbu = 0;

        curr->getPosition(&currm, &currn, &currk);
		  curr->getAbsolutePosition(&currm_dbu,&currn_dbu);
        tmp->getPosition(&tmpm,&tmpn,&tmpk);
		  tmp->getAbsolutePosition(&tmpm_dbu,&tmpn_dbu);

        if (currk == 0 && tmpk == 1) { //change from layer 1 to layer 0
            //set via on curr, which is M1.
            curr->setNeedsVia();
            cout << "Via needed at cell (" << currm << "," << currn << "," << currk << ") ---> (" << currm_dbu << "," << currn_dbu << ")" << endl;
        }
        else if (currk == 1 && tmpk == 0) { //change from layer 0 to layer 1
            //set via on tmp, which is M1.
            tmp->setNeedsVia();
            cout << "Via needed at cell (" << tmpm << "," << tmpn << "," << tmpk << ") ---> (" << tmpm_dbu << "," << tmpn_dbu << ")" << endl;
        }
		else if (currk == 1 && tmpk == 2) { //change from layer 2 to layer 1
            //set via on curr, which is M2.
            curr->setNeedsVia();
            cout << "Via needed at cell (" << currm << "," << currn << "," << currk << ") ---> (" << currm_dbu << "," << currn_dbu << ")" << endl;
        }
		else if (currk == 2 && tmpk == 1) { //change from layer 1 to layer 2
            //set via on tmp, which is M2.
            tmp->setNeedsVia();
            cout << "Via needed at cell (" << tmpm << "," << tmpn << "," << tmpk << ") ---> (" << tmpm_dbu << "," << tmpn_dbu << ")" << endl;
        }
		else if (currk == 0 && tmpk == 2) { //change from layer 2 to layer 0
            //set via on curr, which is M1.
            curr->setNeedsVia();
			Cell * temp = __grid->at(currm, currn, (currk+1));
			CellStatus tempStatus = temp->getStatus();
			//if(tempStatus == CellFree)
				temp->setStatus(CellFilled);
			temp->setNeedsVia();
            cout << "Via needed at cell (" << currm << "," << currn << "," << currk << ") ---> (" << currm_dbu << "," << currn_dbu << ")" << endl;
            cout << "Via needed at cell (" << currm << "," << currn << "," << currk+1 << ") ---> (" << currm_dbu << "," << currn_dbu << ")" << endl;
			cout << "temp->needsVia(): " << temp->needsVia() << endl;
        }
		else if (currk == 2 && tmpk == 0) { //change from layer 0 to layer 2
            //set via on tmp, which is M1.
            tmp->setNeedsVia();
			Cell * temp = __grid->at(currm, currn, (currk-1));
			CellStatus tempStatus = temp->getStatus();
			//if(tempStatus == CellFree)
				temp->setStatus(CellFilled);
			temp->setNeedsVia();
            cout << "Via needed at cell (" << tmpm << "," << tmpn << "," << tmpk << ") ---> (" << tmpm_dbu << "," << tmpn_dbu << ")" << endl;
			cout << "Via needed at cell (" << currm << "," << currn << "," << currk-1 << ") ---> (" << currm_dbu << "," << currn_dbu << ")" << endl;
			cout << "temp->needsVia(): " << temp->needsVia() << endl;
        }
        
        tmp = curr;
        curr = curr->getBacktrace();
    }
}

void MazeRouter::generateKeepouts(oaUInt4 netID) {
	oaUInt4 dim_m,dim_n,dim_k = 0;
    __grid->getDims(&dim_m,&dim_n,&dim_k);

    //Do by layer first
    for (oaUInt4 k = 0; k < dim_k; k++) {
        for (oaUInt4 m = 0; m < dim_m; m++) {
            for (oaUInt4 n = 0; n < dim_n; n++) {
                Cell* curr = __grid->at(m,n,k);
                CellStatus currStatus = curr->getStatus();
                switch (currStatus) {
                    case CellFilled: //keepout
                    case CellContact:
                    case CellVDDRail:
                    case CellVSSRail:
                        if (curr->getNetID() != netID)
                            generateKeepout(curr);                
                        break;
                    case CellKeepout: //do nothing
                        break;
                    case CellFree:
                        //Special keepouts for side of bounding box. Radius depends on which layer.
                        if (
                            (k == 0 && (m <= __keepoutRadius_bbox_m1 || m >= dim_m-__keepoutRadius_bbox_m1)) ||
                            (k == 1 && (m <= __keepoutRadius_bbox_m2 || m >= dim_m-__keepoutRadius_bbox_m2))
                           ) {
                         //   cout << "Keeping out n: " << n << " k: " << k << endl;
                                curr->setStatus(CellKeepout);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void MazeRouter::generateKeepout(Cell* c) {
    oaUInt4 mtmp,ntmp,ktmp = 0;
    oaInt4 m,n,k = 0;
    oaUInt4 dim_m, dim_n, dim_k = 0;
    c->getPosition(&mtmp,&ntmp,&ktmp);
    m = mtmp;
    n = ntmp;
    k = ktmp;
    __grid->getDims(&dim_m, &dim_n, &dim_k);
    
    oaInt4 leftBound = 0;
    oaInt4 rightBound = 0;
    oaInt4 topBound = 0;
    oaInt4 bottomBound = 0;
    
    Cell* tmp = NULL;
    CellStatus tmpStatus;
    CellStatus cStatus = c->getStatus();
    
    
       //SET LATERAL AND LONGITUDINAL BOUNDS DEPENDING ON LAYER AND LINE END STATUS
    if (k == 0) { //metal1
		if(__rules->getMetal1Direction() == 'V') { //vertical only
			
			leftBound = m-__keepoutRadius_lateral;
			if (leftBound < 0)
				leftBound = 0;
			
			rightBound = m+__keepoutRadius_lateral;
			if (rightBound > dim_m-1)
				rightBound = dim_m-1;
			
			//check line end top condition
			if (n+1 < dim_n) {
				tmp = __grid->at(m,n+1,k); //get cell above
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					topBound = n;
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
						 topBound = n+__keepoutRadius_powerRail;
					else //cell of interest is regular net
						 topBound = n+__keepoutRadius_longitudinal;
				}
				if (topBound > dim_n-1)
					topBound = dim_n-1;
			}
			
			//check line end bottom condition
			if (n-1 >= 0) {
				tmp = __grid->at(m,n-1,k); //get cell below
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					bottomBound = n;
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
							bottomBound = n-__keepoutRadius_powerRail;
					else //cell of interest is regular net
							bottomBound = n-__keepoutRadius_longitudinal;
				}
				if (bottomBound < 0)
					bottomBound = 0;
			}
		}
		else if(__rules->getMetal1Direction() == 'H') { //horizontal only
			bottomBound = n-__keepoutRadius_lateral;
			if (bottomBound < 0)
				bottomBound = 0;
			
			topBound = n+__keepoutRadius_lateral;
			if (topBound > dim_n-1)
				topBound = dim_n-1;
			
			//check line end right condition
			if (m+1 < dim_m) {
				tmp = __grid->at(m+1,n,k); //get cell to right
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					rightBound = m;
				else //line end
					rightBound = m+__keepoutRadius_longitudinal;
				
				if (rightBound > dim_m-1)
					rightBound = dim_m-1;
			}
			
			//check line end left condition
			if (m-1 >= 0) {
				tmp = __grid->at(m-1,n,k); //get cell to left
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					leftBound = m;
				else //line end
					leftBound = m-__keepoutRadius_longitudinal;
				
				if (leftBound < 0)
					leftBound = 0;
			}
		}
		else { //bidirectional 
			Cell* tmptop;
			Cell* tmpbottom;
			Cell* tmpleft;
			Cell* tmpright;
			CellStatus tmptops;
			CellStatus tmpbottoms;
			CellStatus tmplefts;
			CellStatus tmprights;
			bool isVertical = false;
			
			//check cell above
			if (n + 1 < dim_n) {
				tmptop = __grid->at(m, n + 1, k);
				tmptops = tmptop->getStatus();
				if (tmptops == CellFilled || tmptops == CellContact){ //not line end
					topBound = n;
					isVertical = true;
				}
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
						topBound = n + __keepoutRadius_powerRail;
					else //regular cell
						topBound = n + __keepoutRadius_longitudinal;
				}
				if (topBound > dim_n - 1)
					topBound = dim_n - 1;
			}
			//check cell below
			if (n - 1 >= 0) {
				tmpbottom = __grid->at(m, n - 1, k);
				tmpbottoms = tmpbottom->getStatus();
				if (tmpbottoms == CellFilled || tmpbottoms == CellContact){ //not line end
					bottomBound = n;
					isVertical = true;
				}
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
						bottomBound = n - __keepoutRadius_powerRail;
					else
						bottomBound = n - __keepoutRadius_longitudinal;
				}
				if (bottomBound < 0)
					bottomBound = 0;
			}

			//Vertical segment - bounds on left and right sides
			if (isVertical) {
				//left side
				if (m - 1 >= 0) {
					tmpleft = __grid->at(m - 1, n, k);
					tmplefts = tmpleft->getStatus();
					if (tmplefts == CellFilled || tmplefts == CellContact) { //right corner connecting vertical and horizontal segments
						leftBound = m;
					}
					else { //left side is empty
						leftBound = m - __keepoutRadius_lateral;
						if (leftBound < 0)
							leftBound = 0;
					}
				}

				//right side
				if (m + 1 < dim_m) {
					tmpright = __grid->at(m + 1, n, k);
					tmprights = tmpright->getStatus();
					if (tmprights == CellFilled || tmprights == CellContact) { //left corner connecting vertical and horizontal segments
						rightBound = m;
					}
					else { //right side is empty
						rightBound = m + __keepoutRadius_lateral;
						if (rightBound > dim_m - 1)
							rightBound = dim_m - 1;
					}
				}
			}

			//if not verical then it's horizontal
			if (!isVertical) {
				if (m - 1 >= 0) {
					tmpleft = __grid->at(m - 1, n, k);
					tmplefts = tmpleft->getStatus();
					if (tmplefts == CellFilled || tmplefts == CellContact) //not line end
						leftBound = m;
					else //line end
						leftBound = m - __keepoutRadius_longitudinal;
					if (leftBound < 0)
						leftBound = 0;
				}

				if (m + 1 < dim_m) {
					tmpright = __grid->at(m + 1, n, k);
					tmprights = tmpright->getStatus();
					if (tmprights == CellFilled || tmprights == CellContact) //not line end
						rightBound = m;
					else //line end
						rightBound = m + __keepoutRadius_longitudinal;
					if (rightBound > dim_m - 1)
						rightBound = dim_m - 1;
				}
				topBound = n + __keepoutRadius_lateral;
				if (topBound > dim_n - 1)
					topBound = dim_n - 1;
				bottomBound = n - __keepoutRadius_lateral;
				if (bottomBound < 0)
					bottomBound = 0;
			}
		}
	}
	if (k == 1) { //metal2
		if(__rules->getMetal2Direction() == 'V') { //vertical only
			
			leftBound = m-__keepoutRadius_lateral;
			if (leftBound < 0)
				leftBound = 0;
			
			rightBound = m+__keepoutRadius_lateral;
			if (rightBound > dim_m-1)
				rightBound = dim_m-1;
			
			//check line end top condition
			if (n+1 < dim_n) {
				tmp = __grid->at(m,n+1,k); //get cell above
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					topBound = n;
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
						 topBound = n+__keepoutRadius_powerRail;
					else //cell of interest is regular net
						 topBound = n+__keepoutRadius_longitudinal;
				}
				if (topBound > dim_n-1)
					topBound = dim_n-1;
			}
			
			//check line end bottom condition
			if (n-1 >= 0) {
				tmp = __grid->at(m,n-1,k); //get cell below
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					bottomBound = n;
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
							bottomBound = n-__keepoutRadius_powerRail;
					else //cell of interest is regular net
							bottomBound = n-__keepoutRadius_longitudinal;
				}
				if (bottomBound < 0)
					bottomBound = 0;
			}
		}
		else if(__rules->getMetal2Direction() == 'H') { //horizontal only
			bottomBound = n-__keepoutRadius_lateral;
			if (bottomBound < 0)
				bottomBound = 0;
			
			topBound = n+__keepoutRadius_lateral;
			if (topBound > dim_n-1)
				topBound = dim_n-1;
			
			//check line end right condition
			if (m+1 < dim_m) {
				tmp = __grid->at(m+1,n,k); //get cell to right
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					rightBound = m;
				else //line end
					rightBound = m+__keepoutRadius_longitudinal;
				
				if (rightBound > dim_m-1)
					rightBound = dim_m-1;
			}
			
			//check line end left condition
			if (m-1 >= 0) {
				tmp = __grid->at(m-1,n,k); //get cell to left
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					leftBound = m;
				else //line end
					leftBound = m-__keepoutRadius_longitudinal;
				
				if (leftBound < 0)
					leftBound = 0;
			}
		}
		else if(__rules->getMetal2Direction() == 'B') { //bidirectional 
			Cell* tmptop;
			Cell* tmpbottom;
			Cell* tmpleft;
			Cell* tmpright;
			CellStatus tmptops;
			CellStatus tmpbottoms;
			CellStatus tmplefts;
			CellStatus tmprights;
			bool isVertical = false;
			
			//top
			if (n + 1 < dim_n) {
				tmptop = __grid->at(m, n + 1, k);
				tmptops = tmptop->getStatus();
				if (tmptops == CellFilled || tmptops == CellContact){ //not line end
					topBound = n;
					isVertical = true;
				}
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
						topBound = n + __keepoutRadius_powerRail;
					else //regular cell
						topBound = n + __keepoutRadius_longitudinal;
				}
				if (topBound > dim_n - 1)
					topBound = dim_n - 1;
			}
			//bottom
			if (n - 1 >= 0) {
				tmpbottom = __grid->at(m, n - 1, k);
				tmpbottoms = tmpbottom->getStatus();
				if (tmpbottoms == CellFilled || tmpbottoms == CellContact){ //not line end
					bottomBound = n;
					isVertical = true;
				}
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
						bottomBound = n - __keepoutRadius_powerRail;
					else
						bottomBound = n - __keepoutRadius_longitudinal;
				}
				if (bottomBound < 0)
					bottomBound = 0;
			}

			//Vertical segment - bounds on left and right sides
			if (isVertical) {
				//left side
				if (m - 1 >= 0) {
					tmpleft = __grid->at(m - 1, n, k);
					tmplefts = tmpleft->getStatus();
					if (tmplefts == CellFilled || tmplefts == CellContact) { //right corner connecting vertical and horizontal segments
						leftBound = m;
					}
					else { //left side is empty
						leftBound = m - __keepoutRadius_lateral;
						if (leftBound < 0)
							leftBound = 0;
					}
				}

				//right side
				if (m + 1 < dim_m) {
					tmpright = __grid->at(m + 1, n, k);
					tmprights = tmpright->getStatus();
					if (tmprights == CellFilled || tmprights == CellContact) { //left corner connecting vertical and horizontal segments
						rightBound = m;
					}
					else { //right side is empty
						rightBound = m + __keepoutRadius_lateral;
						if (rightBound > dim_m - 1)
							rightBound = dim_m - 1;
					}
				}
			}

			//if not verical then it's horizontal
			if (!isVertical) {
				if (m - 1 >= 0) {
					tmpleft = __grid->at(m - 1, n, k);
					tmplefts = tmpleft->getStatus();
					if (tmplefts == CellFilled || tmplefts == CellContact) //not line end
						leftBound = m;
					else //line end
						leftBound = m - __keepoutRadius_longitudinal;
					if (leftBound < 0)
						leftBound = 0;
				}

				if (m + 1 < dim_m) {
					tmpright = __grid->at(m + 1, n, k);
					tmprights = tmpright->getStatus();
					if (tmprights == CellFilled || tmprights == CellContact) //not line end
						rightBound = m;
					else //line end
						rightBound = m + __keepoutRadius_longitudinal;
					if (rightBound > dim_m - 1)
						rightBound = dim_m - 1;
				}
				topBound = n + __keepoutRadius_lateral;
				if (topBound > dim_n - 1)
					topBound = dim_n - 1;
				bottomBound = n - __keepoutRadius_lateral;
				if (bottomBound < 0)
					bottomBound = 0;
			}
		}
	}
	if (k == 2) { //metal3
		if(__rules->getMetal3Direction() == 'V') { //vertical only
			
			leftBound = m-__keepoutRadius_lateral;
			if (leftBound < 0)
				leftBound = 0;
			
			rightBound = m+__keepoutRadius_lateral;
			if (rightBound > dim_m-1)
				rightBound = dim_m-1;
			
			//check line end top condition
			if (n+1 < dim_n) {
				tmp = __grid->at(m,n+1,k); //get cell above
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					topBound = n;
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
						 topBound = n+__keepoutRadius_powerRail;
					else //cell of interest is regular net
						 topBound = n+__keepoutRadius_longitudinal;
				}
				if (topBound > dim_n-1)
					topBound = dim_n-1;
			}
			
			//check line end bottom condition
			if (n-1 >= 0) {
				tmp = __grid->at(m,n-1,k); //get cell below
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					bottomBound = n;
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
							bottomBound = n-__keepoutRadius_powerRail;
					else //cell of interest is regular net
							bottomBound = n-__keepoutRadius_longitudinal;
				}
				if (bottomBound < 0)
					bottomBound = 0;
			}
		}
		else if(__rules->getMetal3Direction() == 'H') { //horizontal only
			bottomBound = n-__keepoutRadius_lateral;
			if (bottomBound < 0)
				bottomBound = 0;
			
			topBound = n+__keepoutRadius_lateral;
			if (topBound > dim_n-1)
				topBound = dim_n-1;
			
			//check line end right condition
			if (m+1 < dim_m) {
				tmp = __grid->at(m+1,n,k); //get cell to right
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					rightBound = m;
				else //line end
					rightBound = m+__keepoutRadius_longitudinal;
				
				if (rightBound > dim_m-1)
					rightBound = dim_m-1;
			}
			
			//check line end left condition
			if (m-1 >= 0) {
				tmp = __grid->at(m-1,n,k); //get cell to left
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					leftBound = m;
				else //line end
					leftBound = m-__keepoutRadius_longitudinal;
				
				if (leftBound < 0)
					leftBound = 0;
			}
		}
		else if(__rules->getMetal3Direction() == 'B') { //bidirectional 
			leftBound = m-__keepoutRadius_lateral;
			if (leftBound < 0)
				leftBound = 0;
			
			rightBound = m+__keepoutRadius_lateral;
			if (rightBound > dim_m-1)
				rightBound = dim_m-1;
			
			//check line end top condition
			if (n+1 < dim_n) {
				tmp = __grid->at(m,n+1,k); //get cell above
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					topBound = n;
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
						 topBound = n+__keepoutRadius_powerRail;
					else //cell of interest is regular net
						 topBound = n+__keepoutRadius_longitudinal;
				}
				if (topBound > dim_n-1)
					topBound = dim_n-1;
			}
			
			//check line end bottom condition
			if (n-1 >= 0) {
				tmp = __grid->at(m,n-1,k); //get cell below
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					bottomBound = n;
				else { //line end
					if (cStatus == CellVDDRail || cStatus == CellVSSRail) //cell of interest is power rail
							bottomBound = n-__keepoutRadius_powerRail;
					else //cell of interest is regular net
							bottomBound = n-__keepoutRadius_longitudinal;
				}
				if (bottomBound < 0)
					bottomBound = 0;
			}
			
			bottomBound = n-__keepoutRadius_lateral;
			if (bottomBound < 0)
				bottomBound = 0;
			
			topBound = n+__keepoutRadius_lateral;
			if (topBound > dim_n-1)
				topBound = dim_n-1;
			
			//check line end right condition
			if (m+1 < dim_m) {
				tmp = __grid->at(m+1,n,k); //get cell to right
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					rightBound = m;
				else //line end
					rightBound = m+__keepoutRadius_longitudinal;
				
				if (rightBound > dim_m-1)
					rightBound = dim_m-1;
			}
			
			//check line end left condition
			if (m-1 >= 0) {
				tmp = __grid->at(m-1,n,k); //get cell to left
				tmpStatus = tmp->getStatus();
				if (tmpStatus == CellFilled || tmpStatus == CellContact) //not line end
					leftBound = m;
				else //line end
					leftBound = m-__keepoutRadius_longitudinal;
				
				if (leftBound < 0)
					leftBound = 0;
			}
		}
	}
    
	oaInt4 newBottom = bottomBound;
	oaInt4 newRight = rightBound;
	oaInt4 newLeft = leftBound;
	oaInt4 newTop = topBound;
	double increase = 1;
	double decrease = 1;
	if(k == 0 && __rules->getMetal1Direction() == 'B') {
		newBottom *= decrease;
		newRight *= increase;
		newLeft *= decrease;
		newTop *= increase;
	}
	else if(k == 1 && __rules->getMetal2Direction() == 'B') {
		newBottom *= decrease;
		newRight *= increase;
		newLeft *= decrease;
		newTop *= increase;
	}
	else if(k == 2 && __rules->getMetal3Direction() == 'B') {
		newBottom *= decrease;
		newRight *= increase;
		newLeft *= decrease;
		newTop *= increase;
	}
	
	if (newTop > dim_n-1)
		newTop = dim_n-1;
	if (newRight > dim_m-1)
		newRight = dim_m-1;
	if (newBottom < 0)
		newBottom = 0;
	if (newLeft < 0)
		newLeft = 0;

    /*for (oaInt4 j = topBound; j >= bottomBound; j--) {
        for (oaInt4 i = leftBound; i <= rightBound; i++) {
            tmp = __grid->at(i,j,k);
            if (tmp->getStatus() == CellFree) {      
                tmp->setStatus(CellKeepout);
            }
        }
    }*/
	 for (oaInt4 j = newTop; j >= newBottom; j--) {
        for (oaInt4 i = newLeft; i <= newRight; i++) {
            tmp = __grid->at(i,j,k);
            if (tmp->getStatus() == CellFree) {      
                tmp->setStatus(CellKeepout);
            }
        }
    }
}
