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
    __metal1_direction = 'V';
    __metal2_direction = 'B';
    __metal3_direction = 'H';
}

ProjectDesignRules::ProjectDesignRules(const string designRuleFilename) {
    //Read design rules from input file
    int dbu_factor = 10;
    //int dbu_factor = 1; //Weiche
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
    infile >> __viaDimensionRule;
    __viaDimensionRule *= dbu_factor;
    infile >> __metal1_direction;
    infile >> __metal2_direction;
    infile >> __metal3_direction;
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

char ProjectDesignRules::getMetal1Direction() {
    return __metal1_direction;
}

char ProjectDesignRules::getMetal2Direction() {
    return __metal2_direction;
}

char ProjectDesignRules::getMetal3Direction() {
    return __metal3_direction;
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

bool ProjectDesignRules::setMetal1Direction(char dir) {
    switch (dir) {
        case 'V':
            __metal1_direction = 'V';
            return true;
        case 'H':
            __metal1_direction = 'H';
            return true;
        case 'B':
            __metal1_direction = 'B';
            return true;
        default:
            return false;
    }
}

bool ProjectDesignRules::setMetal2Direction(char dir) {
    switch (dir) {
        case 'V':
            __metal2_direction = 'V';
            return true;
        case 'H':
            __metal2_direction = 'H';
            return true;
        case 'B':
            __metal2_direction = 'B';
            return true;
        default:
            return false;
    }
}

bool ProjectDesignRules::setMetal3Direction(char dir) {
    switch (dir) {
        case 'V':
            __metal3_direction = 'V';
            return true;
        case 'H':
            __metal3_direction = 'H';
            return true;
        case 'B':
            __metal3_direction = 'B';
            return true;
        default:
            return false;
    }
}

void ProjectDesignRules::print() {   
    cout << "Design rules in DBU (assumed 10 DBU = 1 nm):" << endl
         << "... minimum metal width: " << __metalWidthRule << endl
         << "... minimum metal spacing: " << __metalSpaceRule << endl
         << "... minimum contact/via extension: " << __contactViaExtensionRule << endl
         << "... minimum metal area: " << __minMetalAreaRule << endl
         << "... via dimensions: " << __viaDimensionRule << " * " << __viaDimensionRule << endl
         << "... metal1 direction: " << __metal1_direction << endl
         << "... metal2 direction: " << __metal2_direction << endl
         << "... metal3 direction: " << __metal3_direction << endl;
}
