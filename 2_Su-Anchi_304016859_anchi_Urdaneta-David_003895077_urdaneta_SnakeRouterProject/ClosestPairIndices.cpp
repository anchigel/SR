/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include <float.h>

#include "ClosestPairIndices.h"
#include "Cell.h"

ClosestPairIndices::ClosestPairIndices(int index1,int index2)
{
    this->_index1=index1;
    this->_index2=index2;
}


ClosestPairIndices::~ClosestPairIndices()
{
}
//given the contacts of a net, find the closest pairs
vector<ClosestPairIndices> ClosestPairIndices::getClosestPairs(vector<Cell*> netContacts)
{
    vector<ClosestPairIndices> closestPairs;
  
    vector<Cell*> remainingContacts=netContacts;
    if(netContacts.size()!=0)
    {
    Cell* currCell=netContacts[0];
    remainingContacts[0]=NULL;
    int currIndex=0;
    while(currCell!=NULL)       
    //for(int i=0;i<nContacts;i++)
    {
        
        int x,y;
        //get contact center
        currCell->getAbsolutePosition(&x,&y);
        oaPoint currContactCenter(x,y);
        double minDist=DBL_MAX;
        int closestIndex=0;
        int nRemainingContacts=remainingContacts.size();
        for(int j=0;j<nRemainingContacts;j++)
        {
            if(remainingContacts[j]!=NULL)
            {
                remainingContacts[j]->getAbsolutePosition(&x,&y);
                oaPoint tempContactCenter(x,y);
                double currDist=abs(currContactCenter.x()-tempContactCenter.x())
                            +abs(currContactCenter.y()-tempContactCenter.y());
                if(currDist<=minDist)
                {
                    minDist=currDist;
                    closestIndex=j;
                }
            }
        }
         //Now create a closest pair btween i and closestIndex
        closestPairs.push_back(ClosestPairIndices(currIndex,closestIndex));
       
        currCell=remainingContacts[closestIndex];
        remainingContacts[closestIndex]=NULL;
        currIndex=closestIndex;
       
    }
    }
    
    return closestPairs;
}
bool ClosestPairIndices::operator ==(ClosestPairIndices otherPair)
{
    if(_index1==otherPair.getIndex1() && _index2==otherPair.getIndex2())
    {
        return true;
    }
    if(_index2==otherPair.getIndex1() && _index1==otherPair.getIndex2())
    {
        return true;
    }
    return false;
}

bool ClosestPairIndices::operator !=(ClosestPairIndices otherPair)
{
    return !operator ==(otherPair);
}
