#include "story_draw.h"

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
static uint16_t floor_depth[80];
void story_draw_init(void) {
    for (int i=0;i<256;i++) {
        int h=i&127,p=h*(128-h);
        int v=65536*p/(81920-4*p);
        sin_q12[i]=(int16_t)(i>=128?-v:v);
    }
    /* Division belongs in setup, not in the 4,800 floor pixels per frame. */
    for (int y=40;y<80;y++) {
        int d=4400/(y-39);
        floor_depth[y]=(uint16_t)(d>2300?2300:d);
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
    if(t==','||t=='=')return px==0||py==0?9:10;
    if(t=='g')return (px==3&&py==5)||(((x+y)&3)==0&&px==6&&py==2)?13:11;
    return px==0||py==0?9:8;
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
    int coat=kind=='F'?16:kind=='C'?17:23;
    box(p,x-head/2,top,head,head,31);
    box(p,x-head/2,top,head,1,kind=='F'?24:3);
    box(p,x-w/2,top+head,w,scale/2,coat);
    box(p,x-w/2,top+head+scale/2,w/2,scale/4,3);
    box(p,x+1,top+head+scale/2,w/2,scale/4,3);
    if(scale>8){put(p,x-1,top+head/2,19);put(p,x+1,top+head/2,19);}
    if(kind=='C'&&scale>8)box(p,x+w/2,top+head+scale/4,3,2,18);
    if(kind=='F'&&(tick/10&1))put(p,x+head/2,top+head+1,31);
}
static void marker(uint8_t *p,int x,int y,int type,int size,int tick) {
    if(type=='L') {
        box(p,x-size/2,y-size/2,size,size*2/3,15);
        box(p,x-size/2+1,y-size/2+1,size-2,1,22);
        put(p,x,y-size/2+2,16);
    } else if(type=='B') {
        box(p,x,y-size,1,size,7);box(p,x-size/2,y-size, size, size/3,16);
        box(p,x-size/3,y-size+1,size*2/3,1,15);
    } else person(p,x,y,type,size,tick);
}
static void overhead(const Story *s,int tick,uint8_t *p) {
    int cx=s->x-7, cy=s->y-4;
    for(int ty=0;ty<11;ty++)for(int tx=0;tx<16;tx++){
        int wx=cx+tx,wy=cy+ty;
        int px=tx*8-1,py=ty*8-1;
        tile(p,px,py,wx,wy,tick);
        char c=story_tile(wx,wy);
        if(c=='L'||c=='C'||c=='F'||c=='B')marker(p,px+4,py+7,c,7,tick);
    }
    int dx=0,dy=0;
    if(s->anim){dx=(s->prev_x-(int)s->x)*s->anim*2;dy=(s->prev_y-(int)s->y)*s->anim*2;}
    int px=60+dx,py=36+dy;
    box(p,px-2,py-4,5,4,31);box(p,px-2,py-5,5,1,24);
    box(p,px-3,py,7,4,16);box(p,px-2,py+4,2,3,3);box(p,px+1,py+4,2,3,3);
    put(p,px+(s->facing==0?2:s->facing==2?-2:0),py-2,19);
}
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
        /* Perspective floor projection: near details move with Danny, not with the camera backdrop. */
        for(int y=40;y<80;y++){
            int dist=floor_depth[y];
            int tx=(camx+vx*dist/4096)>>8,ty=(camy+vy*dist/4096)>>8;
            char c=story_tile(tx,ty);
            int color=c=='~'?25:c==','||c=='='?10:c=='g'||c=='T'?11:c=='#'?4:8;
            int fx=(camx+vx*dist/4096)&255, fy=(camy+vy*dist/4096)&255;
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
                int correct=(dist*sin_q12[(off+64+256)&255])/4096;
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
        if(c!='C'&&c!='F'&&c!='B'&&c!='L')continue;
        int dx=x*256+128-camx,dy=y*256+128-camy;
        int forward=(dx*dirx+dy*diry)/4096;
        int side=(dy*dirx-dx*diry)/4096;
        if(forward<80||forward>2300||abs_i(side)>forward*2)continue;
        int sx=60+(side*63)/(forward+1);
        int height=8400/(forward+64);if(height>64)height=64;if(height<3)height=3;
        if(sx<0||sx>=120||forward>depth[sx])continue;
        marker(p,sx,39+height/2,c,height,tick);
    }
}
static void panel(uint8_t *p,int top,int bottom){
    box(p,0,top,120,bottom-top,21);box(p,0,top,120,1,22);
}
void story_draw(const Story *s,int frame,uint8_t *p) {
    if(s->view)first_person(s,frame,p); else overhead(s,frame,p);
    box(p,0,0,120,8,21);
    text(p,3,2,"DANNY / OLD QUARTER",20,29);
    int collected=!!(s->flags&1)+!!(s->flags&2)+!!(s->flags&4);
    for(int i=0;i<3;i++)box(p,100+i*6,2,4,4,i<collected?18:3);
    if(s->dialogue){
        int count; const StoryPage *pages=story_pages(s,&count);
        if(pages&&s->page<count){
            panel(p,49,80);
            const StoryPage *d=&pages[s->page];
            text(p,4,52,d->speaker,18,28);
            text(p,4,60,d->line1,20,28);
            text(p,4,68,d->line2,20,28);
            text(p,91,74,"A NEXT",22,7);
        }
    } else if(s->choosing){
        panel(p,49,80);text(p,4,53,"THE LAST BUS LEAVES.",20,28);
        text(p,4,62,s->choice?"GO    > STAY":" > GO    STAY",18,28);
        text(p,4,72,"LEFT/RIGHT   A CHOOSE",20,29);
    } else if(s->paused){
        panel(p,47,80);text(p,5,53,"PAUSED / OLD QUARTER",20,28);
        text(p,5,63,"START RESUME",18,27);
        text(p,5,72,"B SAVE AND MENU",20,27);
    } else {
        box(p,0,72,120,8,21);
        text(p,3,74,s->view?"L MAP   A TALK   R NOTES":"L 3D   A TALK   R NOTES",20,29);
    }
}
