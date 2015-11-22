/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */



#ifndef PROJECTDESIGNRULES_H
#define	PROJECTDESIGNRULES_H

//#include <vector>
#include <string>
#include "oaDesignDB.h"
using namespace oa;
using namespace std;


class ProjectDesignRules
{
public:
    ProjectDesignRules();
    //ProjectDesignRules(const ProjectDesignRules& orig);
    ProjectDesignRules(const string designRuleFilename);
    virtual ~ProjectDesignRules();
    
    oaUInt4 getMetalWidthRule();
    oaUInt4 getMetalSpaceRule();
    oaUInt4 getContactViaExtensionRule();
    oaUInt4 getViaDimensionRule();
    oaUInt4 getMinMetalAreaRule();
    char getMetal1Direction();
    char getMetal2Direction();
    char getMetal3Direction();

    void setMetalWidthRule(oaUInt4 rule);
    void setMetalSpaceRule(oaUInt4 rule);
    void setContactViaExtensionRule(oaUInt4 rule);
    void setViaDimensionRule(oaUInt4 rule);
    void setMinMetalAreaRule(oaUInt4 rule);
    bool setMetal1Direction(char dir);
    bool setMetal2Direction(char dir);
    bool setMetal3Direction(char dir);

    void print();
    
private:
    oaUInt4 __metalWidthRule;
    oaUInt4 __metalSpaceRule;
    oaUInt4 __contactViaExtensionRule;
    oaUInt4 __viaDimensionRule;
    oaUInt4 __minMetalAreaRule;
    char __metal1_direction; //V, H, or B
    char __metal2_direction; //V, H, or B
    char __metal3_direction; //V, H, or B
};

#endif	/* PROJECTDESIGNRULES_H */

