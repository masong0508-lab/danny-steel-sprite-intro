#pragma once
/* Exact first visible sample of a monotone projected span. Preserve the
   old inclusive/exclusive edges, but skip entire offscreen prefixes. */
static inline int hd_iso_prefix(int start,int lower,unsigned shift){
 return start<=lower?((lower-start)>>shift)+1:0;
}
static inline int hd_iso_ground_prefix(int sx,int sy){
 int a=hd_iso_prefix(sx,-32,4u),b=hd_iso_prefix(sy,-18,3u);
 return a>b?a:b;
}
