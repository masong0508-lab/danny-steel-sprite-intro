#include "story_draw.h"
#include "story_checks.h"

#define RGB(r,g,b) ((r)|((g)<<5)|((b)<<10))
const uint16_t story_palette[32] = {
 RGB(4,5,8), RGB(15,20,25), RGB(20,23,26), RGB(9,11,16),
 RGB(12,13,17),RGB(17,17,19),RGB(23,19,18),RGB(29,24,15),
 RGB(12,13,15),RGB(16,16,18),RGB(23,21,20),RGB(15,19,13),
 RGB(8,14,10),RGB(21,25,17),RGB(14,10,8),RGB(30,29,25),
 RGB(25,13,13),RGB(11,15,24),RGB(27,22,13),RGB(5,7,11),
 RGB(31,30,26),RGB(6,8,13),RGB(24,19,15),RGB(18,10,11),
 RGB(8,7,11),RGB(8,16,20),RGB(18,25,28),RGB(18,24,20),
 RGB(23,18,20),RGB(10,13,18),RGB(22,20,14),RGB(21,17,14)
};
#undef RGB

static int16_t sin_q12[256];
void story_draw_init(void) {
    for (int i=0;i<256;i++) {
        int h=i&127,p=h*(128-h);
        int v=65536*p/(81920-4*p);
        sin_q12[i]=(int16_t)(i>=128?-v:v);
    }
}
static void put(uint8_t *p,int x,int y,int c) {
    if((unsigned)x<120u&&(unsigned)y<80u) p[y*120+x]=(uint8_t)c;
}
static void box(uint8_t *p,int x,int y,int w,int h,int c) {
    int x2=x+w,y2=y+h;
    if(x<0)x=0;
    if(y<0)y=0;
    if(x2>120)x2=120;
    if(y2>80)y2=80;
    for(int yy=y;yy<y2;yy++)for(int xx=x;xx<x2;xx++)p[yy*120+xx]=(uint8_t)c;
}
static const uint8_t font[36][5] = {
 {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},
 {7,4,6,4,7},{7,4,6,4,4},{3,4,5,5,3},{5,5,7,5,5},
 {7,2,2,2,7},{1,1,1,5,2},{5,5,6,5,5},{4,4,4,4,7},
 {5,7,7,5,5},{5,7,7,7,5},{2,5,5,5,2},{6,5,6,4,4},
 {2,5,5,3,1},{6,5,6,5,5},{3,4,2,1,6},{7,2,2,2,2},
 {5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},
 {5,5,2,2,2},{7,1,2,4,7},
 {7,5,5,5,7},{2,6,2,2,7},{6,1,2,4,7},{6,1,2,1,6},
 {5,5,7,1,1},{7,4,6,1,6},{3,4,7,5,7},{7,1,1,2,2},
 {7,5,7,5,7},{7,5,7,1,6}
};
static void glyph(uint8_t *p,int x,int y,char ch,int color) {
    if(ch>='a'&&ch<='z')ch=(char)(ch-'a'+'A');
    int index=ch>='A'&&ch<='Z'?ch-'A':ch>='0'&&ch<='9'?26+ch-'0':-1;
    if(index>=0) {
        for(int j=0;j<5;j++)for(int i=0;i<3;i++)
            if(font[index][j]&(4>>i))put(p,x+i,y+j,color);
    }
    else if(ch=='.')put(p,x+1,y+4,color);
    else if(ch==':'){put(p,x+1,y+1,color);put(p,x+1,y+3,color);}
    else if(ch=='!'){for(int j=0;j<3;j++)put(p,x+1,y+j,color);put(p,x+1,y+4,color);}
    else if(ch=='?'){put(p,x+1,y,color);put(p,x+2,y+1,color);put(p,x+1,y+2,color);put(p,x+1,y+4,color);}
    else if(ch=='-'){for(int i=0;i<3;i++)put(p,x+i,y+2,color);}
    else if(ch=='/'){for(int j=0;j<5;j++)put(p,x+(2-j/2),y+j,color);}
    else if(ch=='\''){put(p,x+1,y,color);put(p,x+1,y+1,color);}
    else if(ch==','){put(p,x+1,y+4,color);}
}
static void text(uint8_t *p,int x,int y,const char *s,int color,int max) {
    for(int i=0;s[i]&&i<max;i++)glyph(p,x+i*4,y,s[i],color);
}
static int abs_i(int x){return x<0?-x:x;}
static int tile_color(int x,int y,int px,int py,int tick) {
    char t=story_tile(x,y);
    (void)tick;
    if(t=='#'||t=='A')return (px==0||py==0)?3:(px==7||py==7)?5:4;
    if(t=='T')return ((px+py+x+y)&3)==0?12:13;
    if(t=='~')return py==3?26:25;
    if(t==','||t=='=')return (px==0&&((x+y)&1)==0)||(py==0&&(y&1)==0)?9:10;
    if(t=='g')return (px==3&&py==5)||(((x+y)&3)==0&&px==6&&py==2)?13:11;
    return (px==0&&(x&3)==0)||(py==0&&(y&3)==0)?9:8;
}
static void tile(uint8_t *p,int sx,int sy,int wx,int wy,int tick) {
    char t=story_tile(wx,wy);
    for(int py=0;py<8;py++)for(int px=0;px<8;px++)put(p,sx+px,sy+py,tile_color(wx,wy,px,py,tick));
    if(t=='#'||t=='A'){
        box(p,sx+1,sy+1,6,5,4);box(p,sx+2,sy+2,4,2,t=='A'?18:23);
        if(((wx+wy)&1)==0){put(p,sx+3,sy+3,7);put(p,sx+4,sy+3,7);}
    } else if(t=='T'){
        box(p,sx+3,sy+5,2,3,14);box(p,sx+1,sy+2,6,4,12);
        box(p,sx+2,sy+1,4,5,13);put(p,sx+3,sy+1,27);
    } else if(t=='~'){
        if((wx+wy+tick/12)&1)box(p,sx+1,sy+3,4,1,26);
    } else if(t==','||t=='='){
        if((wx*5+wy)&3)put(p,sx+3,sy+3,22);
    }
}
static void person(uint8_t *p,int x,int y,int kind,int scale,int tick){
    /* Head, hair, shoulders, coat, feet; two readable animation poses. */
    int w=scale/3;if(w<2)w=2;
    int head=scale/4;if(head<2)head=2;
    int top=y-scale;
    int coat=kind=='F'?16:kind=='C'?17:kind=='P'?25:kind=='V'?28:kind=='Q'?14:kind=='W'?13:kind=='H'?17:23;
    box(p,x-head/2,top,head,head,31);
    box(p,x-head/2,top,head,1,kind=='F'?24:3);
    box(p,x-w/2,top+head,w,scale/2,coat);
    box(p,x-w/2,top+head+scale/2,w/2,scale/4,3);
    box(p,x+1,top+head+scale/2,w/2,scale/4,3);
    if(scale>8){put(p,x-1,top+head/2,19);put(p,x+1,top+head/2,19);}
    if(kind=='C'&&scale>8)box(p,x+w/2,top+head+scale/4,3,2,18);
    if(kind=='F'&&(tick/10&1))put(p,x+head/2,top+head+1,31);
    if(kind=='P'||kind=='V'||kind=='Q'){put(p,x-head/2,top-2,18);put(p,x+head/2,top-2,18);}
}
static void marker(uint8_t *p,int x,int y,int type,int size,int tick) {
    if(type=='L') {
        box(p,x-size/2,y-size/2,size,size*2/3,15);
        box(p,x-size/2+1,y-size/2+1,size-2,1,22);
        put(p,x,y-size/2+2,16);
    } else if(type=='R') {
        box(p,x-size/2,y-size/2,size,size,7);
        box(p,x-size/2+1,y-size/2+1,size-2,size-2,26);
        box(p,x,y,1,size/2,18);
    } else if(type=='K') {
        box(p,x-size/2,y-size/2,size,size,20);
        box(p,x-1,y-size/2+1,3,size-2,16);
        box(p,x-size/2+1,y-1,size-2,2,16);
    } else if(type=='M') {
        box(p,x-size/2,y-size/2,size,size,3);
        box(p,x-size/2+1,y-size/2+1,size-2,size-2,15);
        put(p,x,y,16);
    } else if(type=='B') {
        box(p,x,y-size,1,size,7);box(p,x-size/2,y-size, size, size/3,16);
        box(p,x-size/3,y-size+1,size*2/3,1,15);
    } else person(p,x,y,type,size,tick);
}
/* A grounded, pulsing waypoint in the shared world coordinate system.
   Small enough not to obscure a whole 8x8 map tile. */
static void map_waypoint(uint8_t *p,int sx,int sy,int frame){
    int c=((frame/15)&1)?20:18;
    put(p,sx,sy-4,c);put(p,sx-1,sy-3,c);put(p,sx+1,sy-3,c);
    put(p,sx-2,sy-2,3);put(p,sx+2,sy-2,3);
    put(p,sx,sy-2,20);put(p,sx,sy-1,18);
}
static void overhead(const Story *s,int tick,uint8_t *p) {
    int cx=s->x-7, cy=s->y-4;
    for(int ty=0;ty<11;ty++)for(int tx=0;tx<16;tx++){
        int wx=cx+tx,wy=cy+ty;
        int px=tx*8-1,py=ty*8-1;
        tile(p,px,py,wx,wy,tick);
        char c=story_tile(wx,wy);
        if(c=='L'||c=='C'||c=='F'||c=='B'||c=='P'||c=='V'||c=='Q'||c=='S'||c=='W'||c=='H'||c=='R'||c=='M'||c=='K') {
            if(c=='S')box(p,px+1,py+1,6,5,26);
            else if(c=='H')box(p,px+1,py+1,6,5,18);
            else marker(p,px+4,py+7,c,7,tick);
        }
    }
    int gx,gy;const char *label;
    if(story_goal(s,&gx,&gy,&label)){
        int wx=gx-cx,wy=gy-cy;
        (void)label;
        if(wx>=0&&wx<16&&wy>=0&&wy<11)
            map_waypoint(p,wx*8+3,wy*8+3,tick);
    }
    int dx=0,dy=0;
    if(s->anim){dx=(s->prev_x-(int)s->x)*s->anim*2;dy=(s->prev_y-(int)s->y)*s->anim*2;}
    int px=60+dx,py=36+dy;
    box(p,px-2,py-4,5,4,31);box(p,px-2,py-5,5,1,24);
    box(p,px-3,py,7,4,16);box(p,px-2,py+4,2,3,3);box(p,px+1,py+4,2,3,3);
    put(p,px+(s->facing==0?2:s->facing==2?-2:0),py-2,19);
}
/* Perspective floor distances are entirely fixed for 120x80 mode.
   Storing forty 16-bit constants in ROM avoids 4,800 divides per frame. */
static const uint16_t floor_row_dist[40] = {
    2300,2200,1466,1100,880,733,628,550,488,440,400,366,338,314,293,275,258,244,231,220,209,200,191,183,176,169,162,157,151,146,141,137,133,129,125,122,118,115,112,110
};
static void first_person(const Story *s,int tick,uint8_t *p) {
    static int depth[120];
    int facing=(s->facing*64)&255;
    int dirx=sin_q12[(facing+64)&255],diry=sin_q12[facing];
    int camx=(int)s->x*256+128,camy=(int)s->y*256+128;
    if(s->anim){camx+=(s->prev_x-(int)s->x)*s->anim*64;camy+=(s->prev_y-(int)s->y)*s->anim*64;}
    for(int y=0;y<80;y++)box(p,0,y,120,1,y<22?1:y<39?2:y<60?8:9);
    /* Horizon architecture remains attached to the world view rather than to a random camera cut. */
    for(int x=0;x<120;x++) {
        int skyline=26+((x/11+facing/32)%5)*3;
        for(int y=skyline;y<39;y++)put(p,x,y,29);
        if(x%11==3)for(int y=skyline+3;y<37;y+=5)put(p,x,y,7);
        int off=(x-60)*38/60, ang=(facing+off+256)&255;
        int vx=sin_q12[(ang+64)&255],vy=sin_q12[ang];
        int projection_cos=sin_q12[(off+64+256)&255];
        /* Perspective floor projection: near details move with Danny, not with the camera backdrop. */
        for(int y=40;y<80;y++){
            int dist=floor_row_dist[y-40];
            int wx=camx+vx*dist/4096,wy=camy+vy*dist/4096;
            int tx=wx>>8,ty=wy>>8;
            char c=story_tile(tx,ty);
            int color=c=='~'?25:c==','||c=='='?10:c=='g'||c=='T'?11:c=='#'?4:8;
            int fx=wx&255,fy=wy&255;
            if((c==','||c=='.')&&(fx<5||fy<5))color=9;
            if(c=='~'&&((fx+fy+tick*2)&127)<7)color=26;
            put(p,x,y,color);
        }
        depth[x]=2600;
        int rayx=camx+(vx>>7),rayy=camy+(vy>>7);
        int stepx=vx>>7,stepy=vy>>7;
        for(int step=2;step<82;step++){
            rayx+=stepx;rayy+=stepy;
            int dist=step*32,tx=rayx>>8,ty=rayy>>8;
            char t=story_tile(tx,ty);
            if(t=='#'||t=='A'||t=='T'){
                int correct=(dist*projection_cos)/4096;
                int height=8500/(correct+48);if(height>80)height=80;
                if(height<2)height=2;
                int top=39-height/2,bottom=39+height/2;
                int color=t=='T'?12:t=='A'?17:((tx+ty)&1)?5:4;
                if(dist>1152)color=t=='T'?11:29;
                for(int y=top;y<bottom;y++) {
                    int detail=color;
                    if(t=='#'&&y>top+2&&((y-top)/6)%3==1&&x%10>3&&x%10<8)detail=7;
                    if(t=='A'&&y<top+height/3&&x%6!=0)detail=18;
                    if(t=='T'&&(((x+y+tick/20)&7)==0))detail=13;
                    put(p,x,y,detail);
                }
                depth[x]=correct;break;
            }
        }
    }
    /* Real world-position billboards; a wall can occlude them. */
    for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
        char c=story_tile(x,y);
        if(c!='C'&&c!='F'&&c!='B'&&c!='L'&&c!='P'&&c!='V'&&c!='Q'&&c!='S'&&c!='W'&&c!='H'&&c!='R'&&c!='M'&&c!='K')continue;
        int dx=x*256+128-camx,dy=y*256+128-camy;
        int forward=(dx*dirx+dy*diry)/4096;
        int side=(dy*dirx-dx*diry)/4096;
        if(forward<80||forward>2300||abs_i(side)>forward*2)continue;
        int sx=60+(side*63)/(forward+1);
        int height=8400/(forward+64);if(height>64)height=64;if(height<3)height=3;
        if(sx<0||sx>=120||forward>depth[sx])continue;
        if(c=='S'||c=='H'){box(p,sx-height/4,39-height/2,height/2,height/2,c=='S'?26:18);}
        else marker(p,sx,39+height/2,c,height,tick);
    }
}
/* A hybrid battle arena drawn over the actual perspective scene. No new bitmap,
   textures, trigonometry or per-frame heap allocations. Projected lane lines and
   billboards are consistent with the 3D exploration camera. */
static void line(uint8_t *p,int x0,int y0,int x1,int y1,int c) {
    int dx=abs_i(x1-x0),dy=abs_i(y1-y0),sx=x0<x1?1:-1,sy=y0<y1?1:-1,err=dx-dy;
    for(;;){ put(p,x0,y0,c);if(x0==x1&&y0==y1)break;
        int e=err*2;if(e>-dy){err-=dy;x0+=sx;}if(e<dx){err+=dx;y0+=sy;}}
}
static void cult_arena(const Story *s,int tick,uint8_t *p) {
    first_person(s,tick,p);
    /* Far converging lines establish three traversable floor lanes. */
    for(int i=-2;i<=2;i++)line(p,60+i*5,43,60+i*19,73, i==0?26:22);
    for(int y=51;y<=72;y+=7) {
        int spread=(y-43)*19/30;
        line(p,60-spread*2,y,60+spread*2,y, y==72?18:29);
    }
    /* Distinct readable silhouettes, including comedy-costume ears. */
    int enemy=s->cult_enemy;
    box(p,48,17,25,4,21);
    text(p,49,17,story_enemy_name(s),20,20);
    person(p,60,49,enemy==0?'P':enemy==1?'V':'Q',28,tick);
    box(p,51,29,5,1,15);box(p,64,29,5,1,15);
    if(s->battle==BATTLE_DODGE) {
        int beat=s->battle_timer/15;
        int phase=s->battle_timer%15;
        if(beat<4) {
            int lane=story_attack_lane(s,beat);
            int x=60+(lane-1)*(7+phase*17/15),y=48+phase*22/15;
            int size=4+phase/3;
            box(p,x-size/2,y-size/2,size,size,23);
            box(p,x-size/2+1,y-size/2+1,size-2,size-2,20);
            if(size>6){text(p,x-2,y-2,"!",16,1);}
        }
    }
    /* Danny is screen-anchored at the proper depth of the floor; lateral movement
       is the actual logical lane being collision-tested by story_tick(). */
    int hx=60+((int)s->battle_lane-1)*37;
    box(p,hx-5,65,10,2,3);
    box(p,hx-3,59,6,6,s->battle_parry?26:16);
    box(p,hx-2,56,4,4,31);
    if(s->battle_parry){box(p,hx-8,57,2,12,26);box(p,hx+6,57,2,12,26);}
    box(p,0,0,120,12,21);
    text(p,3,2,s->cult_stage>=CULT_RESOLVED?"ORCHARD / AFTER":s->cult_stage?"ORCHARD / DURING":"ORCHARD / BEFORE",20,26);
    for(int i=0;i<4;i++)box(p,95+i*6,3,4,5,i<s->battle_focus?18:3);
    if(s->battle==BATTLE_MENU) {
        box(p,0,70,120,10,21);
        text(p,3,71,"FACTS  CARE  EXIT",20,25);
        int arrowx=s->battle_choice==0?3:s->battle_choice==1?31:55;
        box(p,arrowx,78,17,1,26);
        text(p,81,71,"A PICK",18,8);
    } else {
        box(p,0,71,120,9,21);
        text(p,3,73,"LEFT RIGHT DODGE",20,17);
        text(p,81,73,"A PARRY",26,8);
    }
}
/* Dedicated low-memory cinematic scenes. They use the same 120x80 buffer
   as roaming and fighting, with no textures, dynamic allocations or new VRAM. */
static void scene_sky(uint8_t *p,int frame,int muted){
    for(int y=0;y<80;y++)box(p,0,y,120,1,y<16?(muted?29:26):y<38?(muted?17:25):8);
    for(int i=0;i<10;i++){
        int x=i*15-7-((frame/32+i*5)%17);
        int h=20+(i*7)%17;
        box(p,x,46-h,13,h,3);
        for(int wy=47-h;wy<41;wy+=7)for(int wx=x+3;wx<x+12;wx+=5)put(p,wx,wy,7);
    }
}
static void scene_rooftop(const Story *s,int frame,uint8_t *p){
    int f=s->finale_frame;
    scene_sky(p,frame,1);
    box(p,0,48,120,32,9);box(p,0,47,120,3,6);
    /* The painting is a physical object in the same perspective every shot. */
    box(p,75,16,3,45,14);box(p,58,21,34,27,7);
    box(p,60,23,30,23,26);
    for(int x=62;x<90;x+=6)box(p,x,35+(x%4),4,9,25);
    box(p,60,42,30,4,13);box(p,57,50,36,2,14);
    box(p,68,52,3,17,14);box(p,82,52,3,17,14);
    /* Danny reaches *towards* the painting, not the outstretched title pose. */
    int sinking=f>=260?(f-260)/24:0;if(sinking>10)sinking=10;
    int head_y=35+sinking;
    int body_x=f>=300?54:43;
    box(p,body_x,head_y,8,8,31);box(p,body_x,head_y,8,2,24);
    box(p,body_x-1,head_y+8,10,15,16);
    box(p,body_x+1,head_y+23,3,9,3);box(p,body_x+6,head_y+23,3,9,3);
    if(f<240){
        /* A small arm movement, with a brush physically meeting the canvas. */
        int arm_y=head_y+12+((f/18)&1);
        for(int x=52;x<66;x++)put(p,x,arm_y-(x-52)/6,31);
        for(int x=65;x<72;x++)put(p,x,arm_y-3-(x-65),14);
    } else if(f<300){
        for(int x=52;x<60;x++)put(p,x,head_y+16+(x-52)/3,31);
    } else {
        /* The brush drops beside the canvas; no graphic injury or close-up. */
        box(p,58,59,8,1,14);
    }
    if(f>=360){
        box(p,0,66,120,14,21);
        text(p,6,70,"THE CITY KEPT ITS SKY",20,28);
    }
}
static void scene_clinic(uint8_t *p,int frame){
    (void)frame;
    box(p,0,0,120,80,8);box(p,0,2,120,8,20);
    box(p,6,13,42,35,17);box(p,9,16,36,29,26);
    box(p,10,33,35,12,25);box(p,13,38,7,7,3);box(p,29,34,7,11,3);
    box(p,60,31,55,3,6);box(p,61,34,6,22,3);box(p,109,34,6,22,3);
    box(p,69,22,14,12,31);box(p,67,34,20,12,20);
    box(p,93,24,14,11,31);box(p,90,35,20,13,17);
    box(p,73,32,6,2,25);box(p,94,28,10,2,29);
    box(p,8,52,38,8,22);box(p,13,55,28,2,20);
}
static void scene_notice(uint8_t *p,int frame){
    scene_sky(p,frame,0);
    box(p,0,58,120,22,9);
    box(p,45,15,31,45,3);box(p,47,17,27,41,25);
    box(p,49,20,23,30,20);box(p,52,23,17,3,18);
    text(p,53,34,"CLINIC",16,6);
    box(p,57,53,7,2,7);
}
static void scene_record(uint8_t *p,int frame){
    scene_sky(p,frame,0);
    box(p,0,48,120,32,3);
    box(p,12,14,95,52,21);box(p,16,17,87,46,10);
    box(p,18,19,83,6,16);
    text(p,21,19,"THE BORE ARCHIVES",20,20);
    for(int x=34;x<86;x++)for(int y=26;y<60;y++){
        int dx=x-60,dy=y-43;
        if(dx*dx+dy*dy<256)put(p,x,y,4);
        else if(dx*dx+dy*dy<289 && dx*dx+dy*dy>256)put(p,x,y,15);
    }
    box(p,58,41,5,5,26);
    text(p,14,68,"SONGS OUTLIVE THE DAY",20,28);
}
/* Five daily moments are optional. The tally is shared by both views. */
static int memories_today(const Story *s){
    unsigned bits=s->daily_mask&STORY_DAY_ALL;
    int count=0;
    for(int i=0;i<5;i++)count+=(bits>>i)&1u;
    return count;
}
static void panel(uint8_t *p,int top,int bottom){
    box(p,0,top,120,bottom-top,21);box(p,0,top,120,1,22);
}
/* A genuine, accessible check-list for game consequences.
   This is computed from live play state, not SRAM checksum markers.
   Access: START (pause), R (open/next page), B (close), START (resume). */
static void check_icon(uint8_t *p,int x,int y,int result,int harmful){
    box(p,x,y,7,7,3);
    box(p,x+1,y+1,5,5,result?(harmful?16:12):5);
    if(!result){box(p,x+2,y+2,3,3,3);return;}
    if(harmful){
        put(p,x+2,y+2,20);put(p,x+4,y+4,20);
        put(p,x+3,y+3,20);put(p,x+4,y+2,20);
        put(p,x+2,y+4,20);
    }else{
        put(p,x+2,y+3,20);put(p,x+3,y+4,20);
        put(p,x+4,y+3,20);put(p,x+5,y+2,20);
    }
}
static void consequence_board(uint8_t *p,const Story *s){
    unsigned page=story_check_page(s);
    if(page>=STORY_CHECK_PAGE_COUNT)page=0;
    int risky=page==2u;
    unsigned total=risky?(unsigned)story_bad_count(s):
                          story_check_count_page(s,page);
    box(p,2,9,116,71,3);
    box(p,4,11,112,67,21);
    box(p,4,11,112,13,24);
    text(p,8,13,page==0?"MAIN STORY":page==1?"LIFE AND LEGACY":
                         "CHOICES AND RISKS",20,24);
    char number[]="0/5";
    number[0]=(char)('0'+total);
    number[2]=risky?'4':'5';
    text(p,99,13,number,20,3);
    text(p,8,20,page==0?"5 CORE MILESTONES":
        page==1?"PEOPLE WORK AND CARE":"HARM IS NEVER AUTOMATIC",18,27);
    box(p,8,25,103,2,3);
    box(p,8,25,(int)total*103/(risky?4:5),2,risky?16:13);
    for(unsigned i=0;i<STORY_CHECKS_PER_PAGE;i++){
        unsigned id=page*STORY_CHECKS_PER_PAGE+i;
        int y=29+(int)i*9;
        int checked=(int)story_check_live(s,id);
        box(p,7,y-1,106,9,(i&1u)?3:21);
        check_icon(p,9,y,checked,risky&&i<4u);
        text(p,21,y+1,story_check_name(id),
             checked?(risky&&i<4u?18:20):10,23);
    }
    box(p,4,73,112,5,24);
    text(p,6,73,"R NEXT  B BACK  START PLAY",20,27);
}

/* Short, staged caption beats; the audio remains the master timing.
   Unlike a blocking textbox these never wait for A during a 72-second cue. */
/* One compact menu is reused for all five locations in either camera mode. */
static void activity_menu(uint8_t *p,const Story *s){
    const char *where="",*left="",*right="";
    const char *effect="";
    unsigned selected=s->activity_choice;
    switch(s->activity){
    case 'F':where="MARA / AFTERNOON";left="WALK";right="TALK";
        effect=selected?"BOND +1 ENERGY +1":"BOND +2 ENERGY -1";break;
    case 'L':where="LETTER / MEMORIES";left="WRITE";right="KEEP";
        effect=selected?"SPARK +1 ENERGY +1":"SPARK +2 ENERGY -1";break;
    case 'C':where="BORE / A FORM";left="FILE";right="JOKE";
        effect=selected?"BOND +1 ENERGY +1":"SPARK +1 ENERGY -1";break;
    case 'M':where="BORE / MUSIC STUDIO";left="ARRANGE";right="LISTEN";
        effect=selected?"SPARK +1 ENERGY +1":"MUSIC STAGE +1";break;
    case 'm':where="BORE TAPE";left="BUILD";right="SOFTEN";
        effect=selected?"STAGE +1 ENERGY +1":"STAGE +1 / MORE WORK";break;
    case 'r':where="ROOFTOP ART";left="LAYER";right="LIGHT";
        effect=selected?"STAGE +1 ENERGY +1":"STAGE +1 / MORE WORK";break;
    default:where="ROOFTOP / THE SKY";left="PAINT";right="REST";
        effect=selected?"ENERGY +2 SPARK +1":"PAINT STAGE +1";break;
    }
    panel(p,42,80);
    box(p,3,43,2,34,18);
    text(p,8,45,where,20,25);
    if(s->activity=='M'||s->activity=='m'||s->activity=='r'){
        char progress[]="0 / 4";
        unsigned phase=s->activity=='r'?s->art_steps:s->song_steps;
        progress[0]=(char)('0'+(phase>4u?4u:phase));
        text(p,91,45,progress,18,5);
    }
    text(p,8,53,effect,18,27);
    box(p,7,61,50,10,selected?3:24);
    box(p,62,61,51,10,selected?24:3);
    box(p,8,62,48,1,selected?5:13);
    box(p,63,62,49,1,selected?13:5);
    text(p,13,64,left,selected?10:20,10);
    text(p,69,64,right,selected?20:10,10);
    text(p,7,74,"LEFT/RIGHT  A OK  B BACK",20,27);
}

/* Explicit on-screen alternatives: harm requires B near a location,
   RIGHT to select the bad action, then A to confirm. */
static void risk_menu(uint8_t *p,const Story *s){
    const char *name="",*act="";
    if(s->risk_menu=='H'){
        name="MARA / AT THE DOOR";act="OPEN / LEAVE";
    }else{
        name=s->risk_menu=='L'?"LETTER / A CHOICE":
             s->risk_menu=='C'?"BORE / A CHOICE":
             s->risk_menu=='M'?"MUSIC / A CHOICE":"TERMINAL / A CHOICE";
        act=s->risk_menu=='L'?"HIDE REPLY":s->risk_menu=='C'?
            "FORGE RECORD":s->risk_menu=='M'?"ERASE DEMO":"LEAK PRIVATE NAME";
    }
    panel(p,40,80);
    text(p,4,43,name,20,26);
    if(s->risk_menu!='H'){
        char count[]="BAD 0/4";
        count[4]=(char)('0'+story_bad_count(s));
        text(p,88,43,count,18,4);
    }
    if(s->risk_menu=='H'){
        text(p,4,53,"LEFT OPEN / RIGHT LEAVE",20,28);
    }else{
        text(p,4,52,act,20,18);
        text(p,4,60,s->rescue_stage==STORY_RESCUE_SAVED?
           "FOURTH CHECK = BAD END":
           s->rescue_stage>=STORY_RESCUE_MISSING?
           "FOURTH CHECK FAILS RESCUE":
           story_bad_count(s)==2?
           "NEXT CHECK RISKS MARA":
           "RIGHT = BAD CHECK +1",18,28);
    }
    text(p,4,69,s->risk_choice?
        (s->risk_menu=='H'?" > LEAVE   A CONFIRM":" > DO IT   A CONFIRM"):
        (s->risk_menu=='H'?" > OPEN    A CONFIRM":" > BACK    A CONFIRM"),20,28);
    text(p,94,74,"B EXIT",22,7);
}
/* Cinematic dialogue shares one clean speaker-first layout in every view,
   including good/neutral/bad ending and rescue. No new image assets. */
static void dialogue_card(uint8_t *p,const Story *s,const StoryPage *page,int count){
    int accent=s->cinema==CINE_RESCUE_DOOR||s->cinema==CINE_BAD?16:
       s->cinema==CINE_MARA||s->cinema==CINE_RESCUE_SAVED?28:
       s->cinema==CINE_MUSIC?25:18;
    if(s->cinema!=CINE_NONE){
        /* Cinematic subtitles live over SKY rather than hiding the
           grounded handoff, brush, door, feet and shared stage. */
        box(p,0,0,120,24,21);
        box(p,0,0,3,24,accent);
        box(p,3,23,117,1,accent);
        text(p,7,2,page->speaker,accent,21);
        for(int j=0;j<count&&j<5;j++)
            box(p,97+j*4,3,3,3,j<=(int)s->page?accent:5);
        text(p,7,9,page->line1,20,27);
        text(p,7,16,page->line2,20,27);
    }else{
        panel(p,48,80);
        box(p,1,49,2,30,accent);
        box(p,4,57,112,1,accent);
        text(p,7,51,page->speaker,accent,21);
        for(int j=0;j<count&&j<5;j++)
            box(p,97+j*4,52,3,3,j<=(int)s->page?accent:5);
        text(p,7,60,page->line1,20,26);
        text(p,7,68,page->line2,20,26);
        text(p,110,74,"A",20,1);
    }
}
static void ending_caption(uint8_t *p,unsigned ending,unsigned rescue,
                           unsigned f,unsigned harm,unsigned music_master){
 const char *a=0,*b=0;
 if(ending==STORY_END_GOOD){
   if(f<530){
     a=harm?"HE STILL OWED AN APOLOGY":"DANNY MADE ONE LAST PROMISE";
     b=harm?"HE DID NOT GET A CLEAN SLATE":"TO FINISH WHAT HE BEGAN";
   }else if(f<1080){
     a="HE LEFT SOME SONGS BEHIND";
     b=harm?"NOT EVERY RECORD WAS HONEST":"AND THE BORE DEMOS TOO";
   }
   else if(f<1560){a="MARA REMEMBERED HIS LAUGH";b="EVEN ON QUIET MORNINGS";}
   else if(f<2050){a="HIS NAME WAS FILED AWAY";
     b=harm?"HIS ACTIONS WERE FILED TOO":"BUT NOT HIS MUSIC";}
   else if(f<2600){a="AFTER DANNY WAS GONE";b="SOMEONE FOUND THE RECORDINGS";}
   else if(f<3200){a="THE SONGS FOUND LISTENERS";b="FAR BEYOND THE OLD QUARTER";}
   else if(f<3730){a="MILLIONS OF RECORDS SOLD";b="THE BORE GAMES FOUND FANS";}
   else if(f<4350){a="HE NEVER SAW THEIR SUCCESS";
     b=harm?"MARA TOLD THE WHOLE STORY":"BUT PEOPLE HEARD HIS VOICE";}
   else if(f<4890){a="ONE MORE SONG FOR DANNY";b="CLOSER TO THE END";}
   else if(f<5140){a="THE CITY KEPT PLAYING";b="THE MUSIC KEPT TRAVELING";}
   else{a="THE LAST DAYS OF DANNY STEEL";b="GOOD END / A LEGACY";}
 }else if(ending==STORY_END_BAD){
    if(rescue==STORY_RESCUE_AFTERSHOCK){
        if(f<780){a="MARA GOT HOME SAFELY";b="THE SECOND CHANCE WAS REAL";}
        else if(f<1650){a="ONE MORE BAD CHOICE";b="AND TRUST FELL APART";}
        else if(f<2490){a="THE CITY REMEMBERED";b="THE PROMISES HE BROKE";}
        else if(f<3670){a="DANNY LOST HIS WAY";b="BEFORE HIS LAST DAYS ENDED";}
        else{a="THE LAST DAYS OF DANNY STEEL";b="BAD END / MISUSED CHANCE";}
    }else{
        if(f<780){a="HE KEPT THE PHONE NEAR";b="SHE WAS SUPPOSED TO CALL";}
        else if(f<1650){a="A DAY PASSED WITH NO WORD";b="STILL NO VERIFIED RECOVERY";}
        else if(f<2490){a="THE DOOR STOOD CLOSED";b="HER FATE WAS UNKNOWN";}
        else if(f<3670){a="DANNY HAD NO ANSWERS";b="ONLY THE DAYS HE LOST";}
        else{a="THE LAST DAYS OF DANNY STEEL";b="BAD END / UNANSWERED";}
    }
 }else{
   if(f<370){a="ONE LAST LINE OF PAINT";b="HIS HAND REACHED THE CANVAS";}
   else if(f<810){a="HIS LAST WORK WAS QUIET";b="THE BRUSH STAYED THERE";}
   else if(f<1130){a="BEFORE / A LETTER FROM HOME";b="SOMEBODY HAD WAITED";}
   else if(f<1590){a="BEFORE / MARA AND DANNY";b="LAUGHED AT THE BAD JOKES";}
   else if(f<2010){a="BEFORE / THE BORE OFFICE";b="CERTIFIED THAT HE EXISTED";}
   else if(f<2490){a="BEFORE / THE MUSIC STUDIO";
     b=music_master?"A FINISHED MASTER TAPE":"AN UNFINISHED SONG";}
   else if(f<2870){a="MARA RETURNED TO THE ROOF";b="THE PAINTING REMAINED";}
   else if(f<3250){a="SHE FOUND A FOLDED NOTE";b="BESIDE THE PAINT SUPPLIES";}
   else if(f<3650){a="SOME WORDS WERE LEFT UNSAID";b="BUT THE LETTER REMAINED";}
   else if(f<4050){a="SHE SAW HIS COLORFUL CITY";b="JUST AS HE HAD PAINTED IT";}
   else{a="THE LAST DAYS OF DANNY STEEL";b="NEUTRAL END / THE CANVAS";}
 }
 box(p,0,61,120,19,21);
 box(p,0,60,120,1,22);
 text(p,4,63,a,20,28);
 text(p,4,71,b,18,28);
}
void story_draw(const Story *s,int frame,uint8_t *p) {
    if(s->battle){cult_arena(s,frame,p);return;}
    if(s->life_stage==STORY_LIFE_FINALE ||
       (s->ended && (s->ending==STORY_END_GOOD ||
                      s->ending==STORY_END_NEUTRAL ||
                      s->ending==STORY_END_BAD))){
        unsigned ending=s->ending==STORY_END_GOOD?STORY_END_GOOD:
                         s->ending==STORY_END_BAD?STORY_END_BAD:STORY_END_NEUTRAL;
        cinema_ending_draw_context(p,ending,s->finale_frame,
            s->rescue_stage==STORY_RESCUE_SAVED ||
            s->rescue_stage==STORY_RESCUE_AFTERSHOCK);
        if(s->dialogue){
            int count;const StoryPage *pages=story_pages(s,&count);
            if(pages && s->page<count){
                const StoryPage *d=&pages[s->page];
                dialogue_card(p,s,d,count);
            }
        }else if(s->finale_frame >= (ending==STORY_END_GOOD?
                STORY_GOOD_FINAL_FRAMES:STORY_END_SCORE_FRAMES)){
            box(p,0,72,120,8,21);
            text(p,3,74,"START / RETURN TO MENU",20,29);
        }else ending_caption(p,ending,s->rescue_stage,s->finale_frame,
                            (unsigned)story_bad_count(s),
                            s->song_steps>=STORY_PROJECT_DONE);
        return;
    }
    if(s->life_stage==STORY_LIFE_FINALE) {
        cinema_draw(p,CINE_FINALE,s->cinema_frame,s->page,s->finale_frame);
    } else if(s->cinema) {
        cinema_draw(p,s->cinema,s->cinema_frame,s->page,s->finale_frame);
        cinema_moment_detail(p,s->cinema,s->work,s->bond);
        cinema_project_detail(p,s->cinema,s->song_steps,s->art_steps,
                              s->project_style,s->day_frame/(STORY_DAY_FRAMES/3u));
    } else if(s->ended && s->ending==STORY_END_NEUTRAL) {
        scene_rooftop(s,frame,p);
    } else if(s->ended && s->ending==STORY_END_GOOD){
        scene_record(p,frame);
    } else if(s->scene==STORY_SCENE_CLINIC){
        scene_clinic(p,frame);
    } else if(s->scene==STORY_SCENE_NOTICE){
        scene_notice(p,frame);
    } else if(s->view)first_person(s,frame,p); else overhead(s,frame,p);
    box(p,0,0,120,8,21);
    /* Both camera modes use the identical next-location query. Keep
       the objective visible even when its actor is behind a wall. */
    int gx=s->x,gy=s->y; const char *goal="";
    int has_goal=story_goal(s,&gx,&gy,&goal);
    int ddx=gx-(int)s->x,ddy=gy-(int)s->y;
    const char *direction=ddx==0&&ddy==0?"HERE":
        abs_i(ddx)>abs_i(ddy)?(ddx>0?"E":"W"):(ddy>0?"S":"N");
    if(s->rescue_stage==STORY_RESCUE_LOST){
        char number[]="DAY 0/2";
        number[4]=(char)('0'+(s->day_number>2u?2u:s->day_number));
        text(p,3,2,number,20,9);
        box(p,47,3,36,2,3);
        box(p,47,3,(int)s->day_frame*36/STORY_DAY_FRAMES,2,18);
    } else if(s->life_stage && s->life_stage<STORY_LIFE_FINALE){
        char number[]={'D','A','Y',' ',(char)('0'+(s->day_number/10)%10),
                       (char)('0'+s->day_number%10),0};
        text(p,3,2,number,20,7);
        int progress=s->day_frame*48/STORY_DAY_FRAMES;
        box(p,30,3,20,2,3);box(p,30,3,progress*20/48,2,18);
        if(has_goal){text(p,54,2,goal,20,7);text(p,84,2,direction,18,1);}
    } else if(s->ended) {
        text(p,3,2,s->ending==STORY_END_GOOD?"GOOD END":
            s->ending==STORY_END_BAD?"BAD END":"NEUTRAL END",20,20);
    } else {
        if(has_goal){text(p,3,2,"NEXT",18,4);
            text(p,23,2,goal,20,12);text(p,75,2,direction,18,4);}
        else text(p,3,2,"DANNY / OLD QUARTER",20,29);
    }
    for(int i=0;i<5;i++){
        int x=90+i*6,done=!!(s->flags&(1<<i));
        box(p,x,2,5,5,done?12:3);
        if(done){put(p,x+1,4,20);put(p,x+2,5,20);put(p,x+3,3,20);}
        else box(p,x+1,3,3,3,5);
    }
    if(s->dialogue){
        int count; const StoryPage *pages=story_pages(s,&count);
        if(pages&&s->page<count){
            const StoryPage *d=&pages[s->page];
            dialogue_card(p,s,d,count);
        }
    } else if(s->risk_menu){
        risk_menu(p,s);
    } else if(s->activity){
        activity_menu(p,s);
    } else if(s->choosing){
        panel(p,49,80);text(p,4,53,"THE LAST BUS LEAVES.",20,28);
        text(p,4,62,s->choice?"GO    > STAY":" > GO    STAY",18,28);
        text(p,4,72,"LEFT/RIGHT   A CHOOSE",20,29);
    } else if(s->paused){
        if(s->choice&STORY_CHECK_PAGE_ACTIVE){
            consequence_board(p,s);
        }else{
            panel(p,38,80);
            box(p,4,41,112,1,18);
            text(p,6,44,"PAUSED / OLD QUARTER",20,27);
            text(p,6,54,"R  LIFE CONSEQUENCES",18,27);
            text(p,6,63,"START  RESUME GAME",20,27);
            text(p,6,72,"B  SAVE AND MENU",20,27);
        }
    } else {
        box(p,0,72,120,8,21);
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            text(p,3,74,"MARA MISSING   R CLUES",20,29);
        }else if(s->rescue_stage==STORY_RESCUE_LOST){
            text(p,3,74,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 "MARA NOT FOUND R NOTES":"MARA MAY CALL  R NOTES",20,29);
        }else if(s->rescue_stage==STORY_RESCUE_SAVED){
            char hint[]="MARA SAFE BAD 0/4 B RISK";
            hint[14]=(char)('0'+story_bad_count(s));
            text(p,3,74,hint,20,29);
        }else if(story_bad_count(s)>=2 && s->rescue_stage==STORY_RESCUE_NONE){
            char hint[]="WARNING  BAD 0/4 START R";
            hint[12]=(char)('0'+story_bad_count(s));
            text(p,3,74,hint,18,28);
        }else if(s->life_stage==STORY_LIFE_WAIT ||
           s->life_stage==STORY_LIFE_FOLLOWUP_WAIT){
            char hint[]="0/5 E0 S0 B0 M0 A0 W00";
            hint[0]=(char)('0'+memories_today(s));
            hint[5]=(char)('0'+s->energy);
            hint[8]=(char)('0'+s->spark);
            hint[11]=(char)('0'+s->bond);
            hint[14]=(char)('0'+s->song_steps);
            hint[17]=(char)('0'+s->art_steps);
            hint[20]=(char)('0'+s->work/10);
            hint[21]=(char)('0'+s->work%10);
            text(p,3,74,hint,20,29);
        } else text(p,3,74,s->ended?"START / RETURN TO MENU":
             s->life_stage==STORY_LIFE_FINALE?"THE LAST PAINTING":
             s->view?"L MAP  A TALK  B RISK  R NOTE":
                      "L 3D  A TALK  B RISK  R NOTE",20,29);
    }
}
