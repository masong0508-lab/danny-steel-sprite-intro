#include "hd_canvas.h"

/* Character art shared by all native-resolution story scenes.
   Roles: Danny 1, Mara 2, archivist 3, witness 4, doctor 5.
   Gestures: listening 0, speaking 1, walking 2, reaching 3,
   worried 7. Everything is drawn from a grounded skeleton:
   one x / foot anchor, no separately animated floating limbs. */
void hd_actor_stage(uint8_t *p,int x,int feet,int role,unsigned frame,int emotion){
 int walk=emotion==2;
 /* Four-phase planted-stride cycle (left / gather / right / gather),
    avoiding the former two-frame mirrored shuffle. Ankles and sleeve
    counter-swing share one phase while the floor contact never floats. */
 unsigned phase=(frame/8u)&3u;
 int stride=walk?(phase==0u?-4:phase==2u?4:0):0;
 int lift=walk?(phase&1u?1:0):0;
 int motion=walk?0:((int)(frame/38u&1u));
 int fy=feet+motion;
 unsigned coat=role==1?HD_RED:
               role==2?HD_GLASS:
               role==3?HD_STONE_LIT:
               role==4?HD_LEAF_DARK:HD_PLASTER_LIT;
 unsigned dark=role==1?HD_RED_SHADE:
               role==2?HD_GLASS:
               role==3?HD_ROAD_COOL:
               role==4?HD_LEAF_DARK:HD_STONE;
 unsigned light=role==1?HD_BRICK_LIT:
                role==2?HD_GLASS_LIT:
                role==3?HD_PAPER:
                role==4?HD_LEAF:HD_PAPER;
 unsigned hair=role==2?HD_HAIR:
               role==3?HD_STONE_DARK:
               role==4?HD_BARK:HD_INK;
 int hand_y=fy-28;
 /* Subtle pool of stage/lamplight, with anchored contact shadow. */
 hd_ellipse(p,x+1,feet+2,18,3,HD_SHADOW);
 hd_line(p,x-6,fy-12,x-6+stride,fy-3-lift,HD_INK);
 hd_line(p,x+6,fy-12,x+6-stride,fy-3-(walk?1-lift:0),HD_INK);
 hd_rect(p,x-10+stride,fy-4-lift,10,4,HD_INK);
 hd_rect(p,x+2-stride,fy-4-(walk?1-lift:0),10,4,HD_INK);
 if(walk){
  hd_line(p,x-7+stride,fy-3-lift,x-1+stride,fy-3-lift,HD_STONE);
  hd_line(p,x+3-stride,fy-3-(1-lift),
          x+9-stride,fy-3-(1-lift),HD_STONE);
 }
 hd_rect(p,x-8,fy-20,7,9,HD_BRICK_DARK);
 hd_rect(p,x+2,fy-20,7,9,HD_INK);
 hd_rect(p,x-7,fy-22,14,2,HD_COPPER);
 /* Asymmetric but mechanically connected coat with outlined silhouette. */
 hd_rect(p,x-13,fy-45,27,25,dark);
 hd_rect(p,x-11,fy-45,23,23,coat);
 hd_rect(p,x-10,fy-44,6,22,light);
 hd_rect(p,x+8,fy-43,4,22,dark);
 hd_rect(p,x-12,fy-23,25,4,dark);
 hd_rect(p,x-11,fy-23,23,2,light);
 hd_line(p,x-6,fy-44,x+1,fy-35,dark);
 hd_line(p,x+6,fy-44,x+1,fy-35,light);
 hd_line(p,x+1,fy-35,x+1,fy-23,dark);
 hd_pixel(p,x+2,fy-32,HD_GOLD);
 hd_pixel(p,x+2,fy-28,HD_GOLD);
 /* Fabric folds follow the moving shoulders without re-scaling faces.
    Sided lapels / cuffs prevent the torso reading as one flat slab. */
 hd_line(p,x-10,fy-42,x-7,fy-24,light);
 hd_line(p,x+10,fy-42,x+8,fy-25,dark);
 hd_rect(p,x-12,fy-32,3,1,HD_COPPER);
 hd_rect(p,x+10,fy-31,3,1,HD_SHADOW);
 /* Sleeves and hands. Poses rotate around shoulder; no detached palms. */
 int sx=x+12,sy=fy-42,ex=x+16,ey=hand_y;
 if(emotion==3){ex=x+23;ey=fy-49;}
 else if(emotion==7){ex=x+19;ey=fy-57;}
 else if(emotion==5){ex=x+21;ey=fy-39;}
 else if(emotion==2){ex=x+16-stride;ey=fy-31-lift;}
 else if(emotion==1){ex=x+19;ey=fy-37+(int)(frame/10u&1u)*2;}
 hd_line(p,sx,sy,ex,ey,dark);
 hd_line(p,sx-1,sy,ex-1,ey,coat);
 hd_rect(p,ex-2,ey-2,5,4,HD_SKIN);
 hd_pixel(p,ex+1,ey-1,HD_SKIN_LIT);
 int other_x=x-17+(walk?stride:0);
 int other_y=hand_y+(walk?lift*2:0);
 hd_line(p,x-12,fy-43,other_x,other_y,coat);
 hd_line(p,x-13,fy-42,other_x,other_y,dark);
 hd_rect(p,other_x-2,other_y-1,5,5,HD_SKIN);
 /* Neck and shoulders sit behind the head, restoring proper silhouette. */
 hd_rect(p,x-3,fy-52,7,9,HD_SKIN);
 hd_rect(p,x-5,fy-47,4,5,light);
 hd_rect(p,x+2,fy-47,4,5,dark);
 hd_rect(p,x-10,fy-64,20,16,HD_SKIN);
 hd_rect(p,x-9,fy-65,18,4,hair);
 hd_rect(p,x-10,fy-61,3,12,hair);
 hd_rect(p,x+8,fy-61,3,7,hair);
 /* Faces do not become identical heads: hairstyle, clothes, posture
    and subtle details convey each character at native LCD resolution. */
 if(role==1){
  hd_rect(p,x-11,fy-65,11,4,hair);
  hd_rect(p,x-8,fy-66,13,3,hair);
  hd_rect(p,x-5,fy-41,3,13,HD_RED_SHADE);
  hd_line(p,x-6,fy-42,x+7,fy-23,HD_COPPER); /* satchel */
  hd_rect(p,x+6,fy-25,7,7,HD_BARK);
  hd_rect(p,x+7,fy-24,5,2,HD_GOLD);
 }else if(role==2){
  hd_rect(p,x-12,fy-66,24,5,hair);
  hd_rect(p,x-12,fy-61,4,23,hair);
  hd_rect(p,x+9,fy-61,5,24,hair);
  hd_rect(p,x-2,fy-46,4,5,HD_GOLD);
  hd_line(p,x-9,fy-22,x+12,fy-22,HD_GLASS_LIT);
 }else if(role==3){
  hd_rect(p,x-10,fy-66,23,3,hair);
  hd_rect(p,x+7,fy-62,5,3,HD_STONE);
  hd_rect(p,x-3,fy-42,6,11,HD_PAPER);
  hd_rect(p,x-2,fy-41,4,3,HD_GOLD);
  hd_rect(p,x+6,fy-34,6,8,HD_PAPER);
  hd_rect(p,x+7,fy-32,4,1,HD_INK);
 }else if(role==4){
  hd_rect(p,x-12,fy-67,24,4,HD_LEAF_DARK);
  hd_rect(p,x-14,fy-64,5,24,HD_LEAF_DARK);
  hd_rect(p,x+10,fy-64,5,20,HD_LEAF_DARK);
  hd_rect(p,x-4,fy-44,1,18,HD_PLASTER);
 }else{
  hd_rect(p,x-12,fy-45,4,23,HD_PLASTER);
  hd_rect(p,x+10,fy-45,4,23,HD_PLASTER);
  hd_rect(p,x-4,fy-42,8,16,HD_PAPER);
  hd_rect(p,x-3,fy-39,6,7,HD_GLASS);
  hd_pixel(p,x+5,fy-33,HD_RED);
 }
 /* Eye animation and actual facial articulation. */
 if((frame%111u)<105u){
  hd_rect(p,x-6,fy-57,3,2,HD_INK);
  hd_rect(p,x+3,fy-57,3,2,HD_INK);
  hd_pixel(p,x-6,fy-58,HD_BARK);
  hd_pixel(p,x+3,fy-58,HD_BARK);
 }else{
  hd_rect(p,x-6,fy-56,3,1,HD_BARK);
  hd_rect(p,x+3,fy-56,3,1,HD_BARK);
 }
 hd_pixel(p,x,fy-53,HD_SKIN_LIT);
 if(emotion==7){
  hd_line(p,x-8,fy-61,x-3,fy-59,HD_INK);
  hd_line(p,x+3,fy-59,x+8,fy-61,HD_INK);
  hd_rect(p,x-3,fy-50,6,1,HD_BRICK_DARK);
 }else if(emotion==1&&(frame/11u&1u)){
  hd_rect(p,x-3,fy-51,6,3,HD_BRICK_DARK);
  hd_rect(p,x-2,fy-50,4,1,HD_SKIN_LIT);
 }else{
  hd_rect(p,x-3,fy-50,6,1,HD_BRICK_DARK);
  if(role==2)hd_pixel(p,x+4,fy-51,HD_BRICK_LIT);
 }
 if(emotion==5){
  /* Reassuring/reflective gestures stay near the chest, not in space. */
  hd_line(p,x+21,fy-39,x+9,fy-42,HD_SKIN);
 }else if(emotion==7){
  /* The reaching hand touches the hairline rather than floating. */
  hd_line(p,x+19,fy-57,x+6,fy-61,HD_SKIN);
 }
}

/* A selective 240x160 painted cinematic close-up. Only drawn in carefully
   authored reaction shots: no big static portraits, sprites, bitmap scale
   buffers or heap. The same 1-5 role identity and emotion as stage actors. */
void hd_actor_closeup(uint8_t *p,int x,int base,int role,
                      unsigned frame,int emotion){
 unsigned jacket=role==1?HD_RED:role==2?HD_GLASS:
                 role==3?HD_STONE_LIT:role==4?HD_LEAF_DARK:HD_PAPER;
 unsigned rim=role==1?HD_RED_SHADE:role==2?HD_GLASS:
              role==3?HD_STONE_DARK:HD_LEAF_DARK;
 unsigned hair=role==2?HD_HAIR:role==3?HD_STONE_DARK:
               role==4?HD_BARK:HD_INK;
 int breathe=(int)((frame/27u)&1u),y=base-breathe;
 /* Foreground torso runs behind the lower subtitle matte so every bust
    remains attached to the picture rather than floating above a panel. */
 /* A tapered shoulder-to-waist garment rather than three round blobs.
    Draw two scanlines per band instead of issuing four draw calls per
    *single* row; visual width quantization is at native pixel scale.
    This halves torso-loop command count without any image buffer. */
 hd_ellipse(p,x,y+4,36,6,HD_SHADOW);
 for(int d=0;d<86;d+=2){
  int yy=y-49+d;
  int middle=d+1;
  int half=middle<14?18+middle:
           middle<45?32+(middle-14)/8:35-(middle-45)/4;
  if(half<24)half=24;
  hd_rect(p,x-half-1,yy,half*2+2,2,HD_INK);
  hd_rect(p,x-half,yy,half*2,2,jacket);
  hd_rect(p,x-half,yy,4,2,rim);
  hd_rect(p,x+half-3,yy,3,2,rim);
  if(d%12==0)hd_rect(p,x-half+7,yy,9,1,HD_STONE_LIT);
 }
 /* Ten stepped fabric segments follow actual shoulder -> elbow -> wrist
    trajectories. Unlike two oval shoulder-pads, these have cuffs and
    remain anchored to the torso at every frame. Chunked four-pixel
    strips keep the handheld draw-call budget modest. */
 for(int k=0;k<11;k++){
  int yy=y-44+k*4;
  int shift=k/2;
  int wide=k<3?12+k*2:k<8?18:18-(k-8)*2;
  int lc=x-24-shift,rc=x+24+shift;
  if(emotion!=5){
   hd_rect(p,lc-wide/2-1,yy,wide+2,4,HD_INK);
   hd_rect(p,lc-wide/2,yy+1,wide,3,jacket);
   hd_rect(p,lc-wide/2+1,yy+1,3,3,rim);
  }
  if(emotion!=7){
   hd_rect(p,rc-wide/2-1,yy,wide+2,4,HD_INK);
   hd_rect(p,rc-wide/2,yy+1,wide,3,jacket);
   hd_rect(p,rc+wide/2-4,yy+1,3,3,rim);
  }
 }
 if(emotion!=5){
  hd_rect(p,x-36,y-4,15,3,HD_STONE_LIT);
  hd_rect(p,x-35,y-1,11,6,HD_SKIN);
  hd_rect(p,x-33,y,6,2,HD_SKIN_LIT);
 }
 if(emotion!=7){
  hd_rect(p,x+22,y-4,15,3,HD_STONE_LIT);
  hd_rect(p,x+24,y-1,11,6,HD_SKIN);
  hd_rect(p,x+25,y,6,2,HD_SKIN_LIT);
 }
 hd_rect(p,x-8,y-55,16,17,HD_SKIN);
 hd_rect(p,x-7,y-45,14,5,HD_SKIN_LIT);
 hd_line(p,x-21,y-41,x-4,y-20,HD_STONE_LIT);
 hd_line(p,x-4,y-20,x-9,y+6,rim);
 hd_line(p,x+21,y-42,x+4,y-20,rim);
 hd_line(p,x+4,y-20,x+9,y+9,HD_STONE_LIT);
 hd_rect(p,x-3,y-17,6,3,HD_GOLD);
 if(role==1){
  hd_line(p,x+25,y-43,x-10,y+18,HD_BARK);
  hd_rect(p,x+13,y+7,15,12,HD_BARK);
  hd_rect(p,x+14,y+8,13,2,HD_COPPER);
 }else if(role==2){
  hd_rect(p,x-14,y-39,28,4,HD_GLASS_LIT);
  hd_rect(p,x+10,y-26,5,5,HD_GOLD);
 }else if(role==3){
  hd_rect(p,x-4,y-34,8,24,HD_PAPER);
  hd_line(p,x,y-31,x-4,y-14,HD_COPPER);
 }
 if(role==5){
  hd_rect(p,x-4,y-35,8,16,HD_PAPER);
  hd_rect(p,x-3,y-32,6,8,HD_GLASS_LIT);
  hd_pixel(p,x+9,y-26,HD_RED);
 }
 /* Face has a deliberately asymmetric outline; hair, eyes, cheeks,
    ears and jaw are separately articulated at native pixel scale. */
 hd_ellipse(p,x,y-69,23,27,HD_INK);
 hd_ellipse(p,x,y-69,20,25,HD_SKIN);
 hd_ellipse(p,x-6,y-73,11,17,HD_SKIN_LIT);
 /* A slight jaw-plane shadow, lower-eyelid shape and cheek glint give
    expressions depth without blowing out the native skin palette. */
 hd_line(p,x-16,y-59,x-9,y-54,HD_BARK);
 hd_line(p,x+10,y-54,x+16,y-60,HD_BARK);
 hd_rect(p,x-16,y-66,4,2,HD_SKIN_LIT);
 hd_rect(p,x+12,y-65,4,2,HD_SKIN_LIT);
 hd_rect(p,x-22,y-72,5,12,HD_SKIN);
 hd_rect(p,x+18,y-71,5,12,HD_SKIN);
 hd_rect(p,x-19,y-66,4,3,HD_RED_SHADE);
 hd_rect(p,x+15,y-65,4,3,HD_RED_SHADE);
 if(role==2 || role==4){
  hd_ellipse(p,x,y-90,24,13,hair);
  hd_rect(p,x-24,y-85,7,26,hair);
  hd_rect(p,x+18,y-85,8,29,hair);
  hd_line(p,x-19,y-56,x-22,y-32,hair);
  hd_line(p,x+21,y-55,x+18,y-32,hair);
 }else if(role==3){
  hd_rect(p,x-20,y-94,39,11,hair);
  hd_rect(p,x+13,y-91,9,6,HD_STONE);
 }else{
  hd_ellipse(p,x-2,y-93,21,10,hair);
  hd_rect(p,x-20,y-89,8,14,hair);
  hd_line(p,x-18,y-89,x-5,y-94,hair);
  hd_rect(p,x+11,y-90,9,7,hair);
 }
 if(role==1){
  hd_line(p,x-15,y-91,x-7,y-95,HD_INK);
  hd_rect(p,x-7,y-50,4,8,HD_RED_SHADE);
 }
 int gaze=emotion==7?-2:emotion==5?2:0;
 int blink=((frame+role*17u)%127u)>=122u;
 if(blink){
  hd_line(p,x-15,y-72,x-4,y-72,HD_BARK);
  hd_line(p,x+4,y-72,x+15,y-72,HD_BARK);
 }else{
  hd_rect(p,x-15,y-75,11,7,HD_PAPER);
  hd_rect(p,x+4,y-75,11,7,HD_PAPER);
  hd_rect(p,x-12+gaze,y-74,5,6,HD_GLASS);
  hd_rect(p,x+7+gaze,y-74,5,6,HD_GLASS);
  hd_rect(p,x-11+gaze,y-73,3,5,HD_INK);
  hd_rect(p,x+8+gaze,y-73,3,5,HD_INK);
  hd_pixel(p,x-11+gaze,y-73,HD_PAPER);
  hd_pixel(p,x+8+gaze,y-73,HD_PAPER);
  hd_line(p,x-16,y-76,x-4,y-76,HD_INK);
  hd_line(p,x+4,y-76,x+16,y-76,HD_INK);
 }
 hd_line(p,x-2,y-69,x-4,y-61,HD_SKIN_LIT);
 hd_line(p,x-4,y-60,x+1,y-59,HD_BARK);
 if(emotion==7){
  hd_line(p,x-15,y-80,x-5,y-77,HD_BARK);
  hd_line(p,x+5,y-77,x+15,y-80,HD_BARK);
  hd_line(p,x-8,y-48,x+9,y-49,HD_BRICK_DARK);
 }else if(emotion==1 && ((frame/10u)&1u)){
  hd_rect(p,x-8,y-50,15,5,HD_BRICK_DARK);
  hd_rect(p,x-5,y-47,9,2,HD_SKIN_LIT);
 }else if(emotion==5){
  hd_line(p,x-8,y-48,x+8,y-46,HD_BRICK_DARK);
  hd_pixel(p,x+10,y-47,HD_SKIN_LIT);
 }else{
  hd_line(p,x-8,y-49,x+8,y-49,HD_BRICK_DARK);
 }
 /* Closeup gestures remain physically anchored to a shoulder. */
 if(emotion==5){
  hd_line(p,x-28,y-29,x-17,y-48,jacket);
  hd_line(p,x-17,y-48,x-7,y-53,HD_SKIN);
  hd_ellipse(p,x-7,y-53,5,4,HD_SKIN_LIT);
 }else if(emotion==7){
  hd_line(p,x+28,y-29,x+20,y-55,rim);
  hd_line(p,x+20,y-55,x+14,y-61,HD_SKIN);
  hd_ellipse(p,x+14,y-61,4,4,HD_SKIN_LIT);
 }
 if(role==5){ /* doctor's lapels and instrument remain recognizable */
  hd_line(p,x-19,y-39,x-4,y-20,HD_PAPER);
  hd_line(p,x+19,y-39,x+4,y-20,HD_PAPER);
  hd_ellipse(p,x+13,y-34,5,5,HD_INK);
  hd_ellipse(p,x+13,y-34,3,3,HD_GLASS_LIT);
 }
}
