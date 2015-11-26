/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */


#ifndef POSTPROCESSOR_H
#define	POSTPROCESSOR_H
#include "Grid.h"
#include "oaDesignDB.h"
#include "ProjectDesignRules.h"
using namespace oa;
class PostProcessor {
public:
    PostProcessor();
    virtual ~PostProcessor();
    void Postprocess(Grid* grid, oaDesign* design, ProjectDesignRules dr);
    void getForbiddenRegion(oaRect* rect, oaBox& forbiddenRegion,
                            int spacingRule);
    /*vector<oaShape*> findConflictingPolygons(oaRect* originalRect,
                                             vector<oaShape*> candidates);
      */
    bool MergeShapes(oaShape* origShape,
                     vector<oaShape*> shapes, 
                     oaDesign* design,
                     oaBox& mergedBox,
                     int spacingRule,
                     bool verticalLayer);
    bool isPowerRail(oaRect* rect);
    int dumpStats(oaDesign* design);
   
private:
    bool isSuperset(vector<oaShape*> candidateSubset, 
                    vector<oaShape*> candidateSuperset);
     void removeUnnecessarySegments(oaDesign* design, oaBox mergedBBox,
                                   oaNet* net,
                                   int mergedPolygonsLayerIndex,
                                   ProjectDesignRules dr);
    
};

#endif	/* POSTPROCESSOR_H */

