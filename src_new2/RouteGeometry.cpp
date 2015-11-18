/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */


#include "RouteGeometry.h"
#include "Definitions.h"
#include "ProjectDesignRules.h"
#include <iostream>
using namespace std;

RouteGeometry::RouteGeometry()
{
}

RouteGeometry::~RouteGeometry()
{
}

/*
//Weiche
void RouteGeometry::mazeToGeometry(Grid* grid, oaDesign* design,
        ProjectDesignRules dr)
{
     oaNativeNS	ns;
    //Now, each continuous horizontal route will be created as a continuous path
    //segment
    //a jump from m1 to m2 should create a via

    oaBlock* topBlock = design->getTopBlock();
    //get dimensions of grid
    unsigned int nLayers, nHorizTiles, nVerticTiles;
    grid->getDims(&nHorizTiles, &nVerticTiles, &nLayers);
   
    for (int layer = 0; layer < nLayers; layer++)
    {
        
        oaLayerNum layNum = METAL_LAYERS_INFO[layer].layerNum; //LPPHeader->getLayerNum();
        oaPurposeNum purpNum = 1; //LPPHeader->getPurposeNum();

        //first dimension is the dimension perpendicular to routing layer direction
        //and second dimension is the dimension along routing layer direction
        int nTilesFirstDim, nTilesSecondDim;

        METAL_LAYER_INFO layerInfo = METAL_LAYERS_INFO[layer];
        int extFromCenterX, extFromCenterY;
        int wireWidth = dr.getMetalWidthRule();
        unsigned int w, h;
        grid->getCellDims(&w, &h);
        if (layerInfo.vertical)
        {
            nTilesFirstDim = nHorizTiles;
            nTilesSecondDim = nVerticTiles;
           
        } else
        {
            nTilesFirstDim = nVerticTiles;
            nTilesSecondDim = nHorizTiles;
        }
        extFromCenterX = static_cast<int>(ceil(wireWidth / 2));
        extFromCenterY = static_cast<int>(ceil(wireWidth / 2));
        for (int firstDim = 0; firstDim < nTilesFirstDim; firstDim++)
        {
            //Now I want to get the longest possible segments from the 
            //second dimension
            int secDim = 0;
            oaInt4 currNetID=-1;
            while (secDim < nTilesSecondDim)
            {
                //create a path segment representing every few successive
                //tiles along the routing layer direction

                //1-Skip non-occupied cells till we get to first filled cell
                Cell* currCell;
                do{
                    currCell = GetCell(grid, layerInfo, firstDim, secDim, layer);
                    secDim++;
                }
                while (!(currCell->getStatus() == CellFilled|| currCell->getStatus() == CellContact)
                      && secDim < nTilesSecondDim);
                if (currCell->getStatus() == CellFilled|| currCell->getStatus() == CellContact)//check that we found filled cell
                    //and not that we went out of bounds
                {
                    int left, bottom, right, top;
                    //Now we have found the first filled cell==>Get its center             
                    //Found start point: get left and bottom of rectangle starting point
                    int xCenter, yCenter;
                    currCell->getAbsolutePosition(&xCenter, &yCenter);
                                     
                    oaNet* currNet=NULL;
                     //create net
                    char netNameCharP[25];
                    snprintf(netNameCharP, sizeof (currCell->getNetID()), "%d", 
                             currCell->getNetID());

                    oaName netName(ns,netNameCharP);
                    currNet=oaNet::find(topBlock, netName);
                    if(!currNet)//if not already created
                         currNet=oaNet::create(topBlock, netName);
                   
                    left = xCenter - extFromCenterX;
                    bottom = yCenter - extFromCenterY;
                    Cell* firstCell=currCell;
                    //Now go till the last filled cell
                    do
                    {
                        currNetID=currCell->getNetID();//currCell starts at the first cell
                        int tempX, tempY;
                        currCell->getAbsolutePosition(&tempX,&tempY);
                        if(currCell->needsVia())
                        {
                            oaRect* via=createVia(currCell, dr,design);
                            if(currNet==NULL)
                                cerr<<"currNet is NULL"<<endl;
                            else via->addToNet(currNet);
                        }
                       
                        //if pin, create label
                        if (currCell->isPin())
                        {                         
                            //Create pin label
                            CreatePinTextLabel(currCell, design,dr);
                        }
                        if(secDim==nTilesSecondDim) break;
                        currCell = GetCell(grid, layerInfo, firstDim, secDim, layer);
                        secDim++;
                    }
                    while ((currCell->getStatus() == CellFilled
                           || currCell->getStatus() == CellContact)
                           && currCell->getNetID()==firstCell->getNetID());//same net)
               
                    //Now we have found the first empty cell after this segment
                    //endpoint should be the last cell
                    if(secDim==nTilesSecondDim)
                    {     
                        currCell = GetCell(grid, layerInfo, firstDim, secDim-1, layer);   
                    }
                    else //if we stopped because we found different net or because found an empty cell,
                        //then backup to the last cell
                    {
                        currCell = GetCell(grid, layerInfo, firstDim, secDim - 2, layer);
                        secDim--;
                    }

                    currCell->getAbsolutePosition(&xCenter, &yCenter);
                    right = xCenter + extFromCenterX;
                    top = yCenter + extFromCenterY;

                    //Create the wiring segment
                    cout << "coords: " << left << " " << bottom << " " << right << " " << top << endl;

                    oaRect* newSeg = oaRect::create(topBlock, layNum, purpNum,
                            oaBox(left, bottom, right, top));
                    if (currNet == NULL)
                        cerr << "currNet is NULL after creation of newSeg" << endl;
                    else newSeg->addToNet(currNet);
                }
            }
        }
    }
}
*/

void RouteGeometry::mazeToGeometry(Grid* grid, oaDesign* design,
        ProjectDesignRules dr)
{
     oaNativeNS	ns;
    //Now, each continuous horizontal route will be created as a continuous path
    //segment
    //a jump from m1 to m2 should create a via

    oaBlock* topBlock = design->getTopBlock();
    //get dimensions of grid
    unsigned int nLayers, nHorizTiles, nVerticTiles;
    grid->getDims(&nHorizTiles, &nVerticTiles, &nLayers);
   
    for (int layer = 0; layer < nLayers; layer++)
    {
        //find layer and purpose number
        //ASSUME one purpose number
        oaLayerHeader* layHead = oaLayerHeader::find(topBlock,
                METAL_LAYERS_INFO[layer].layerNum);
        //        oaIter<oaLPPHeader> LPPHeaderIter(layHead->getLPPHeaders());
        //        oaLPPHeader * LPPHeader=LPPHeaderIter.getNext();
        //        if(LPPHeader==NULL)
        //        {
        //            cerr<<"No LPP headers found in this layer!"<<endl;
        //            return;
        //        }
        oaLayerNum layNum = METAL_LAYERS_INFO[layer].layerNum; //LPPHeader->getLayerNum();
        oaPurposeNum purpNum = 1; //LPPHeader->getPurposeNum();

        //first dimension is the dimension perpendicular to routing layer direction
        //and second dimension is the dimension along routing layer direction
        int nTilesFirstDim, nTilesSecondDim;

        METAL_LAYER_INFO layerInfo = METAL_LAYERS_INFO[layer];
        int extFromCenterX, extFromCenterY;
        int wireWidth = dr.getMetalWidthRule();
        unsigned int w, h;
        grid->getCellDims(&w, &h);
        if (layerInfo.vertical)
        {
            nTilesFirstDim = nHorizTiles;
            nTilesSecondDim = nVerticTiles;
            extFromCenterX = static_cast<int>(ceil(wireWidth / 2));
            extFromCenterY = extFromCenterX;//ceil(h / 2);
        } else
        {
            nTilesFirstDim = nVerticTiles;
            nTilesSecondDim = nHorizTiles;
            
            extFromCenterY = static_cast<int>(ceil(wireWidth / 2));
            extFromCenterX = extFromCenterY;//ceil(h / 2);
        }

        for (int firstDim = 0; firstDim < nTilesFirstDim; firstDim++)
        {
            //Now I want to get the longest possible segments from the 
            //second dimension
            int secDim = 0;
            oaInt4 currNetID=-1;
            while (secDim < nTilesSecondDim)
            {
                //create a path segment representing every few successive
                //tiles along the routing layer direction

                //1-Skip non-occupied cells till we get to first filled cell
                Cell* currCell = GetCell(grid, layerInfo, firstDim, secDim, layer);
                while (!(currCell->getStatus() == CellFilled

                       || currCell->getStatus() == CellContact
                       )
                       && secDim < nTilesSecondDim)
		                      
{
                    currCell = GetCell(grid, layerInfo, firstDim, secDim, layer);
                    secDim++;
                  	if(currCell->getStatus() == CellVDDRail || currCell->getStatus() == CellVSSRail) {
						 oaNet* currNet=NULL;
						//create net
						char netNameCharP[25];
						snprintf(netNameCharP, sizeof (currCell->getNetID()), "%d", 
                             currCell->getNetID());

						oaName netName(ns,netNameCharP);
						currNet=oaNet::find(topBlock, netName);
						if(!currNet)//if not already created
							 currNet=oaNet::create(topBlock, netName);
						 if(currCell->needsVia())
                        {
                             oaRect* via=createVia(currCell, dr,design);
                            if(currNet==NULL)
                                cerr<<"currNet is NULL"<<endl;
                            else via->addToNet(currNet);
                        }
					}	

                }
                if (secDim < nTilesSecondDim)//check that we found filled cell
                    //and not that we went out of bounds
                {
                    int left, bottom, right, top;
                    //Now we have found the first filled cell==>Get its center             
                    //Found start point: get left and bottom of rectangle starting point
                    int xCenter, yCenter;
                    currCell->getAbsolutePosition(&xCenter, &yCenter);
					//std::cout<<"xCenter="<<xCenter<<" yCenter="<<yCenter<<endl; //Weiche
                    
                    bool viaExtensionNeeded = isViaExtNeeded(currCell,grid);
                    
                    oaNet* currNet=NULL;
                     //create net
                    char netNameCharP[25];
                    snprintf(netNameCharP, sizeof (currCell->getNetID()), "%d", 
                             currCell->getNetID());

                    oaName netName(ns,netNameCharP);
                    currNet=oaNet::find(topBlock, netName);
                    if(!currNet)//if not already created
                         currNet=oaNet::create(topBlock, netName);
						 
					bool via_is_set = false;
					if(currCell->needsVia())
                    {  
						oaRect* via=createVia(currCell, dr,design);
                        if(currNet==NULL)
							cerr<<"currNet is NULL"<<endl;
                        else {
							via->addToNet(currNet);
							//via_is_set = true;
						}
                    }
                    /*If the first cell is a contact, then extend with contact width*/
                    /* also handle via extension*/
                    if (currCell->getStatus() == CellContact
                        || viaExtensionNeeded
                        )
                        //|| isBackTraceOfCellAdjLayer(currCell, grid,2,
                        //                                cellAdjLayer))//has via below/above it)
                    {
                          //Check if contact is part of vertical segment
						Cell* tmp;
						bool vertical;
						if(secDim - 1 > 0) {
							tmp = GetCell(grid, layerInfo, firstDim, secDim - 1, layer);
							if(tmp->getStatus() == CellFilled)
								vertical = true;
						}
						if(secDim + 1 < nTilesSecondDim) {
							tmp = GetCell(grid, layerInfo, firstDim, secDim + 1, layer);
							if(tmp->getStatus() == CellFilled)
								vertical = true;
						}
                    
                        if (vertical || METAL_LAYERS_INFO[layer].vertical)
                        {
                            bottom = yCenter - dr.getViaDimensionRule() / 2
                                    - dr.getContactViaExtensionRule();
                            left = xCenter - extFromCenterX;
                        } else
                        {
                            left = xCenter - dr.getViaDimensionRule() / 2
                                    - dr.getContactViaExtensionRule();
                            bottom = yCenter - extFromCenterY;
                        }
                    } else//not contact,
                        //if not pin then just extend with wirewidth/2 and half cell size 
                    {
                        left = xCenter - extFromCenterX;
                        bottom = yCenter - extFromCenterY;
                    }
                    bool foundDiffNet=false;
                    //Now go till the last filled cell  
					int first_loop = 0;
                    while ((currCell->getStatus() == CellFilled
                           || currCell->getStatus() == CellContact
                        )
                           && secDim < nTilesSecondDim
                            &&(currNetID==-1||currCell->getNetID()==currNetID ))
                    {
                        
                        currNetID=currCell->getNetID();
                        int tempX, tempY;
                        currCell->getAbsolutePosition(&tempX,&tempY);
                        if(first_loop != 0) {
							if(currCell->needsVia())
							{
								
								 oaRect* via=createVia(currCell, dr,design);
								if(currNet==NULL)
									cerr<<"currNet is NULL"<<endl;
								else via->addToNet(currNet);
							}
                        }
                        //if pin, create label
                        if (currCell->isPin())
                        {                         
                            //Create pin label
                            CreatePinTextLabel(currCell, design,dr);
                        }
                        currCell = GetCell(grid, layerInfo, firstDim, secDim, layer);
                        secDim++;
                        first_loop++;
                    }
                    //Now we have found the first empty cell after this segment
                    //endpoint should be the last cell
                    if(currCell->getNetID()!=currNetID)
                    {     
                        currCell = GetCell(grid, layerInfo, firstDim, secDim, layer);   
                        secDim++;
                    }
                    else currCell = GetCell(grid, layerInfo, firstDim, secDim - 2, layer);
                   
                    currCell->getAbsolutePosition(&xCenter, &yCenter);
                   
                    viaExtensionNeeded=isViaExtNeeded(currCell, grid);
//                    //only create via if the segment length is more than one
//                    //cell long (or otherwise will be duplicate)
//                    if(viaNeeded && firstCell!=lastCell)
//                        createVia(currCell,grid, design, dr);

                    /*If the last cell is a contact, then extend with contact width/2
                     and via/contact extension extension*/
                    //OR if via has been inserted
                   
                    if (currCell->getStatus() == CellContact || viaExtensionNeeded
                        //using Mark's new method
                        //   || isBackTraceOfCellAdjLayer(currCell, grid,2,cellAdjLayer)//has via below/above it
                        )
                    {
                        Cell* tmp;
						bool vertical;
						if(secDim - 1 > 0) {
							tmp = GetCell(grid, layerInfo, firstDim, secDim - 1, layer);
							if(tmp->getStatus() == CellFilled)
								vertical = true;
						}
						if(secDim + 1 < nTilesSecondDim) {
							tmp = GetCell(grid, layerInfo, firstDim, secDim + 1, layer);
							if(tmp->getStatus() == CellFilled)
								vertical = true;
						}
						if (vertical || METAL_LAYERS_INFO[layer].vertical)
                        {
                            //extend by half via and extension
                            top = yCenter + dr.getViaDimensionRule() / 2
                                    + dr.getContactViaExtensionRule();
                            right = xCenter + extFromCenterX;
                        } else
                        {
                            right = xCenter + dr.getViaDimensionRule() / 2
                                    + dr.getContactViaExtensionRule();
                            top = yCenter + extFromCenterY;
                        }
                    } else//not contact 
                    {

                        right = xCenter + extFromCenterX;
                        top = yCenter + extFromCenterY;
                    }

                    
                    //Create the wiring segment
                    //cout<<"coords: "<<left<<" "<<bottom<<" "<<right<<" "<<top<<endl;  //Weiche
                    //cout<<"currCellID: "<<currCell->getNetID()<<endl; //Weiche
					oaRect* newSeg=oaRect::create(topBlock, layNum, purpNum, 
                                                oaBox(left, bottom, right, top));
					if(currNet==NULL)
                                cerr<<"currNet is NULL after creation of newSeg"<<endl;
                    else newSeg->addToNet(currNet);
                   

                }
            }
        }
    }
}
//Function returns cell at the given indices. However the indices are given 
//as first dimension (perpendicular to routing direction)
//and second dimension (parallel to routing direction)

Cell* RouteGeometry::GetCell(Grid* grid, METAL_LAYER_INFO layerInfo,
        int firstDimIndex, int secDimIndex,
        int layerIndex)
{
    if (layerInfo.vertical)
    {
        //vertical so first dim is horiz, and second dim is vertical
        return grid->at(firstDimIndex, secDimIndex, layerIndex);
    } else
    {
        //horizontal so first dim is vertic, and second dim is horiz
        return grid->at(secDimIndex, firstDimIndex, layerIndex);
    }
}

//bool RouteGeometry::CreateViaIfNeeded(Cell* currCell, Grid* grid,oaDesign* design,
//        ProjectDesignRules dr)
//{
//    
//    bool viaRequired =false;
//    unsigned int h, v, layer;
//    currCell->getPosition(&h, &v, &layer);
//   
//    //if this is filled,
//    //and its trace_back is on different metal layer, then add via
//     unsigned int m, n, k;
//    if (currCell->getStatus() == CellFilled||currCell->getStatus()==CellContact)
//    {
//       
//        Cell* backTrace = currCell->getBacktrace();
//        if (backTrace != NULL)
//        {
//            backTrace->getPosition(&m, &n, &k);
//            if (k != layer)//if coming from a tile of lower metal layer
//            {
//                viaRequired = true;
//            }    
//        }
//        if(!viaRequired)//if not already set
//            //Other possibility: this is the backtrack of a cell in adjacent layer
//            //to make sure via is not created twice, only create via now
//            //if the other cell is not an endpoint (i,e. not checked for 
//            //via creation)
//        {
//            //get the cell below and above it and check if its backtrace is this cell
//            int lowAbove=0;
//            Cell* lowerCell;
//            if(isBackTraceOfCellAdjLayer(currCell, grid,lowAbove,lowerCell))
//            {
//               if(!(isEndpointOfSegment(lowerCell,grid)))
//                    viaRequired=true;
//                 //   cout<<"will create via"<<endl;
//            }
//            lowAbove=1;
//            Cell* aboveCell;
//            if(isBackTraceOfCellAdjLayer(currCell, grid,lowAbove,aboveCell))
//            {
//               if(!(isEndpointOfSegment(aboveCell,grid)))
//                    viaRequired=true;
//                    //cout<<"Will create via"<<endl;
//                    
//            }
//        }
//    }
//    
//    if (viaRequired)
//    {
//        //decide on via layer
//        //if more via layers, this will need to be changed
//        int viaLayerIndex = layer / 2; //layers 0 and 1 map to 0th via layer, then 1..
//        //Create via
//        int viaLeft, viaBottom, viaRight, viaTop;
//        int x, y;
//        currCell->getAbsolutePosition(&x, &y);
//        viaLeft = x - ceil(dr.getViaDimensionRule() / 2);
//        viaRight = x + (dr.getViaDimensionRule() / 2);
//        viaBottom = y - ceil(dr.getViaDimensionRule() / 2);
//        viaTop = y + (dr.getViaDimensionRule() / 2);
//        oaRect::create(design->getTopBlock(), VIA_LAYER_NUMBERS[viaLayerIndex],
//                PURPOSE_NUM, oaBox(viaLeft, viaBottom, viaRight, viaTop));
//       
//    }
//     //testing code, leave it for now
////     Cell* c=grid->atXY(14225,2275,0);
////     cout<<"status: "<<c->getStatus()<<endl;
////     if(isEndpointOfSegment(c,grid))
////     {
////         cout<<"is endpoint"<<endl;
////         getchar();
////     }
////     else
////     {
////         cout<<"is not endpoint "<<endl;
////         getchar();
////                 
////     }
////     Cell* test=c->getBacktrace();
////     if(test==NULL)
////     {
////         cout<<"test is NULL"<<endl;
////         getchar();
////     }
////    unsigned  int x,y,z;
////     test->getPosition(&x,&y,&z);
////     if(z==0)
////     {   cout<<"Backtrace is 0"<<endl;
////         getchar();
////     }
////     else
////     {
////          cout<<"Backtrace is 1"<<endl;
////         getchar();
////     }
//    
//    return viaRequired;
//}
//bool RouteGeometry::CreateViaIfNeeded(Cell* currCell, Grid* grid, oaDesign* design,
//                                      ProjectDesignRules dr)
//{
//    bool viaNeeded=false;
//    viaNeeded=isViaExtNeeded(currCell, grid);
//        
//    if(viaNeeded)
//    {
//        createVia(currCell, dr);
//    }
//    return viaNeeded;
//}
//Function checks if the given cell is the backtrace of another cell at same position
//but adjacent layer. Used to decide whether a via exists beside it or not
//lowerAboveBoth:
//      0==>search lower layer only
//      1==>search upper layer only
//      2==>search lower and upper layers
//Return the cell in adj layer in lowerAboveCell (other wise NULL))
bool RouteGeometry::isBackTraceOfCellAdjLayer(Cell* cell, Grid* grid,
                                              int lowerAboveBoth,
                                              Cell*& lowerAboveCell)
{
    lowerAboveCell=NULL;
    //Get m,n,currLayer of cell
    unsigned int m, n, currLayer;
    cell->getPosition(&m, &n, &currLayer);
    //check below
    Cell* below = NULL, *above = NULL;
    if ((currLayer != 0) //if not most bottom layer
         && (lowerAboveBoth==0 || lowerAboveBoth==2))//and flag is above or both
    {
        below = grid->at(m, n, currLayer - 1);
        if (below->getBacktrace() == cell)
        {
            lowerAboveCell=below;
            return true;
        }
    }
    unsigned int nHoriz, nVert, nLayers;
    grid->getDims(&nHoriz, &nVert, &nLayers);
    //check above
    if ((currLayer < nLayers - 1) //if not top most layer
         &&(lowerAboveBoth==1 || lowerAboveBoth==2))//and flag is above or both
    {
        above = grid->at(m, n, currLayer + 1);
        
        if (above->getBacktrace() == cell)
        {
            lowerAboveCell=above;
            return true;
        }
    }
    return false;

}

void RouteGeometry::CreatePinTextLabel(Cell* cell, oaDesign* design,
                                        ProjectDesignRules& dr)
{
    oaBlock* topBlock = design->getTopBlock();
    int pinCenterX, pinCenterY;
    cell->getAbsolutePosition(&pinCenterX, &pinCenterY); //get center
    unsigned int m, n, layer;
    cell->getPosition(&m, &n, &layer);//get indices of this cell
    
    oaPoint textLoc(pinCenterX, 
            pinCenterY);
    oaText::create(topBlock, METAL_LAYERS_INFO[layer].layerNum, PURPOSE_NUM,
            cell->getPinName(),
            textLoc,
            oaTextAlign(oacLowerLeftTextAlign), oaOrient(oacR0),
            oaFont(oacRomanFont), oaDist(1000), false, true, true);
    

}
/*
 * Given this segment, check if it's an endpoint of a segment
 */
bool RouteGeometry::isEndpointOfSegment(Cell* cell, Grid* grid)
{
    //to be an enpoint:
    /*either
     * 1- single cell
     * 2-has a neigbor from either its left or right (if horiz layer)
     * 3-has as neighbor from either its top or bottom (if vertic layer)
     */
    unsigned int m,n,k;
    cell->getPosition(&m,&n,&k);
    unsigned int width, height, nLayers;
    grid->getDims(&width,&height,&nLayers);
    //check if it lies on a  horiz or vertical layer
    bool vertical;
    if(METAL_LAYERS_INFO[k].vertical)
    {
        vertical=true;
        //if it's first cell or last then it's an endpoint
        if(n==0||n==height-1)
            return true;
         //check below
        bool belowFilled=false;
        Cell* below=grid->at(m,n-1,k);
        if(below->getStatus()==CellContact || below->getStatus()==CellFilled)
        {
            belowFilled=true;
        }    
         //check above
        bool aboveFilled=false;
        Cell* above=grid->at(m,n+1,k);
        if(above->getStatus()==CellContact || above->getStatus()==CellFilled)
        {
            aboveFilled=true;
        }   
        //if single lonely cell
        if(!(aboveFilled||belowFilled))
            return true;
        if(aboveFilled^belowFilled)//if only one of them filled
            return true;        
    }
    else
    {
        vertical=false;
        if(m==0 ||m==width-1)//if first or last
            return true;
         //check left
        bool leftFilled=false;
        Cell* left=grid->at(m-1,n,k);
        if(left->getStatus()==CellContact || left->getStatus()==CellFilled)
        {
            leftFilled=true;
        }    
         //check right
        bool rightFilled=false;
        Cell* right=grid->at(m+1,n,k);
        if(right->getStatus()==CellContact || right->getStatus()==CellFilled)
        {
            rightFilled=true;
        }   
        //if single lonely cell
        if(!(rightFilled||leftFilled))
            return true;
        if(rightFilled^leftFilled)//if only one of them filled
            return true; 
    }
    return false;    
}
/*
 * This function checks the cell below this cell (i.e. in lower metal layer)
 * and checks its viaNEeded flag and returns it
 */
bool RouteGeometry::hasViaBelow(Cell* currCell, Grid* grid)
{
    //Get m,n,currLayer of cell
    unsigned int m, n, currLayer;
    currCell->getPosition(&m, &n, &currLayer);
    //check below
    Cell* below = NULL, *above = NULL;
    if (currLayer != 0) //if not most bottom layer    
    {
        below = grid->at(m, n, currLayer - 1);
        return below->needsVia();
    }
    return false;
}
//Function only checks if via needs to be created from this cell
//Via needed if this is an M1 cell that has viaNEeded flag on
//or if M2 but M1 cell below has viaNeeded flag on, but 
//the M1 cell is not an enpoint of a segment (so will not be checked for 
//the need of via)
bool RouteGeometry::isViaExtNeeded(Cell* currCell, Grid* grid)
{
     bool viaNeeded=false;
     if(currCell->needsVia())
        viaNeeded=true;
    //other case if this is on M2, then the cell below it on M1 will say if 
    //it needs a via. 
     else
         viaNeeded=hasViaBelow(currCell, grid);
    return viaNeeded;
}

oaRect* RouteGeometry::createVia(Cell* currCell, ProjectDesignRules dr,
                              oaDesign* design)
{
        //decide on via layer
        //if more via layers, this will need to be changed
         unsigned int h, v, layer;
        currCell->getPosition(&h, &v, &layer);
        //int viaLayerIndex = layer / 2; //layers 0 and 1 map to 0th via layer, then 1..
        int viaLayerIndex;
		if(layer == 0)
			viaLayerIndex = 0;
		else
			viaLayerIndex = 1;
        //Create via
        int viaLeft, viaBottom, viaRight, viaTop;
        int x, y;
        currCell->getAbsolutePosition(&x, &y);
       
        
        viaLeft = x - static_cast<int>(ceil(dr.getViaDimensionRule() / 2));
        viaRight = x + static_cast<int>(dr.getViaDimensionRule() / 2);
        viaBottom = y - static_cast<int>(ceil(dr.getViaDimensionRule() / 2));
        viaTop = y + static_cast<int>(dr.getViaDimensionRule() / 2);
           
        
        return oaRect::create(design->getTopBlock(), VIA_LAYER_NUMBERS[viaLayerIndex],
                PURPOSE_NUM, oaBox(viaLeft, viaBottom, viaRight, viaTop));
}
