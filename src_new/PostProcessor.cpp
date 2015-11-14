/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include "PostProcessor.h"
#include "Definitions.h"
#include "ShapesSaverQuery.h"
#include "ProjectDesignRules.h"
#include "OAHelper.h"
#include <iostream>
using namespace std;

PostProcessor::PostProcessor()
{
}

void PostProcessor::Postprocess(Grid* grid, oaDesign* design,
        ProjectDesignRules dr)
{
    //do postprocessing 
    //Go to layer 1 and layer 2
    //for each segment do reqion query on the boundign box of the 
    //polygons
    //if region query returns polygons other than the current polygon
    //then merge them
    oaBlock* topBlock = design->getTopBlock();
    //get dimensions of grid
    unsigned int nLayers, nHorizTiles, nVerticTiles;
    grid->getDims(&nHorizTiles, &nVerticTiles, &nLayers);
    int k = 0;
    for(int layer=0;layer<nLayers;layer++)
    {
        //find layer and purpose number
        //ASSUME one purpose number
        oaLayerHeader* layHead = oaLayerHeader::find(topBlock,
                METAL_LAYERS_INFO[layer].layerNum);
        if (layHead != NULL)//if layer found
        {
            oaLayerNum layNum = METAL_LAYERS_INFO[layer].layerNum; //LPPHeader->getLayerNum();
            oaPurposeNum purpNum = 1; //LPPHeader->getPurposeNum();
            oaIter<oaLPPHeader> headers(layHead->getLPPHeaders());
            while (oaLPPHeader * lppHeader = headers.getNext())
            {
                //oaLayerNum lppLayerNum = lppHeader->getLayerNum();
                // oaPurposeNum lppPurpNum = lppHeader->getPurposeNum();
                // if (lppLayerNum == layerNum)
                {
                    if (!lppHeader->getShapes().isEmpty())
                    {
                        oaIter<oaShape> shapes(lppHeader->getShapes());
                        while (oaShape * shape = shapes.getNext())
                        {
                            oaType type = shape->getType();
                            oaRect *rect = NULL;
                            if (type.getName() == "Polygon")
                            {
                                rect = ((oaPolygon*) shape)->convertToRect();
                            } else if (type.getName() == "Rect")
                            {
                                rect = ((oaRect*) shape);
                            }
                            //Check that this is not power rail
                            if (rect != NULL)
                            {
                                if (isPowerRail(rect))
                                    continue;
                                //Now check if any other polygon overlaps with this rect
                                //or is at distance less than min spacing
                                oaBox forbiddenRegion;
                                getForbiddenRegion(rect, forbiddenRegion, dr.getMetalSpaceRule());
                                ShapesSaverQuery ssq;
                                ssq.query(design, layNum, purpNum, forbiddenRegion);
                                //Now we have shapes that cut this forbidden region
                                //Need to check that they not only touch:
                                // *** either they violate dist (close to each other) or
                                // *** they overlap
                                vector<oaShape*> shapes = ssq.getShapes();
                                //if only one shape returned then skip it
                                if (shapes.size() == 1)
                                    continue;
                                oaBox mergedBBox;
                                oaNet* currNet = rect->getNet();
                                if (MergeShapes(rect, shapes, design, mergedBBox,
                                    dr.getMetalSpaceRule(),
                                    METAL_LAYERS_INFO[layer].vertical))
                                {
                                    //Don't use rect, it will have been destroyed
                                    oaRect* newRect = oaRect::create(topBlock, layNum, purpNum, mergedBBox);

                                    if (currNet != NULL)
                                        newRect->addToNet(currNet); //add to net
                                    //remove unnecessary segments from other metal layer
                                    //that were used to connect these merged polygons
                                    //if any
                                    //if(layer==0)//if this is layer 0, try to remove unnecessary segments on M2
                                        //if M2, don't try to remove from M1, because we need to consider contacts
                                        // and it's more complicated
                                    //Call it for M1 and M2. for M2 it will only 
                                    //check overlapping vias
                                   
                                     removeUnnecessarySegments(design, mergedBBox, currNet, layer, dr);

                                }
                                k++;
                            
                            }

                        }
                    }
                }
            }
        }
        // break;
    }

}

PostProcessor::~PostProcessor()
{
}
//Given a rectangle, get the forbidden region which is 
//bounding box of rectangle plus spacing rule from each side

void PostProcessor::getForbiddenRegion(oaRect* rect, oaBox& forbiddenRegion,
        int spacingRule)
{
    rect->getBBox(forbiddenRegion);
    oaPoint lowerLeft, upperRight;
    //substract spacing from x and y
    lowerLeft = forbiddenRegion.lowerLeft() - oaPoint(spacingRule, spacingRule);
    //add spacing in x and y
    upperRight = forbiddenRegion.upperRight() + oaPoint(spacingRule, spacingRule);
    forbiddenRegion.set(lowerLeft, upperRight);
}
///*Given a vector of candidate polygons, find the polygons that are conflicting 
// * with the original rect
// */
//vector<oaShape*> PostProcessor::findConflictingPolygons(oaRect* originalRect,
//                                                        vector<oaShape*> candidates)
//{
//        //
//}

/*
 * Function merges all these shapes into one rect which is the bouding box of
 *  them all
 * Function assumes all of them are rectangles
 * WARNING: do not attempt to use origShape after this function 
 * because it will have been destroyed if the merge was successful
 */
bool PostProcessor::MergeShapes(oaShape* origShape, vector<oaShape*> shapes,
        oaDesign* design,
        oaBox& mergedBBox,
        int spacingRule,
        bool verticalLayer)
{
    //Now we're given a set of shapes that may be conflicting with an origShape
    //Now shapes that overlap with origShape need to be merged with it
    //Also shapes that are at dist less than spacing rule need to be merged with it
    //however while doing the merge, we need to be careful, not to touch any different
    //net
    OAHelper oaHelper;
    oaRect* origRect = oaHelper.convertToRectangle(origShape);
    //initialize bounding box to the bounding box of the origShape
    origRect->getBBox(mergedBBox);
    //get the net the orig shape belongs to
    oaNet* origNet = origShape->getNet();
    bool merge = true;
    vector<oaShape*> shapesToBeDestroyed; //save shapes that need to be destroyed
    //Now iterate on shapes
    int nShapes = shapes.size();
    if (nShapes == 1)//if only one shape, then it is the original and no need to 
        //do anything
        return false;

    for (int i = 0; i < nShapes; i++)
    {
        oaShape* currShape = shapes[i];
        if (currShape != origShape)//if not the original shape
        {

            OAHelper oaHelper;
            oaRect * currRect = oaHelper.convertToRectangle(currShape);
            if (currRect != NULL)//exclude other objects like text
            {
                //get bounding box
                oaBox currBBox;
                currRect->getBBox(currBBox);
                //needs merge if this currBBox overlaps with mergedBBox
                //or it is at dist less spacing from mergedBBox
                bool needsMerging = false;
                if (!currBBox.overlaps(mergedBBox))//if not overlapping, check dist

                {
                    int dist = oaHelper.calculateDist(mergedBBox, currBBox);
                    if (dist < spacingRule)
                        needsMerging = true;
                } else
                    needsMerging = true;
                if (needsMerging)
                {
                    if (currShape->getNet() != origNet)//if not same not, dont merge
                    {
                        merge = false;
                        break;
                    }
                    //merge bounding boxes
                    mergedBBox.merge(currBBox);
                    //mark polygons as to be destroyed
                    shapesToBeDestroyed.push_back(currShape);
                }
            }
        }
    }
    //check if there is need for merging
    int nShapesToBeDestroyed = shapesToBeDestroyed.size();
    if (nShapesToBeDestroyed == 0)
        merge = false;
    else
    {
        //Check that the merged bounding box doesn't violate the unidirectionality
        //i.e. for M1, it is still vertical and for metal2, still horizontal
        int width, height;
        width = mergedBBox.getWidth();
        height = mergedBBox.getHeight();
        if (verticalLayer)
        {
            if (height <= width)
                merge = false;
        } else
        {
            if (height >= width)
                merge = false;
        }
    }
    if (merge)
    {
        /*Now we have the merged bbox. However this merged bounding box may happen 
         * to overlap with other shapes that didn't fall in the forbidden region
         */
        oaLayerNum layNum = origShape->getLayerNum();
        oaPurposeNum purpNum = origShape->getPurposeNum();
        ShapesSaverQuery ssq;
        ssq.query(design, layNum, purpNum, mergedBBox);
        vector<oaShape*> shapesInMergedBBox = ssq.getShapes();
        bool willHitNewPolygons = !isSuperset(shapesInMergedBBox, shapes);
        if (!willHitNewPolygons)
        {
            origShape->destroy(); //destroy orig
            for (int i = 0; i < nShapesToBeDestroyed; i++)
            {
                shapesToBeDestroyed[i]->destroy(); //destroy shapes
            }
        } else merge = false;
    }

    return merge;
}

bool PostProcessor::isPowerRail(oaRect* rect)
{
    //Check that currRect is not power rail
    oaBox rectBBox;
    if (rect->getLayerNum() == METAL_LAYERS_INFO[0].layerNum)
    {
        //power rail will be horizontal although metal 1 is vertical

        rect->getBBox(rectBBox);

        if (rectBBox.getWidth() > rectBBox.getHeight())
            return true;

    }
    return false;
}
//Function checks if a set of shapes is a subset of another

bool PostProcessor::isSuperset(vector<oaShape*> candidateSubset,
        vector<oaShape*> candidateSuperset)
{
    int nShapesSubset = candidateSubset.size();
    for (int i = 0; i < nShapesSubset; i++)
    {
        vector<oaShape*>::iterator iter;
        iter = find(candidateSuperset.begin(), candidateSuperset.end(), candidateSubset[i]);
        if (iter == candidateSuperset.end())//if not found
            return false;
    }
    return true;
}

/*
 * After merging some polygons on a metal layer (e.g. M1), in some case there will be 
 * some polygons on another layer (e.g. M1) with vias for connections, where the 
 * other polygons (M2) were created just to connect the segments on the first 
 * layer (M1)
 */
void PostProcessor::removeUnnecessarySegments(oaDesign* design,
        oaBox mergedBBox, oaNet* net,
        int mergedPolygonsLayerIndex,
        ProjectDesignRules dr)
{
    //if it is metal 1, then look for shapes on metal2 and vice versa
    int layerIndex = !mergedPolygonsLayerIndex;
    oaLayerNum layNum = METAL_LAYERS_INFO[layerIndex].layerNum;
    oaLayerNum viaLayNum = VIA_LAYER_NUMBERS[0];
    oaPurposeNum purpNum = PURPOSE_NUM; //assuming one purpose number among layouts

    if(mergedPolygonsLayerIndex==1)//if M2
    {
        //get vias overlapping with merged shape
        ShapesSaverQuery ssqVias;
        ssqVias.query(design, viaLayNum, purpNum, mergedBBox);
        vector<oaShape*> vias = ssqVias.getShapes();
        int nVias=vias.size();
        
        for(int i=0;i<nVias;i++)
        {
            if(vias[i]!=NULL)
            {
                oaBox currBox;
                vias[i]->getBBox(currBox);
                for(int j=i+1;j<nVias;j++)
                {
                    oaBox tempBox;
                    vias[j]->getBBox(tempBox);
                    if(currBox.overlaps(tempBox,false))
                    {    
                        vias[j]->destroy();
                         vias[j]=NULL;
                    }
                }
            }
        }
        return;
    }
    //get shapes on other metal layer: expecting one shape only overlapping 
    //with this mergedBBox
    ShapesSaverQuery ssqOtherLayer;
    ssqOtherLayer.query(design, layNum, purpNum, mergedBBox);
    vector<oaShape*> shapesOtherMetalLayer = ssqOtherLayer.getShapes();
    int nShapesOtherMetalLayer = shapesOtherMetalLayer.size();
    //check that they all belong to same net
    bool allShapesRemoved=true;
    for (int i = 0; i < nShapesOtherMetalLayer; i++)
    {

        //if belongs to another net, then skip it
        if (shapesOtherMetalLayer[i]->getNet() != net)
        {
            shapesOtherMetalLayer[i] = NULL;
        } else
        {
            //Now there are shapes on the other metal layer
            //that overlaps with the merged bbox
            //Check that they extend only with up to via extension beyond the bounding box both side
            oaBox otherShapeBBox;
            shapesOtherMetalLayer[i]->getBBox(otherShapeBBox);
            if (METAL_LAYERS_INFO[layerIndex].vertical)
            {
               
                int bottom = mergedBBox.bottom() - dr.getContactViaExtensionRule();
                int top = mergedBBox.top() + dr.getContactViaExtensionRule();
                //if not the required coordinates
                if (otherShapeBBox.bottom() < bottom || otherShapeBBox.top() > top)
                {
                    //set it to NULL(later checked) in order not to be deleted
                    shapesOtherMetalLayer[i] = NULL;
                    allShapesRemoved=false;
                }
            } else//horizontal like M2
            {
               
                int left = mergedBBox.left() - dr.getContactViaExtensionRule();
                int right = mergedBBox.right() + dr.getContactViaExtensionRule();
                //if not the required coordinates, then eliminate it from 
                //vector so as not to be touched
                if (otherShapeBBox.left() < left || otherShapeBBox.right() > right)
                {
                    //set it to NULL(later checked) in order not to be deleted
                    shapesOtherMetalLayer[i] = NULL;
                    allShapesRemoved=false;
                }
            }
        }
    }
    //now check if there are vias
    ShapesSaverQuery ssqViasLayer1;
    ssqViasLayer1.query(design, viaLayNum, purpNum, mergedBBox);
    vector<oaShape*> viasLayer1 = ssqViasLayer1.getShapes();
    vector<oaShape*> viasNotDestroyed;
    int nVias = viasLayer1.size();
  
    if (nVias > 0)//if found vias then delete other layer polygons and vias
    {
        //check that all vias belong to same net(just a sanity check)
        for (int j = 0; j < nVias; j++)
            if (viasLayer1[j]->getNet() != net)
            {
           
                return;
            }
        int nShapesToDestroy = shapesOtherMetalLayer.size();
        
        for (int i = 0; i < nShapesToDestroy; i++)
        {
            
            if (shapesOtherMetalLayer[i] != NULL)
            {
                //get vias in the overlap area between the M1 and M2 shapes
                oaBox otherLayerShapeBBox;
                shapesOtherMetalLayer[i]->getBBox(otherLayerShapeBBox);
                ShapesSaverQuery ssqViasLayer2;
                ssqViasLayer2.query(design, viaLayNum, purpNum,
                        otherLayerShapeBBox);
                vector<oaShape*> viasLayer2 = ssqViasLayer2.getShapes();
                //Now check each via, if it lies in the overlap, then destroy it
                //i.e. found in bounding boxes of both shapes( M1 and M2 shapes)
                bool destroyedOneVia=false;
                for (int j = 0; j < nVias; j++)
                {
                    vector<oaShape*>::iterator iter;
                    iter = find(viasLayer2.begin(), viasLayer2.end(),
                            viasLayer1[j]);
                    if (iter != viasLayer2.end())
                    {
                         //destroy one via for each polygon removed                      
                        if(!destroyedOneVia)
                        { 
                            destroyedOneVia=true;   
                            viasLayer1[j]->destroy();
                            viasLayer1[j]=NULL;
                        }
                        else
                        {
                            //if via not already saved
                            if(find(viasNotDestroyed.begin(),
                                    viasNotDestroyed.end(), 
                                    viasLayer1[j])==viasNotDestroyed.end())
                                viasNotDestroyed.push_back(viasLayer1[j]);
                        }
                    } 
                }
                shapesOtherMetalLayer[i]->destroy();
                shapesOtherMetalLayer[i]=NULL;
            }
        }
        //Now if all M2 shapes have been destroyed, then 
        //all vias will be destroyed.
        //otherwise, only one via will be destroyed for each destroyed polygon
        //if(allShapesRemoved)
        {
            
            for(int i=0;i<viasNotDestroyed.size();i++)
            {
                if( viasNotDestroyed[i]!=NULL)
                {
                    //check if this via overlaps with any M2 polygons
                    //if it doesn't then delete it              
                    oaBox viaBox;
                    viasNotDestroyed[i]->getBBox(viaBox);
                    ShapesSaverQuery shapesM2Query;
                                      
                    shapesM2Query.query(design, layNum, purpNum,viaBox);
                    vector<oaShape*> overlappingM2Shapes=shapesM2Query.getShapes();
                    
                    if(overlappingM2Shapes.size()==0)
                    {     
                       
                        viasNotDestroyed[i]->destroy();
                         viasNotDestroyed[i]=NULL;
                    }
                    else//check that it doesn't overlap with another via
                    {
                        for(int j=i+1;j<viasNotDestroyed.size();j++)
                        {
                            oaBox tempBox;
                            viasNotDestroyed[j]->getBBox(tempBox);
                            //if overlap then erase it
                            if(viaBox.overlaps(tempBox,false))
                            {
                                viasNotDestroyed[j]->destroy();
                                viasNotDestroyed[j]=NULL;
                            }
                        }
                    }
                }
            }
        }
    }
}
int PostProcessor::dumpStats(oaDesign* design)
{
    oaBlock* topBlock = design->getTopBlock();
   
    vector<oaLayerNum> layersVec;
    layersVec.push_back(METAL_LAYERS_INFO[1].layerNum);//M1
    layersVec.push_back(VIA_LAYER_NUMBERS[0]);//via0
    int totalLength=0;
         int nVias=0;
    for(int i=0;i<layersVec.size();i++)
    {
    
         
        //find layer and purpose number
        //ASSUME one purpose number
        oaLayerHeader* layHead = oaLayerHeader::find(topBlock,
                layersVec[i]);
        if (layHead != NULL)//if layer found
        {
           
            oaIter<oaLPPHeader> headers(layHead->getLPPHeaders());
            while (oaLPPHeader * lppHeader = headers.getNext())
            {
                //oaLayerNum lppLayerNum = lppHeader->getLayerNum();
                // oaPurposeNum lppPurpNum = lppHeader->getPurposeNum();
                // if (lppLayerNum == layerNum)
                {
                    if (!lppHeader->getShapes().isEmpty())
                    {
                        oaIter<oaShape> shapes(lppHeader->getShapes());
                        while (oaShape * shape = shapes.getNext())
                        {
                            oaType type = shape->getType();
                            oaRect *rect = NULL;
                            if (type.getName() == "Polygon")
                            {
                                rect = ((oaPolygon*) shape)->convertToRect();
                            } else if (type.getName() == "Rect")
                            {
                                rect = ((oaRect*) shape);
                            }
                            if(rect!=NULL)
                            {
                                if(i==0)
                                {
                                oaBox box;
                                rect->getBBox(box);
                                totalLength+=box.getWidth();
                                }
                                else
                                    nVias++;
                            }
                        }
                    }
                }
            }
        }
    }
     cout<<"M2 WL: "<<totalLength<<endl;
     cout<<"N vias: "<<nVias<<endl;
}
