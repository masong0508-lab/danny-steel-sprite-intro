#pragma once
#include <stdint.h>
/* Grounded scenery is shared by both views, with fixed positions.
   Decorative small props are not new quest anchors or solid walls. */
typedef struct {uint8_t x,y,kind;} CityFurniture;
static const CityFurniture city_furniture[]={
 {31,2,'j'},
 {27,6,'j'},
 {33,7,'j'},
 {32,12,'j'},
 {29,16,'j'},
 {35,16,'j'},
 {10,19,'j'},
 {12,26,'j'},
 {21,19,'j'},
 {24,24,'j'},
 {34,22,'j'},
 {37,23,'j'},
 {9,22,'u'},
 {12,24,'u'},
 {36,6,'u'},
 {28,16,'u'},
 {31,22,'u'},
 {22,26,'u'},
 {26,6,'i'},
 {31,18,'i'},
 {11,24,'i'},
 {21,26,'i'},
 {34,23,'i'},
};
