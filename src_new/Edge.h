/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef EDGE_H
#define	EDGE_H
#include "oaDesignDB.h"
using namespace oa;
class Edge {
public:
    Edge(){}
    Edge(oaPoint point1, oaPoint point2);
  
    virtual ~Edge();
    bool overlapsWith( Edge& edge);
    bool isParallelTo( Edge& edge);
    bool isVertical();
    bool includesPoint( oaPoint& point, bool strict=true);
    int getDistance(Edge edge2);
private:
    oaPoint point1;
    oaPoint point2;

};

#endif	/* EDGE_H */

