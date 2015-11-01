/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef INPUTOUTPUTMANIPULATOR_H
#define	INPUTOUTPUTMANIPULATOR_H

#include <vector>
#include <string>
#include "oaDesignDB.h"
#include "Connection.h"
using namespace std;
using namespace oa;

struct DesignInfo
{
    string designPath;
    string designName;
    string designCellName;
    string routedCellName;
    string designView; 
    string netlistFileName;
    string DRFileName;
};

class InputOutputManipulator
{
public:
    InputOutputManipulator();
    InputOutputManipulator(const InputOutputManipulator& orig);
    virtual ~InputOutputManipulator();
    static void ReadInputArguments(string fileName, vector<DesignInfo>& designsInfo);
    static void ReadInputArguments(char* argv[], DesignInfo& designInfo);
    static oaDesign* ReadOADesign(DesignInfo designInfo);
    static void SaveRoutedCell(DesignInfo designInfo, oaDesign* design);
    static void GetCellNames(DesignInfo designInfo,
                                 vector<string> & cellNames);
    vector<Connection> ParseNetlist(oaDesign* design, string netlistFileName);

    
private:

};

#endif	/* INPUTOUTPUTMANIPULATOR_H */

