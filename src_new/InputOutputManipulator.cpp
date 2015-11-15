/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include "InputOutputManipulator.h"
#include "Connection.h"
#include "Definitions.h"
#include <iostream>
using namespace std;
InputOutputManipulator::InputOutputManipulator()
{
}

InputOutputManipulator::~InputOutputManipulator()
{
}
/*
 * Function reads info about OA designs from given file name
 * It can read multiple designs info. Each design should have a keyword "design"
 * before it.
 * E.g.:
 * design
./DesignLib
DesignLib
INV_X1
layout
design
./DesignLib2
DesignLib2
AND2_X1
layout
 */
void InputOutputManipulator::ReadInputArguments(string fileName,
                                        vector<DesignInfo> & designsInfo)
{
    //reads designs info from a file: path, libName,cell, view
    fstream f;
    f.open(fileName.c_str());
    while(!f.eof())
    {
        string initiator;
        f>>initiator;
        if(initiator=="design")//keyword "design" expected before each new design
        {
                DesignInfo design;
                f>>design.designPath;
                f>>design.designName;
                f>>design.designCellName;
                f>>design.designView;
                designsInfo.push_back(design);
        }
        else break;
    }
    f.close();
}
//Function sets design info as instructed by TA
//OA lib name is always DesignLib
//Input: cellName cellName_afterRouting netlistFileName DRFileName
void InputOutputManipulator::ReadInputArguments(char* argv[],
                                                DesignInfo& designInfo)
{
    designInfo.designPath="./DesignLib";
    designInfo.designName="DesignLib";
    designInfo.designCellName=string(argv[1]);
    designInfo.routedCellName=string(argv[2]);
    designInfo.designView="layout";
    designInfo.netlistFileName=string(argv[3]);
    designInfo.DRFileName=string(argv[4]);
}
/*
 * Given design info, function opens library and returns pointer to oaDesign
 */
oaDesign* InputOutputManipulator::ReadOADesign(DesignInfo designInfo)
{
    oaNativeNS	ns;
    oaScalarName	libName(ns, designInfo.designName.c_str());
    oaScalarName	cellName(ns, designInfo.designCellName.c_str());
    oaScalarName	viewName(ns, designInfo.designView.c_str());
    oaString	libraryPath(designInfo.designPath.c_str());
    
    // open the libs defined in "lib.def"
  oaLibDefList::openLibs();

  // locate the library
  oaLib *lib = oaLib::find(libName);
  
  if (!lib)
  {
    if (oaLib::exists(libraryPath))
    {
      lib = oaLib::open(libName, libraryPath);
    }
    else
    {
      lib = oaLib::create(libName, libraryPath);
    }
    if (lib)
    {
      //update the lib def list
      oaLibDefList *list = oaLibDefList::getTopList();
      if (list) 
      {
        oaString topListPath;
        list->getPath(topListPath);
        list->get(topListPath,'a');
        oaLibDef *newLibDef = oaLibDef::create(list, libName, libraryPath);
        list->save();
      }
    }
    else
    {
      cerr << "Error : Unable to create " << libraryPath << "/";
      //cerr << library << endl;
      return NULL;
    }
  }
    oaDesign *view = oaDesign::open(libName, cellName, viewName,'r');//change r to a to write

    view->openHier();//to flatten
    
    // open the tech for new layer creating
    oaTech * tech = oaTech::open(lib, 'a');
    //Make sure via and M2 layers exist (M1 assumed to exist)
    oaLayer * layer;

    // check if via1 layer is in the database
    layer =  oaLayer::find(tech, "via1");
    if (layer==NULL) 
    {
      cout << "Creating via1 layer\n";
      oaPhysicalLayer::create(tech, "via1", VIA_LAYER_NUMBERS[0], oacMetalMaterial, 11);
    }
	// check if via2 layer is in the database
    layer =  oaLayer::find(tech, "via2");
    if (layer==NULL) 
    {
      cout << "Creating via1 layer\n";
      oaPhysicalLayer::create(tech, "via2", VIA_LAYER_NUMBERS[1], oacMetalMaterial, 13);
    }
    // check if metal2 layer is in the database
    layer =  oaLayer::find(tech, "metal2");
    if (layer==NULL) 
    {
      cout << "Creating metal2 layer\n";
      oaPhysicalLayer::create(tech, "metal2", METAL_LAYERS_INFO[1].layerNum, 
                              oacMetalMaterial, METAL_LAYERS_INFO[1].layerNum);
    }
	// check if metal3 layer is in the database
    layer =  oaLayer::find(tech, "metal3");
    if (layer==NULL) 
    {
      cout << "Creating metal3 layer\n";
      oaPhysicalLayer::create(tech, "metal3", METAL_LAYERS_INFO[2].layerNum, 
                              oacMetalMaterial, METAL_LAYERS_INFO[2].layerNum);
    }
    //oaRect::create(view->getTopBlock(), 11,1,oaBox(0,0,200,200));
    //save tech
    tech->save();
    return view;
}
/*
 * When this function is called after opening design, it saves a copy
 * where the copy is used later for routing 
 */
void InputOutputManipulator::SaveRoutedCell(DesignInfo designInfo,
                                                    oaDesign* design)
{
    oaNativeNS	ns;
    oaScalarName	libName(ns, designInfo.designName.c_str());
    oaScalarName	routedCellName(ns, designInfo.routedCellName.c_str());
    oaScalarName	viewName(ns, designInfo.designView.c_str());
    oaString	libraryPath(designInfo.designPath.c_str());
    design->saveAs(libName,
                   routedCellName,
                   viewName);
}
/*
 * Given the design info, the function gets the list of cells in this design
 * starting at the given top cell name
 */

void InputOutputManipulator::GetCellNames(DesignInfo designInfo,
                                           vector<string> & cellNames)
{
    oaNativeNS	ns;
    oaScalarName	libName(ns, designInfo.designName.c_str());
    oaScalarName	cellName(ns, designInfo.designCellName.c_str());
    oaScalarName	viewName(ns, designInfo.designView.c_str());
    oaString	libraryPath(designInfo.designPath.c_str());
    if (oaLib::exists(libraryPath)) 
    {
        if (!oaLib::find(libName))
        {
                oaLib::open(libName, libraryPath);
        }
    } 
    else 
    {
        cout << "Library " << libraryPath << "is not present." << endl;
    }
    oaDesign *view = oaDesign::open(libName, cellName, viewName,'r');//change r to a to write
    oaIter<oaInstHeader> headers(view->getTopBlock()->getInstHeaders());

    //read cell names in this cell
    while (oaInstHeader *header = headers.getNext())
    {    
        oaScalarName cellName;
        header->getCellName(cellName);
        oaString cellNameAsStr;
        cellName.get(cellNameAsStr);
        string tempCellName((const char*)cellNameAsStr);
        cellNames.push_back(tempCellName);
    }
    view->close();
}
/*
 * Function reads the netlist file and creates a vector of connections accordingly
 */
vector<Connection> InputOutputManipulator::ParseNetlist(oaDesign* design, string netlistFileName)
{
    int i = 0;
    ifstream f(netlistFileName.c_str());
    vector<Connection> connections;
    if(f.is_open())//if opened successfully
    {
        vector<oaPoint> contactCoordinates_lowerLeft;
        vector<oaPoint> contactCoordinates_center; //ATTN: THIS IS A HACK UNTIL WE ARE SURE OF LIANGZHEN'S INPUT FOR CONTACTS
        while(!f.eof())
        {
            string word;
            string netType;
            f>>word;
            //use "/" as delimiter (because for IO: it will be represented
            //as IO/pinName)
            //Other netnames will not have the pin name 
            int indexPinName=word.find_first_of('/');
            int length;
            if(indexPinName==-1)//slash not found==>not IO
                length=word.size();
            else length=indexPinName;
            netType=word.substr(0,length);
           
            if(netType=="VDD"|| netType=="VSS"||netType=="IO"||netType=="S")
            {
                Connection conn;
                conn.contactCoords_lowerLeft=contactCoordinates_lowerLeft;
                conn.contactCoords=contactCoordinates_center;
                conn.netType=netType;
                conn.netID = i++;
                conn.pinName="";//init to empty string
                /*if netType is IO, then set pinName
                 */
                if(netType=="IO")
                {
                    string pinName=word.substr(length+1,word.size());
                    conn.pinName=pinName;
                }
                
                connections.push_back(conn);
                //clear contactCoords to hold the next connection
                contactCoordinates_lowerLeft.clear();
                contactCoordinates_center.clear();
            }
            else//if not a net type, then it's an X coordinate
            {
                int x, y;
                x=atoi(word.c_str());
                //read the y coord
                f>>word;
                y=atoi(word.c_str());
                contactCoordinates_lowerLeft.push_back(oaPoint(x,y));
                contactCoordinates_center.push_back(oaPoint(x+325,y+325));
            }
        }
        f.close();
    }
    else cerr<<"Netlist file not found!"<<endl;
    return connections;
    
}

