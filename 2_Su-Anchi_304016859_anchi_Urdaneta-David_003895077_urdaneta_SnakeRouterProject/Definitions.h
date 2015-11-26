/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

#define VDD_NET_ID 0
#define VSS_NET_ID 1

//#define M1 8
//#define M2 12
//#define via 11
struct METAL_LAYER_INFO
{
    int layerNum;
    bool vertical;//if true then vertical
	bool horizontal;
};
//static METAL_LAYER_INFO METAL_LAYERS_INFO[]={{11,true,false},{13,false,false},{15,true,false}};
static METAL_LAYER_INFO METAL_LAYERS_INFO[]={{8,false,false},{12,false,false},{14,false,false}};
static int VIA_LAYER_NUMBERS[]={11, 13};
#define PURPOSE_NUM 1

#endif	/* DEFINITIONS_H */

