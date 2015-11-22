/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef SHAPESSAVERQUERY_H
#define	SHAPESSAVERQUERY_H
#include <vector>
#include "oaDesignDB.h"
using namespace oa;
using namespace std;
class ShapesSaverQuery:public oaShapeQuery
{
public:
    ShapesSaverQuery();
    ShapesSaverQuery(const ShapesSaverQuery& orig);
    virtual ~ShapesSaverQuery();
    void queryShape(oaShape *shape);
    vector<oaShape*> getShapes();
    
private:
    vector<oaShape*> foundShapes;
};

#endif	/* SHAPESSAVERQUERY_H */

