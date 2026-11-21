#include "hd_canvas.h"
#include <string.h>
#define RGB(r,g,b) ((uint16_t)((r)|((g)<<5)|((b)<<10)))
/* 32 measured dusk / ink / skin / rain tones in the spare half of the
   mode-4 palette. Existing story colors 0..31 are untouched. */
const uint16_t hd_extra_palette[32]={
 RGB(7,9,13),RGB(10,13,17),RGB(12,16,20),RGB(6,8,11),
 RGB(14,17,20),RGB(17,18,19),RGB(23,22,21),RGB(11,12,13),
 RGB(13,9,9),RGB(20,13,12),RGB(26,17,14),RGB(21,18,16),
 RGB(25,22,19),RGB(4,5,8),RGB(30,23,12),RGB(15,21,20),
 RGB(22,26,24),RGB(9,15,19),RGB(24,15,10),RGB(7,13,11),
 RGB(10,18,13),RGB(16,22,16),RGB(15,11,8),RGB(30,28,22),
 RGB(4,5,7),RGB(20,21,20),RGB(26,10,10),RGB(17,8,10),
 RGB(27,20,15),RGB(30,24,17),RGB(20,14,9),RGB(29,23,13)
};
#undef RGB
/* A drawn screen has no relation to 120x80 scaled text: these are true
   5x7 letterforms with a one-pixel stroke and native GBA kerning. */
static const uint8_t alphabet[36][7]={
 {14,17,17,31,17,17,17}, /* A */
 {30,17,17,30,17,17,30}, /* B */
 {15,16,16,16,16,16,15}, /* C */
 {30,17,17,17,17,17,30}, /* D */
 {31,16,16,30,16,16,31}, /* E */
 {31,16,16,30,16,16,16}, /* F */
 {15,16,16,23,17,17,15}, /* G */
 {17,17,17,31,17,17,17}, /* H */
 {31,4,4,4,4,4,31}, /* I */
 {7,2,2,2,18,18,12}, /* J */
 {17,18,20,24,20,18,17}, /* K */
 {16,16,16,16,16,16,31}, /* L */
 {17,27,21,21,17,17,17}, /* M */
 {17,25,21,19,17,17,17}, /* N */
 {14,17,17,17,17,17,14}, /* O */
 {30,17,17,30,16,16,16}, /* P */
 {14,17,17,17,21,18,13}, /* Q */
 {30,17,17,30,20,18,17}, /* R */
 {15,16,16,14,1,1,30}, /* S */
 {31,4,4,4,4,4,4}, /* T */
 {17,17,17,17,17,17,14}, /* U */
 {17,17,17,17,17,10,4}, /* V */
 {17,17,17,21,21,27,17}, /* W */
 {17,17,10,4,10,17,17}, /* X */
 {17,17,10,4,4,4,4}, /* Y */
 {31,1,2,4,8,16,31}, /* Z */
 {14,17,19,21,25,17,14}, /* 0 */
 {4,12,4,4,4,4,14}, /* 1 */
 {14,17,1,2,4,8,31}, /* 2 */
 {30,1,1,14,1,1,30}, /* 3 */
 {2,6,10,18,31,2,2}, /* 4 */
 {31,16,16,30,1,1,30}, /* 5 */
 {15,16,16,30,17,17,14}, /* 6 */
 {31,1,2,4,8,8,8}, /* 7 */
 {14,17,17,14,17,17,14}, /* 8 */
 {14,17,17,15,1,1,30} /* 9 */
};
void hd_pixel(uint8_t *p,int x,int y,unsigned c){
 if((unsigned)x<HD_W&&(unsigned)y<HD_H)p[y*HD_W+x]=(uint8_t)c;
}
void hd_rect(uint8_t *p,int x,int y,int w,int h,unsigned c){
 int x2=x+w,y2=y+h;
 if(x<0)x=0;
 if(y<0)y=0;
 if(x2>HD_W)x2=HD_W;
 if(y2>HD_H)y2=HD_H;
 if(x>=x2||y>=y2)return;
 for(int yy=y;yy<y2;yy++)memset(p+yy*HD_W+x,(int)c,(unsigned)(x2-x));
}
static int abs_i(int n){return n<0?-n:n;}
void hd_line(uint8_t *p,int x,int y,int xx,int yy,unsigned c){
 int dx=abs_i(xx-x),dy=abs_i(yy-y),sx=x<xx?1:-1,sy=y<yy?1:-1,err=dx-dy;
 for(;;){
  hd_pixel(p,x,y,c);
  if(x==xx&&y==yy)break;
  int twice=2*err;
  if(twice>-dy){err-=dy;x+=sx;}
  if(twice<dx){err+=dx;y+=sy;}
 }
}
void hd_ellipse(uint8_t *p,int cx,int cy,int rx,int ry,unsigned c){
 if(rx<1||ry<1)return;
 for(int y=-ry;y<=ry;y++){
  /* integer ellipse, avoids float, sqrt and a new graphics dependency */
  int numerator=rx*rx*(ry*ry-y*y),width=0;
  while((width+1)*(width+1)*ry*ry<=numerator)width++;
  hd_rect(p,cx-width,cy+y,width*2+1,1,c);
 }
}
int hd_text_width(const char *s,int scale,int track){
 int n=0;for(;s[n];n++){}
 return n?(n*(5*scale+track)-track):0;
}
void hd_text(uint8_t *p,int x,int y,const char *s,unsigned ink,int scale,int track){
 if(scale<1)scale=1;
 if(scale>2)scale=2;
 for(int ch=0;s[ch]&&ch<55;ch++){
  char c=s[ch];if(c>='a'&&c<='z')c=(char)(c-32);
  unsigned n=c>='A'&&c<='Z'?(unsigned)(c-'A'):c>='0'&&c<='9'?(unsigned)(26+c-'0'):36u;
  if(n<36u){
   for(int j=0;j<7;j++)for(int i=0;i<5;i++)
    if(alphabet[n][j]&(16u>>i))hd_rect(p,x+i*scale,y+j*scale,scale,scale,ink);
  }else if(c=='.')hd_rect(p,x+2*scale,y+6*scale,scale,scale,ink);
  else if(c==':'){hd_rect(p,x+2*scale,y+2*scale,scale,scale,ink);hd_rect(p,x+2*scale,y+5*scale,scale,scale,ink);}
  else if(c=='/'){for(int j=0;j<7;j++)hd_rect(p,x+(4-j*2/3)*scale,y+j*scale,scale,scale,ink);}
  else if(c=='-')hd_rect(p,x,y+3*scale,5*scale,scale,ink);
  else if(c=='!'){hd_rect(p,x+2*scale,y,scale,4*scale,ink);hd_pixel(p,x+2*scale,y+6*scale,ink);}
  else if(c=='?'){
   hd_rect(p,x+scale,y,3*scale,scale,ink);hd_rect(p,x+4*scale,y+scale,scale,2*scale,ink);
   hd_rect(p,x+2*scale,y+3*scale,2*scale,scale,ink);hd_rect(p,x+2*scale,y+6*scale,scale,scale,ink);
  }
  x+=5*scale+track;
 }
}
void hd_copy_120_to_240(uint8_t *out,const uint8_t *in){
 for(int y=0;y<80;y++){
  uint8_t *a=out+y*2*HD_W,*b=a+HD_W;
  const uint8_t *src=in+y*120;
  for(int x=0;x<120;x++){
   uint8_t c=src[x];a[x*2]=a[x*2+1]=b[x*2]=b[x*2+1]=c;
  }
 }
}

/* Backward traversal makes this safe with source and destination using the
   same 38,400-byte EWRAM allocation. The source 9,600 pixels occupy its
   beginning; already-read pixels are overwritten, never pending pixels. */
void hd_upscale_inplace(uint8_t *p){
 for(int y=79;y>=0;y--)for(int x=119;x>=0;x--){
  uint8_t ink=p[y*120+x];
  int i=(y*2)*HD_W+x*2;
  p[i]=ink;p[i+1]=ink;p[i+HD_W]=ink;p[i+HD_W+1]=ink;
 }
}
