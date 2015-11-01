/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include "ShapesSaverQuery.h"

ShapesSaverQuery::ShapesSaverQuery()
{
}

ShapesSaverQuery::ShapesSaverQuery(const ShapesSaverQuery& orig) {
}

ShapesSaverQuery::~ShapesSaverQuery() {
}
void  ShapesSaverQuery::queryShape(oaShape *shape)
{
    foundShapes.push_back(shape);
}
vector<oaShape*> ShapesSaverQuery::getShapes()
{       
    return foundShapes;
}
