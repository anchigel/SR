/* Authors: Mark Gottscho and Yasmine Badr
 * Email: mgottscho@ucla.edu, ybadr@ucla.edu
 * Copyright (C) 2013 Mark Gottscho and Yasmine Badr
 */

#include "Cell.h"
#include "Definitions.h"

Cell::Cell() {
    __m = 0;
    __n = 0;
    __k = 0;
    __x = 0;
    __y = 0;
    __w = 0;
    __h = 0;
    
    __distance = -1;
    __status = CellFree;
 //   __weight = 1;
    __isSource = false;
    __isSink = false;
    __netType = "";
    __netID = -1;
    __backtrace = NULL;  
    __touched = false;
    __isPin = false;
    __pinName = "";
    __needsVia = false;
}

Cell::Cell(oaUInt4 m, oaUInt4 n, oaUInt4 k, oaInt4 x, oaInt4 y, oaUInt4 w, oaUInt4 h) {
    __m = m;;
    __n = n;
    __k = k;
    __x = x;
    __y = y;
    __w = w;
    __h = h;
    
    __distance = -1;
    __status = CellFree;
  //  __weight = 1;
    __isSource = false;
    __isSink = false;
    __netType = "";
    __netID = -1;
    __backtrace = NULL;
    __touched = false;
    __isPin = false;
    __pinName = "";
    __needsVia = false;
}

Cell::~Cell() {
}

//Getters
void Cell::getPosition(oaUInt4* m, oaUInt4* n, oaUInt4 *k) {
    *m = __m;
    *n = __n;
    *k = __k;
}

void Cell::getAbsolutePosition(oaInt4* x, oaInt4* y) {
    *x = __x;
    *y = __y;
}

void Cell::getAbsoluteWH(oaUInt4* w, oaUInt4* h) {
    *w = __w;
    *h = __h;
}
 
/*oaInt4 Cell::getWeight() {
    return __weight;
}*/

bool Cell::isSource() {
    return __isSource;
}

bool Cell::isSink() {
    return __isSink;
}

CellStatus Cell::getStatus() {
    return __status;
}

oaUInt4 Cell::getDistance() {
    return __distance;
}

Cell* Cell::getBacktrace() {
    return __backtrace;
}

oaString Cell::getNetType() {
    return __netType;
}
bool Cell::isPin() {
    return __isPin;
}
oaString Cell::getPinName()
{
    return __pinName;
}
oaInt4 Cell::getNetID() {
    return __netID;
}

//setters
/*void Cell::setWeight(oaInt4 weight) {
    __weight = weight;
}*/

void Cell::setSource(bool b) {
    __isSource = b;
}

void Cell::setSink(bool b) {
    __isSink = b;
}

void Cell::setStatus(CellStatus status) {
    __status = status;
}

void Cell::setDistance(oaUInt4 dist) {
    __distance = dist;
}

void Cell::setBacktrace(Cell* prev) {
    __backtrace = prev;
}

void Cell::setNetType(oaString netType) {
    __netType = netType;
}

void Cell::setNetID(oaInt4 nid) {
    __netID = nid;
}
void Cell::setPinName(oaString pinName)
{
    __pinName=pinName;
}
void Cell::setPin(bool isPin) {
    __isPin = isPin;
}
void Cell::reset() {
   __distance = -1;
  // __weight = 1;  
   __isSource = false;
   __isSink = false;
   __touched = false;
   if (__status == CellFree || __status == CellKeepout)
        __backtrace = NULL;
   if (__status == CellKeepout)
       __status = CellFree;
}

void Cell::softReset() {
   __distance = -1;
   //__weight = 1;  
   __isSource = false;
   __isSink = false;
   __touched = false;
   if (__status == CellFree || __status == CellKeepout)
        __backtrace = NULL;
}

bool Cell::touched() {
    return __touched;
}

void Cell::touch() {
    __touched = true;
}

void Cell::setNeedsVia() {
    __needsVia = true;
}

bool Cell::needsVia() {
    return __needsVia;
}
void Cell::setIsRouted()
{
    __isRouted=true;
}
bool Cell::isRouted()
{
    return __isRouted;
}

