/* Authors: Mark Gottscho
 * Email: mgottscho@ucla.edu
 * Copyright (C) 2015 Mark Gottscho
 */

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include "ProjectDesignRules.h"
#include "OAHelper.h"
#include "Definitions.h"
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

void ReadInputArguments(char* argv[],
                                                DesignInfo& designInfo)
{
    designInfo.designPath="./DesignLib";
    designInfo.designName="DesignLib";
    designInfo.designCellName=string(argv[1]);
    designInfo.routedCellName=string(argv[2]);
    designInfo.designView="layout";
    designInfo.netlistFileName=string(argv[3]);
    designInfo.DRFileName=string(argv[4]);
}/*
 * Given design info, function opens library and returns pointer to oaDesign
 */
oaDesign* ReadOADesign(DesignInfo designInfo)
{
    oaNativeNS	ns;
    oaScalarName	libName(ns, designInfo.designName.c_str());
    oaScalarName	cellName(ns, designInfo.routedCellName.c_str());
    oaScalarName	viewName(ns, designInfo.designView.c_str());
    oaString	libraryPath(designInfo.designPath.c_str());
    
    // open the libs defined in "lib.def"
  oaLibDefList::openLibs();

  // locate the library
  oaLib *lib = oaLib::find(libName);
  
  if (!lib)
  {
    //cout << "OA library " << designInfo.designName.c_str() << " doesn't exist" << endl;
    //return NULL;
    if (oaLib::exists(libraryPath))
    {
      lib = oaLib::open(libName, libraryPath);
    }
    else
    {
      //lib = oaLib::create(libName, libraryPath);
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
   /* 
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
    // check if metal2 layer is in the database
    layer =  oaLayer::find(tech, "metal2");
    if (layer==NULL) 
    {
      cout << "Creating metal2 layer\n";
      oaPhysicalLayer::create(tech, "metal2", METAL_LAYERS_INFO[1].layerNum, 
                              oacMetalMaterial, METAL_LAYERS_INFO[1].layerNum);
    }
    //oaRect::create(view->getTopBlock(), 11,1,oaBox(0,0,200,200));
    //save tech
    tech->save();*/
    return view;
}

/*
 * Function reads the netlist file and creates a vector of connections accordingly
 */
vector<Connection> ParseNetlist(oaDesign* design, string netlistFileName)
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

int main(int argc, char** argv)
{
    //Hello World
    cerr << "=================================================" << endl;
    cerr << "This is Osprey, hunter of SnakeRouters" << endl;
    cerr << "Author: Mark Gottscho" << endl;
    cerr << "Originally written for UCLA EE 201A Fall 2015" << endl;
    cerr << "=================================================" << endl << endl;
   
    //Usage
    cerr << "Ensure you have an existing OA design database before running Osprey. Also please adhere to the following command-line usage:" << endl;
    cerr << "./Osprey <DESIGN NAME> <ROUTED DESIGN NAME> <NETLIST FILE NAME> <DESIGN RULE VALUE FILE NAME>" << endl;
    cerr << endl;
    cerr << "For example:" << endl;            
    cerr << "./Osprey INV_X1 INV_X1_routed_minrule testcases/INV_X1.netlist testcases/minimum.designrules" << endl;

    if (argc < 4)
        return 0;

    oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);
    oaRegionQuery::init("oaRQSystem");
    
    //Read in design library
    cerr << "Reading design library..." << endl;
    DesignInfo designInfo;
    ReadInputArguments(argv, designInfo);
    oaDesign* design= ReadOADesign(designInfo);
    if (design == NULL) {
        cerr << "Failed to open design!" << endl;
        return -1;
    }
    
    //now, get the design rule input
    cerr << "Reading design rules..." << endl;
    ProjectDesignRules designRules(designInfo.DRFileName); 
    designRules.print();
    
    //now, read the netlist input
    cerr << "Reading netlist..." << endl;
    vector<Connection> connections=ParseNetlist(design,
                                                designInfo.netlistFileName.c_str());
    cerr << "... Found " << connections.size() << " nets" << endl;

    //Print the bounding box of the cell
    cerr << "Finding bounding box of design (DBU)..." << endl;
    oaBox bbox;
    OAHelper::getCellBBox(design, bbox);
    cerr << "... left: " << bbox.left() << endl
         << "... right: " << bbox.right() << endl
         << "... top: " << bbox.top() << endl
         << "... bottom: " << bbox.bottom() << endl
         << "... height: " << bbox.getHeight() << endl
         << "... width: " << bbox.getWidth() << endl;
       
    //Get lower coordinate of vdd rail and upper coordinate of gnd rail
    cerr << "Determining VDD/VSS rail boundaries (DBU)..." << endl;
    OAHelper oaHelper;
    int vddLowerY, gndUpperY;
    oaHelper.getPowerRailYCoordinates(design,vddLowerY, gndUpperY);
    cerr<<"... VDD Lower Y: "<<vddLowerY<<endl;
    cerr<<"... VSS Upper Y: "<<gndUpperY<<endl;

    //Compute total wirelengths
    oaBlock* topBlock = design->getTopBlock();
   
    vector<oaLayerNum> layersVec;
	layersVec.push_back(8);//M1 is OA layer 8 and GDS layer 11
	layersVec.push_back(11); //via1 is OA layer 11 and GDS layer 12
	layersVec.push_back(12);//M2 is OA layer 12 and GDS layer 13
	layersVec.push_back(13); //via2 is OA layer 13 and GDS layer 14
	layersVec.push_back(14);//M3 is OA layer 14 and GDS layer 15

    uint32_t metal1_length = 0,
             metal2_length = 0,
             metal3_length = 0;
    uint32_t num_via1 = 0,
             num_via2 = 0;
    
    for(int i=0;i<layersVec.size();i++)
    {
        //find layer and purpose number
        //ASSUME one purpose number
        oaLayerHeader* layHead = oaLayerHeader::find(topBlock,
                layersVec[i]);
        if (layHead != NULL)//if layer found
        {
           
            oaIter<oaLPPHeader> headers(layHead->getLPPHeaders());
            while (oaLPPHeader * lppHeader = headers.getNext())
            {
                //oaLayerNum lppLayerNum = lppHeader->getLayerNum();
                // oaPurposeNum lppPurpNum = lppHeader->getPurposeNum();
                // if (lppLayerNum == layerNum)
                {
                    if (!lppHeader->getShapes().isEmpty())
                    {
                        oaIter<oaShape> shapes(lppHeader->getShapes());
                        while (oaShape * shape = shapes.getNext())
                        {
                            oaType type = shape->getType();
                            oaRect *rect = NULL;
                            if (type.getName() == "Polygon")
                            {
                                rect = ((oaPolygon*) shape)->convertToRect();
                            //    cerr << "Non-rectangle Polygon detected, skipping it for metal length/via count" << endl;
                            } else if (type.getName() == "Rect")
                            {
                                rect = ((oaRect*) shape);
                            }
                            if(rect!=NULL)
                            {
                                uint32_t height,width = 0;
                                oaBox box;
                                switch (i) {
                                    case 0: //Metal1    
                                        rect->getBBox(box);
                                        if (designRules.getMetal1Direction() == 'H')
                                            metal1_length += box.getWidth();
                                        else if (designRules.getMetal1Direction() == 'V')
                                            metal1_length += box.getHeight();
                                        else if (designRules.getMetal1Direction() == 'B') {
                                            height = box.getHeight();
                                            width = box.getWidth();
                                            if (height > width)
                                                metal1_length += height;
                                            else
                                                metal1_length += width;
                                        }
                                        break;
                                    case 1: //Via1    
                                        num_via1++;
                                        break;
                                    case 2: //Metal2    
                                        rect->getBBox(box);
                                        if (designRules.getMetal2Direction() == 'H')
                                            metal2_length += box.getWidth();
                                        else if (designRules.getMetal2Direction() == 'V')
                                            metal2_length += box.getHeight();
                                        else if (designRules.getMetal2Direction() == 'B') {
                                            height = box.getHeight();
                                            width = box.getWidth();
                                            if (height > width)
                                                metal2_length += height;
                                            else
                                                metal2_length += width;
                                        }
                                        break;
                                    case 3: //Via2    
                                        num_via2++;
                                        break;
                                    case 4: //Metal3    
                                        rect->getBBox(box);
                                        if (designRules.getMetal3Direction() == 'H')
                                            metal3_length += box.getWidth();
                                        else if (designRules.getMetal3Direction() == 'V')
                                            metal3_length += box.getHeight();
                                        else if (designRules.getMetal3Direction() == 'B') {
                                            height = box.getHeight();
                                            width = box.getWidth();
                                            if (height > width)
                                                metal3_length += height;
                                            else
                                                metal3_length += width;
                                        }
                                        break;
                                    default:
                                        std::cerr << "HELP" <<endl;
                                        return 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    double metal_pitch = designRules.getMetalWidthRule() + designRules.getMetalSpaceRule();
    double num_vertical_tracks = static_cast<uint32_t>(floor(static_cast<double>(bbox.getWidth()) / static_cast<double>(metal_pitch)));
    double num_horizontal_tracks = static_cast<uint32_t>(floor(static_cast<double>(bbox.getHeight()) / static_cast<double>(metal_pitch)));
    double total_wirelength = metal1_length+metal2_length+metal3_length;
    double num_vias = num_via1 + num_via2;
    double w = max(
        1-total_wirelength/(num_vertical_tracks * bbox.getHeight() + num_horizontal_tracks * bbox.getWidth()),
        static_cast<double>(0)
    ); //wirelength metric

    double v = max(1-(num_vias / (2*connections.size())), static_cast<double>(0));
     cout <<"Cell name: " << designInfo.routedCellName << endl;
     cout << endl;
     cout<<"Metal1 wirelength: "<<metal1_length<<endl;
     cout<<"Metal2 wirelength: "<<metal2_length<<endl;
     cout<<"Metal3 wirelength: "<<metal3_length<<endl;
     cout<<"Total wirelength: " << total_wirelength << " (DBU)" <<endl;
     cout<<"---> # vertical tracks: " <<num_vertical_tracks << endl;
     cout<<"---> # horizontal tracks: " <<num_horizontal_tracks << endl;
     cout<<"# Via1: " << num_via1 << endl;
     cout<<"# Via2: " << num_via2 << endl;
     cout<<"Total # vias: " << num_vias << endl;
     cout<<"-------------- Project Metrics ------------------" << endl; 
     cout<<"w = " << w << " (normalized metric)" << endl;
     cout<<"v = " << v << " (normalized metric)" << endl;
     cout<<"-------------------------------------------------"<<endl;
    
    cout << endl << "Done." << endl;
    if(!design && design->isDesign())//if design is opened and not NULL ptr
        design -> close();
    return 0;
}
