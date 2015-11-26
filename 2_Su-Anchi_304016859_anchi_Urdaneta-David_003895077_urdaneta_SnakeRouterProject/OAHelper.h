/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef OAHELPER_H
#define	OAHELPER_H
#include "oaDesignDB.h"
#include <vector>
#include "Edge.h"
using namespace std;
using namespace oa;
class OAHelper 
{
public:
    OAHelper();
    virtual ~OAHelper();
    static void getCellBBox(oaDesign* design, oaBox &bbox);
    void getPowerRailYCoordinates(oaDesign* design,
                                int & vddLowerY, int &gndUpperY);
    oaRect* convertToRectangle(oaShape* shape);
    int calculateDist(oaRect* rect1, oaRect* rect2);
    int calculateDist(oaBox box1, oaBox box2);
    void GetEdges(oaRect* rect, vector<Edge>& edges);
    void GetEdges(oaBox box, vector<Edge> & edges);
    void GetPoints(oaBox box, oaPointArray& points);
private:
    
};

#endif	/* OAHELPER_H */

