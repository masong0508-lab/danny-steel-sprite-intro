#pragma once
/* Uses the PotPoint and fixed-point projection defined in hd_potheads_set.h. */
static HD_CANVAS_CODE void pot_shape(uint8_t *p,const PotPoint *v,unsigned count,unsigned ink){
 /* Scan-convert actual polygon spans, including the loose concave hem.
    Divide once per edge; never divide once per painted pixel or row. */
 typedef struct {int low,high,x,step;} Edge;
 Edge edges[8];unsigned n=0;
 int low=135,high=-1,minx=HD_W,maxx=-1;
 if(count>8u)return;
 for(unsigned i=0;i<count;i++){
  if(v[i].y<low)low=v[i].y;
  if(v[i].y>high)high=v[i].y;
  if(v[i].x<minx)minx=v[i].x;
  if(v[i].x>maxx)maxx=v[i].x;
 }
 if(high<0||low>=135||maxx<0||minx>=HD_W)return;
 if(low==high){hd_rect(p,minx,low,maxx-minx+1,1,ink);return;}
 if(low<0)low=0;
 if(high>134)high=134;
 for(unsigned i=0;i<count;i++){
  PotPoint a=v[i],b=v[(i+1u)%count];
  if(a.y>b.y){PotPoint swap=a;a=b;b=swap;}
  if(a.y==b.y||b.y<=low||a.y>high)continue;
  int first=a.y<low?low:a.y;
  int step=(b.x-a.x)*65536/(b.y-a.y);
  edges[n++]=(Edge){first,b.y,a.x*65536+step*(first-a.y),step};
 }
 int run_x=0,run_w=0,run_y=low,run_h=0;
 for(int y=low;y<=high;y++){
  int hits[8];unsigned used=0;
  for(unsigned i=0;i<n;i++)if(y>=edges[i].low&&y<edges[i].high){
   int x=edges[i].x;unsigned at=used;
   while(at&&hits[at-1u]>x){hits[at]=hits[at-1u];at--;}
   hits[at]=x;used++;edges[i].x+=edges[i].step;
  }
  /* Most sleeve/leg/head rows have one span. Equal adjacent spans
     are one rectangle, not repeated clip/dispatch calls. Flush before
     multi-span rows so concave cloth and self-crossing edges stay exact. */
  if(used==2u){
   int x=pot_set_round(hits[0],65536),xx=pot_set_round(hits[1],65536);
   int w=xx-x+1;
   if(run_h&&x==run_x&&w==run_w){run_h++;continue;}
   if(run_h)hd_rect(p,run_x,run_y,run_w,run_h,ink);
   run_x=x;run_y=y;run_w=w;run_h=1;
   continue;
  }
  if(run_h){hd_rect(p,run_x,run_y,run_w,run_h,ink);run_h=0;}
  for(unsigned i=0;i+1u<used;i+=2u){
   int x=pot_set_round(hits[i],65536),xx=pot_set_round(hits[i+1u],65536);
   hd_rect(p,x,y,xx-x+1,1,ink);
  }
 } if(run_h)hd_rect(p,run_x,run_y,run_w,run_h,ink);
}
