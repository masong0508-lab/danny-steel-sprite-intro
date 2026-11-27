#pragma once
#include "hd_canvas.h"
static inline void hd_guide_map(uint8_t *p,const Story *s,int x,int y){
 /* Full 64x30 collision map, one pixel per tile. Gold is the target;
    paper is Danny. This does not mutate routing state. */
 int gx=0,gy=0;const char *name=0;
 int has=story_goal(s,&gx,&gy,&name);
 hd_rect(p,x-2,y-2,STORY_W+4,STORY_H+4,HD_STONE_DARK);
 for(int row=0;row<STORY_H;row++)for(int col=0;col<STORY_W;col++){
  char tile=story_tile(col,row);
  unsigned ink=story_solid(col,row)?(tile=='~'?HD_GLASS:HD_STONE_DARK):HD_ROAD;
  if(tile=='T'||tile=='g')ink=HD_LEAF_DARK;
  if(tile=='=')ink=HD_STONE_LIT;
  if(story_rural(col,row)&&!story_solid(col,row)&&tile!='=')
   ink=story_terrain_vertex(s,col,row)>44?HD_STONE:HD_LEAF;
  if(story_rural(col,row)&&((col==44&&row==8)||(col==54&&row==24)||(col==56&&row==11)||(col==43&&row==22)||
      (col==42&&row==7)||(col==49&&row==6)||(col==53&&row==13)))ink=HD_LAMP;
  if(tile=='M'||tile=='R'||tile=='F'||tile=='C'||tile=='K'||tile=='B')ink=HD_COPPER;
  if(has&&col==gx&&row==gy)ink=HD_GOLD;
  if(col==s->x&&row==s->y)ink=HD_PAPER;
  hd_rect(p,x+col,y+row,1,1,ink);
 }
 /* Only a genuine reachable next step is highlighted. */
 int dir=-1,steps=-1;
 static const int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
 if(story_route(s,&dir,&steps)&&steps>0&&dir>=0&&dir<4){
  int nx=(int)s->x+dx[dir],ny=(int)s->y+dy[dir];
  if(!story_solid(nx,ny))hd_rect(p,x+nx,y+ny,1,1,HD_LEAF_LIT);
 }
}
