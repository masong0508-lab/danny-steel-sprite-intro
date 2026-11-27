#pragma once
#include "hd_canvas.h"
/* Eight authored period facades: tenement, stone corner, timber,
   art house, shop, boarded shop, archive, and Potheads storefront.
   Always render behind all roof, sign and street furniture layers. */
HD_CANVAS_CODE void hd_facade_wall(uint8_t *p,int sx,int sy,int height,
                    unsigned variant,unsigned side);

/* Both cameras sample the very same 16x40 indexed surface artwork. */
const uint8_t *hd_facade_texture(unsigned variant,unsigned side);
const uint8_t *hd_facade_at(int wx,int wy,char type,unsigned side);

/* Matching shop-wall lettering for the existing first-person raycaster.
   Returns -1 outside the fixed texture-space Potheads fascia. */
HD_CANVAS_CODE int hd_potheads_wall_ink(int u,int v);

