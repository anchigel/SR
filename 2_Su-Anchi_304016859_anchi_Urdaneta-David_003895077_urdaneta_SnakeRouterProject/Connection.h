/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef CONNECTION_H
#define	CONNECTION_H
#include <vector>
#include "oaDesignDB.h"
using namespace std;
using namespace oa;
class Connection
{
public:
    //Connection();
    //~Connection();
    vector<oaPoint> contactCoords_lowerLeft;
    vector<oaPoint> contactCoords;
    string netType;
    string pinName;//only used if it's IO
    oaInt4 netID;
};

#endif	/* CONNECTION_H */

