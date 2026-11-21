#pragma once
#include "hd_canvas.h"
/* Eight authored period facades: tenement, stone corner, timber,
   art house, shop, boarded shop, archive, and Potheads storefront.
   Always render behind all roof, sign and street furniture layers. */
void hd_facade_wall(uint8_t *p,int sx,int sy,int height,
                    unsigned variant,unsigned side);
