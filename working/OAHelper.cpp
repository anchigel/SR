/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include "OAHelper.h"
#include <iostream>
#include "Definitions.h"
#include "Edge.h"
using namespace std;
OAHelper::OAHelper()
{
}
OAHelper::~OAHelper()
{
}
void OAHelper::getPowerRailYCoordinates(oaDesign* design,
                                        int& vddLowerY, 
                                        int& gndUpperY)
{
     oaBlock* topBlock=design->getTopBlock();
     oaLayerHeader * m1LayerHeader= oaLayerHeader::find(topBlock,
                                                  METAL_LAYERS_INFO[0].layerNum);//M1
    //  find existing shapes on M1 layer:expecting 2 shapes only
    //but will work with more shapes as well
    oaIter<oaLPPHeader> LPPHeaderIter(m1LayerHeader->getLPPHeaders());
    oaLPPHeader * LPPHeader;
   
    while (LPPHeader = LPPHeaderIter.getNext())
    {
      oaShape * shape;
      oaIter<oaShape> shapeIter (LPPHeader->getShapes());
  
      while (shape = shapeIter.getNext())
      {
        oaPolygon * polygon;
        if (shape->getType()==oacPolygonType||shape->getType()==oacRectType)
        {
            if(shape->getType()==oacPolygonType)
            {
                polygon = (oaPolygon*)shape;
            }
            else if(shape->getType()==oacRectType)
            {
                polygon=((oaRect*)shape)->convertToPolygon();
            }    
            oaBox bbox;
            polygon->getBBox(bbox);
            if(bbox.bottom()<0)//gnd
            {
                gndUpperY=bbox.top();
            }
            else//vdd
            {
                vddLowerY=bbox.bottom();
            }
        }
      }
    }
}
 void OAHelper::getCellBBox(oaDesign* design, oaBox &bbox)
 {
     //since getting the design bounding box is getting a much bigger
     //bbox than the actual box, I will use the power rail coordinates on metal1
     //to get the bounding box
     oaBlock* topBlock=design->getTopBlock();
     oaLayerHeader * m1LayerHeader= oaLayerHeader::find(topBlock,
                                                  METAL_LAYERS_INFO[0].layerNum);//M1
 
    //  find existing shapes on M1 layer:expecting 2 shapes only
    //but will work with more shapes as well
    oaIter<oaLPPHeader> LPPHeaderIter(m1LayerHeader->getLPPHeaders());
    oaLPPHeader * LPPHeader;
    bool firstShape=true;
    while (LPPHeader = LPPHeaderIter.getNext())
    {
      oaShape * shape;
      oaIter<oaShape> shapeIter (LPPHeader->getShapes());
  
      while (shape = shapeIter.getNext())
      {
        oaPolygon * polygon;
        if (shape->getType()==oacPolygonType||shape->getType()==oacRectType)
        {
            if(shape->getType()==oacPolygonType)
            {
                polygon = (oaPolygon*)shape;
            }
            else if(shape->getType()==oacRectType)
            {
                polygon=((oaRect*)shape)->convertToPolygon();
            }                
            
            //if first shape then initialize design bbox to the bbox of this shape
            if(firstShape)
            {
                polygon->getBBox(bbox);
                firstShape=false;
            }
            else//otherwise merge this shape bounding box with others
            {
                oaBox tempBox;
                polygon->getBBox(tempBox);
                bbox.merge(tempBox);
            }         
        }
      }
    }
 }
 //Given a shape which is a 4-sided polygon or rectange,
 //function returns it as a rectangle
 oaRect* OAHelper::convertToRectangle(oaShape* shape)
 {
        oaRect *rect=NULL;
        oaType type = shape->getType();        
   
        if (type.getName() == "Polygon")
        {
            rect = ((oaPolygon*)shape)->convertToRect();                           
        } 
        else if (type.getName() == "Rect")
        {
            rect = ((oaRect*)shape);
        }
        return rect;
 }
 
 int OAHelper::calculateDist(oaRect* rect1, oaRect* rect2)
 {
     oaBox box1,box2;
     rect1->getBBox(box1);
     rect2->getBBox(box2);
     return calculateDist(box1,box2);
     
     
 }
 int OAHelper::calculateDist(oaBox box1, oaBox box2)
 {
     
     int dist=INT_MAX;
     int dist2=INT_MAX;
     //calculate dist between overlapping and parallel edges
     vector<Edge> edges1, edges2;
     GetEdges(box1, edges1);
     GetEdges(box2, edges2);
     int nEdges1=edges1.size();
     int nEdges2=edges2.size();
     //get distance between edges (if overlapping))
     for(int e1=0;e1<nEdges1;e1++)
         for(int e2=0;e2<nEdges2;e2++)
         {
             if(edges1[e1].overlapsWith(edges2[e2]))
                 dist=min(dist, edges1[e1].getDistance(edges2[e2]));
         }
     if(dist==INT_MAX)//if no edges overlap
     {     
        
        oaPointArray points2;
        //get points of rect2
        GetPoints(box2,points2);

        int numPoints = points2.getNumElements();
        //calculate min dist between rectangle1 and points of rectangle2
        for (int i = 0; i < numPoints; i++)
        {
            dist2 = min(dist2, (int)box1.distanceFrom2(points2[i]));
        }
        dist = static_cast<int>(sqrt(dist2));
     }
     return dist;
 }
void OAHelper::GetEdges(oaRect* rect, vector<Edge>& edges)
{
    oaPolygon* polygon=rect->convertToPolygon();
    oaPointArray points;    
    polygon->getPoints(points);
    int nPoints=points.getNumElements();
    for(int i=0;i<nPoints;i++)
    {
        if(i!=nPoints-1)
        {
                Edge currEdge(points[i],points[i+1]);
                edges.push_back(currEdge);
        }
        else//last edge between last point and first point
        {
                Edge currEdge(points[i],points[0]);
                edges.push_back(currEdge);
        }
    }
}

void OAHelper::GetEdges(oaBox box, vector<Edge>& edges)
{
    oaPointArray points;
    GetPoints(box,points);
    int nPoints=points.getNumElements();
    for(int i=0;i<nPoints;i++)
    {
        if(i!=nPoints-1)
        {
                Edge currEdge(points[i],points[i+1]);
                edges.push_back(currEdge);
        }
        else//last edge between last point and first point
        {
                Edge currEdge(points[i],points[0]);
                edges.push_back(currEdge);
        }
    }
}
//get coordinates of box
void OAHelper::GetPoints(oaBox box, oaPointArray& points)
{
    int left, bottom, right, top;
    left=box.left();
    bottom=box.bottom();
    right=box.right();
    top=box.top();
          
    //create points
    oaPoint p1(left, bottom);
    oaPoint p2(right, bottom);
    oaPoint p3(right, top);
    oaPoint p4(left, top);
    points.append(p1);
    points.append(p2);
    points.append(p3);
    points.append(p4);
}
