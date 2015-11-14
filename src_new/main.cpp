/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */


#include <cstdlib>
#include <iostream>
#include "InputOutputManipulator.h"
#include "ProjectDesignRules.h"
#include "OAHelper.h"
#include "Grid.h"
#include "MazeRouter.h"
#include "RouteGeometry.h"
#include "PostProcessor.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv)
{
    //Hello World
    cout << "=================================================" << endl;
    cout << "This is SnakeRouter" << endl;
    cout << "Authors: Yasmine Badr and Mark Gottscho" << endl;
    cout << "Originally written for UCLA EE 201A Fall 2013" << endl;
    cout << "UCLA EE 201A Fall 2015 Project Edition" << endl;
    cout << endl;
    cout << "<YOUR TEAM NUMBER HERE>" << endl;
    cout << "<YOUR NAMES HERE>" << endl;
    cout << "<YOUR STUDENT IDS HERE>" << endl;
    cout << "=================================================" << endl << endl;
   
    //Usage
    cout << "Ensure you have an existing OA design database before running SnakeRouter. Also please adhere to the following command-line usage:" << endl;
    cout << "./SnakeRouter <DESIGN NAME> <ROUTED DESIGN NAME> <NETLIST FILE NAME> <DESIGN RULE VALUE FILE NAME>" << endl;
    cout << endl;
    cout << "For example:" << endl;            
    cout << "./SnakeRouter INV_X1 INV_X1_routed_minrule testcases/INV_X1.netlist testcases/minimum.designrules > $LOGS/INV_X1_routed_minrule.log" << endl;

    oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);
    oaRegionQuery::init("oaRQSystem");
    
    //Read in design library
    cout << "Reading design library..." << endl;
    DesignInfo designInfo;
    InputOutputManipulator::ReadInputArguments(argv, designInfo);
    oaDesign* design= InputOutputManipulator::ReadOADesign(designInfo);
    
    //now, get the design rule input
    cout << "Reading design rules..." << endl;
    ProjectDesignRules designRules(designInfo.DRFileName); 
    designRules.print();
    
    //now, read the netlist input
    cout << "Reading netlist..." << endl;
    InputOutputManipulator io;
    vector<Connection> connections=io.ParseNetlist(design,
                                                designInfo.netlistFileName.c_str());
    cout << "... Found " << connections.size() << " nets" << endl;

    //Print the bounding box of the cell
    cout << "Finding bounding box of design (DBU)..." << endl;
    oaBox bbox;
    OAHelper::getCellBBox(design, bbox);
    cout << "... left: " << bbox.left() << endl
         << "... right: " << bbox.right() << endl
         << "... top: " << bbox.top() << endl
         << "... bottom: " << bbox.bottom() << endl
         << "... height: " << bbox.getHeight() << endl
         << "... width: " << bbox.getWidth() << endl;
       
    //Get lower coordinate of vdd rail and upper coordinate of gnd rail
    cout << "Determining VDD/VSS rail boundaries (DBU)..." << endl;
    OAHelper oaHelper;
    int vddLowerY, gndUpperY;
    oaHelper.getPowerRailYCoordinates(design,vddLowerY, gndUpperY);
    cout<<"... VDD Lower Y: "<<vddLowerY<<endl;
    cout<<"... VSS Upper Y: "<<gndUpperY<<endl;

    //Construct MazeRouter
    cout << "Constructing a MazeRouter..." << endl;
    MazeRouter* router = new MazeRouter(&connections, &designRules, vddLowerY, gndUpperY, &bbox);
   
    cout << "Starting the MazeRouter, try #1 with given design rules..." << endl;
    bool routerSuccess = router->route();
    if (routerSuccess)
		 cout << "Routing try #1 SUCCESS with given design rules!" << endl;
	 else {
		 cout << "Routing try #1 FAILED with given design rules!" << endl;
		 delete router;
		 designRules.setMetalSpaceRule(5); //5nm spacing rule to improve density
		 designRules.setContactViaExtensionRule(0); //Eliminate extension rule to make things even easier
		 cout << "Constructing a MazeRouter..." << endl;
		 router = new MazeRouter(&connections, &designRules, vddLowerY, gndUpperY, &bbox);
		 designRules.print();
		 cout << "Starting the MazeRouter, try #2 with relaxed metal spacing and via extension rules..." << endl;
		 routerSuccess = router->route();
		 if (routerSuccess) {
			 cout << "Routing try #2 SUCCESS with relaxed metal spacing and via extension rules!" << endl;
		 }
		 else {
			 cout << "Routing try #2 FAILED with relaxed metal spacing and via extension rules!" << endl;
			 delete router;
			 designRules.setMetalSpaceRule(5); //5nm spacing rule to allow better density
			 designRules.setContactViaExtensionRule(0); //0 extension to make things even easier
			 designRules.setMetalWidthRule(5); //5nm metal width rule to allow super thin wires for better density
			 designRules.setViaDimensionRule(5); //via width same as metal width
                         cout << "Constructing a MazeRouter..." << endl;
			 router = new MazeRouter(&connections, &designRules, vddLowerY, gndUpperY, &bbox);
			 cout << "Starting the MazeRouter, try #3 with relaxed metal width/spacing and via extension rules..." << endl;
			 designRules.print();
			 routerSuccess = router->route();
			 if (routerSuccess)
				 cout << "Routing try #3 SUCCESS with relaxed metal width/spacing and via extension rules!" << endl;
		    else
				 cout << "Routing try #3 FAILED with relaxed metal width/spacing and via extension rules! WE GIVE UP!" << endl;
		 }
	 }
    
    
    cout << "Finished routing phase, generating layout geometry..." << endl;
    RouteGeometry routGeom;
    routGeom.mazeToGeometry(router->getGrid(),design, designRules);

    //Do postprocessing to merge some polygons
    cout << "Post-processing layout..." << endl;
    PostProcessor postProc;
    postProc.Postprocess(router->getGrid(),design, designRules);
    postProc.dumpStats(design);

     //Save the routed version
    InputOutputManipulator::SaveRoutedCell(designInfo, design);
    cout << endl << "Done." << endl;
    if(!design && design->isDesign())//if design is opened and not NULL ptr
        design -> close();

    return 0;
}

