/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef CLOSESTPAIRINDICES_H
#define	CLOSESTPAIRINDICES_H
#include <vector>
#include "Cell.h"
using namespace std;
class ClosestPairIndices {
public:
    ClosestPairIndices(int index1, int index2);
  
    virtual ~ClosestPairIndices();

    int getIndex1() const {
        return _index1;
    }

    void setIndex1(int _index1) {
        this->_index1 = _index1;
    }

    int getIndex2() const {
        return _index2;
    }

    void setIndex2(int _index2) {
        this->_index2 = _index2;
    }
    static vector<ClosestPairIndices> getClosestPairs(vector<Cell*> netContacts);
    bool operator==(ClosestPairIndices otherPair);
     bool operator!=(ClosestPairIndices otherPair);
private:
    int _index1;
    int _index2;
};

#endif	/* CLOSESTPAIRINDICES_H */

