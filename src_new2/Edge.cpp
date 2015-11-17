/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include <float.h>

#include "Edge.h"
#include "oaDesignDB.h"
using namespace oa;
Edge::Edge(oaPoint point1, oaPoint point2)
{
    this->point1=point1;
    this->point2=point2;
}
Edge::~Edge()
{
}
/*
 * Function returns true if both edges are parallel and there is
 * an overlap between them
 *  */
bool Edge::overlapsWith( Edge& edge)
{
    if(isParallelTo(edge))
    {
        //if(isVertical()) no need for this check, because same code
        {
            //check the overlap in the y-coordinates:
            //check any of the endpoints of edge fall within shadow of
            //this edge
            if(includesPoint(edge.point1)||includesPoint(edge.point2))
            {
                return true;
            }
            //check the other way: the end point lies within t
            if(edge.includesPoint(point1)||edge.includesPoint(point2))
            {
                return true;
            }
            //Now the above comparisons were using strict comparison: i.e point 
            //has to lie within the ends, can not coincide with any of the 2 points
            //So this doesn't handle the case when both edges are exactly parallel
            //and start and end at same x/y if horizal/vertical
            //so this case can be handled if we do non-strict comparison
            //but require both endpoints to be included with the endpoints
            //of the other edge
            //check the other way: the end point lies within t
            if(edge.includesPoint(point1,false)&& edge.includesPoint(point2,false))
            {
                return true;
            }
        }
        
    }
    return false;
}

/*
 * Function returns true if a point lies within range of edge
 * It does NOT have to lie on line: i.e. if it is a vertical line,
 * it is enough for point to have a y-coordinate between ends of the
 * line 
 * strict=true==> point can not be at the ends
 * 
 */
bool Edge::includesPoint( oaPoint& point, bool strict)
{
    if(isVertical())
    {
        //check if point y-coordinate lies between the 2 y-coordinates
        //of the ends of this line
        if(strict)
        {
            //point between y1 and y2 (assuming y1 bigger than y2)
            if(point.y()< point1.y() && point.y()>point2.y())
            {
                return true;
            }
             //point between y1 and y2 (assuming y2 bigger than y1)
            if(point.y()< point2.y() && point.y()>point1.y())
            {
                return true;
            }
        }
        else
        {
            //point between y1 and y2 (assuming y1 bigger than y2)
            if(point.y()<= point1.y() && point.y()>=point2.y())
            {
                return true;
            }
             //point between y1 and y2 (assuming y2 bigger than y1)
            if(point.y()<= point2.y() && point.y()>=point1.y())
            {
                return true;
            }
        }
    }
    else//horizontal
    {
        //check if point y-coordinate lies between the 2 y-coordinates
        //of the ends of this line
        if(strict)
        {
            //point between y1 and y2 (assuming y1 bigger than y2)
            if(point.x()< point1.x() && point.x()>point2.x())
            {
                return true;
            }
             //point between y1 and y2 (assuming y2 bigger than y1)
            if(point.x()< point2.x() && point.x()>point1.x())
            {
                return true;
            }
        }
        else
        {
            //point between x1 and x2 (assuming x1 bigger than x2)
            if(point.x()<= point1.x() && point.x()>=point2.x())
            {
                return true;
            }
             //point between y1 and y2 (assuming y2 bigger than y1)
            if(point.x()<= point2.x() && point.x()>=point1.x())
            {
                return true;
            }
        }
    }
    return false;
}
bool Edge::isParallelTo( Edge& edge)
{
    if(!(isVertical()^ edge.isVertical()))
        return true;
    return false;
}
bool Edge::isVertical()
{
    if(point1.x()==point2.x())
        return true;
    return false;
}
/*
 * Calculates distance between 2 edges, assuming they are parallel
 */
int Edge::getDistance(Edge edge2)
{
    double dist=DBL_MAX;
    if(isParallelTo(edge2))//if parallel
    {
        if(isVertical())//if vertical return diff in x
        {
            return abs(point1.x()-edge2.point1.x());
        }
        else
        {
            return abs(point1.y()-edge2.point1.y());
        }
    }
    return static_cast<int>(dist);
}
    
    
