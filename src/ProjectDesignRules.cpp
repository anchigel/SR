/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */


#include "ProjectDesignRules.h"
#include <iostream>

using namespace oa;
using namespace std;

//PUBLIC METHODS
ProjectDesignRules::ProjectDesignRules() {
    __metalWidthRule = 0;
    __metalSpaceRule = 0;
    __contactViaExtensionRule = 0;
    __minMetalAreaRule = 0;
    __viaDimensionRule = 0;
    __metal1Direction = 'H';
    __metal2Direction = 'V';
    __metal3Direction = 'B';
}

/*ProjectDesignRules::ProjectDesignRules(const ProjectDesignRules& orig) {
    __metalWidthRule = orig.__metalWidthRule;
    __metalSpaceRule = orig.__metalSpaceRule;
    __contactViaExtensionRule = orig.__contactViaExtensionRule;
    __minMetalAreaRule = orig.__minMetalAreaRule;
    __viaDimensionRule = orig.__viaDimensionRule;
}*/

ProjectDesignRules::ProjectDesignRules(const string designRuleFilename) {
    //Read design rules from input file
    //int dbu_factor = 10;
    int dbu_factor = 1;
    ifstream infile;
    infile.open(designRuleFilename.c_str());

    if (!infile) {
        cout << "There was a problem opening design rule file "
             << designRuleFilename
             << " for reading."
             << endl;
        exit(1);
    }
    
    infile >> __metalWidthRule;
    __metalWidthRule *= dbu_factor; //convert to DBU
    infile >> __metalSpaceRule;
    __metalSpaceRule *= dbu_factor;
    infile >> __contactViaExtensionRule;
    __contactViaExtensionRule *= dbu_factor;
    infile >> __minMetalAreaRule;
    __minMetalAreaRule *= dbu_factor;
    infile >> __viaDimensionRule;
    __viaDimensionRule *= dbu_factor;
    if(!infile.eof())
        infile >> __metal1Direction;
    if(!infile.eof())
        infile >> __metal2Direction;
    if(!infile.eof())
        infile >> __metal3Direction;
}

ProjectDesignRules::~ProjectDesignRules() {}

oaUInt4 ProjectDesignRules::getMetalWidthRule() {
    return __metalWidthRule;
}
oaUInt4 ProjectDesignRules::getMetalSpaceRule() {
    return __metalSpaceRule;
}
oaUInt4 ProjectDesignRules::getContactViaExtensionRule() {
    return __contactViaExtensionRule;
}
oaUInt4 ProjectDesignRules::getMinMetalAreaRule() {
    return __minMetalAreaRule;
}
oaUInt4 ProjectDesignRules::getViaDimensionRule() {
    return __viaDimensionRule;
}

oaInt1 ProjectDesignRules::getMetal1Direction() {
    return __metal1Direction;
}
oaInt1 ProjectDesignRules::getMetal2Direction() {
    return __metal2Direction;
}
oaInt1 ProjectDesignRules::getMetal3Direction() {
    return __metal3Direction;
}

void ProjectDesignRules::setMetalWidthRule(oaUInt4 rule) {
	__metalWidthRule = rule;
}

void ProjectDesignRules::setMetalSpaceRule(oaUInt4 rule) {
	__metalSpaceRule = rule;
}

void ProjectDesignRules::setContactViaExtensionRule(oaUInt4 rule) {
	__contactViaExtensionRule = rule;
}

void ProjectDesignRules::setViaDimensionRule(oaUInt4 rule) {
	__viaDimensionRule = rule;
}

void ProjectDesignRules::setMinMetalAreaRule(oaUInt4 rule) {
	__minMetalAreaRule = rule;
}

void ProjectDesignRules::print() {   
    cout << "Design rules in DBU (assumed 10 DBU = 1 nm):" << endl
         << "... minimum metal width: " << __metalWidthRule << endl
         << "... minimum metal spacing: " << __metalSpaceRule << endl
         << "... minimum contact/via extension: " << __contactViaExtensionRule << endl
         << "... minimum metal area (optional, bonus pts): " << __minMetalAreaRule << endl
         << "... via dimensions: " << __viaDimensionRule << " * " << __viaDimensionRule << endl;
}

//PRIVATE METHODS
