// Deterministic source for the 42 native illustrated neighborhood panels.
// Node.js: node tools/build_neighborhood_panels.js [output-directory]
function makePanels(){
 const W=240,H=104;
 const P={road:32,stone:37,light:38,slate:39,brick:40,wall:41,rose:42,plaster:43,cream:44,shade:45,lamp:46,glass:47,sky:48,water:49,copper:50,green:51,leaf:52,lime:53,wood:54,paper:55,ink:56,chalk:57,red:58,darkred:59,skin:60,lit:61,hair:62,gold:63,wet:35,cool:34,blue:33,mortar:36};
 const {road,stone,light,slate,brick,wall,rose,plaster,cream,shade,lamp,glass,sky,water,copper,green,leaf,lime,wood,paper,ink,chalk,red,darkred,skin,lit,hair,gold,wet,cool,blue,mortar}=P;
 function panel(){
  const a=new Uint8Array(W*H);
  const px=(x,y,c)=>{if(x>=0&&x<W&&y>=0&&y<H)a[y*W+x]=c;};
  const rect=(x,y,w,h,c)=>{for(let yy=Math.max(0,y);yy<Math.min(H,y+h);yy++)a.fill(c,yy*W+Math.max(0,x),yy*W+Math.min(W,x+w));};
  const line=(x,y,x2,y2,c)=>{let dx=Math.abs(x2-x),dy=-Math.abs(y2-y),sx=x<x2?1:-1,sy=y<y2?1:-1,e=dx+dy;for(;;){px(x,y,c);if(x===x2&&y===y2)break;let q=2*e;if(q>=dy){e+=dy;x+=sx;}if(q<=dx){e+=dx;y+=sy;}}};
  const ellipse=(cx,cy,rx,ry,c)=>{for(let y=-ry;y<=ry;y++){let x=Math.floor(rx*Math.sqrt(Math.max(0,1-y*y/(ry*ry||1))));rect(cx-x,cy+y,x*2+1,1,c)}};
  const poly=(pts,c)=>{for(let y=Math.max(0,Math.min(...pts.map(p=>p[1])));y<=Math.min(H-1,Math.max(...pts.map(p=>p[1])));y++){let xs=[];for(let i=0;i<pts.length;i++){let [x1,y1]=pts[i],[x2,y2]=pts[(i+1)%pts.length];if((y1<=y&&y<y2)||(y2<=y&&y<y1))xs.push(x1+(y-y1)*(x2-x1)/(y2-y1));}xs.sort((a,b)=>a-b);for(let j=0;j+1<xs.length;j+=2){let x=Math.ceil(xs[j]);rect(x,y,Math.floor(xs[j+1])-x+1,1,c)}}};
  const frame=(x,y,w,h,c,inner)=>{rect(x,y,w,h,shade);rect(x+1,y+1,w-2,h-2,c);rect(x+3,y+3,w-6,h-6,inner);line(x+2,y+2,x+w-3,y+2,cream)};
  const floor=(y,material=road)=>{rect(0,y,W,H-y,material);line(0,y+1,239,y+1,slate);for(let i=0;i<4;i++){let z=y+3+i*i*3;line(0,z,239,z,cool)}for(let i=-4;i<=4;i++)line(120+i*13,y,120+i*34,103,slate)};
  const skyline=(day,seed)=>{rect(0,0,W,30,day?sky:cool);rect(0,30,W,29,day?glass:slate);rect(0,59,W,45,blue);ellipse(184,22,14,14,day?copper:slate);ellipse(184,22,10,10,day?lamp:blue);
   for(let i=0;i<8;i++){let x=i*34-10,h=12+(i*19+seed*11)%25;rect(x,60-h,32,h+12,day?slate:cool);rect(x+4,57-h,21,3,slate)}
   for(let i=0;i<9;i++){let x=i*29-8,h=14+(i*11+seed*7)%24;rect(x,72-h,28,h+7,i%3===0?brick:shade);line(x+1,72-h,x+26,72-h,slate);for(let j=0;j<3;j++)for(let z=0;z<2;z++){let xx=x+5+j*7,yy=77-h+z*9;if(yy<71){rect(xx,yy,3,4,glass);if((j+z+i+seed)%5===0)px(xx+1,yy+1,gold)}}}
  };
  const room=(family,seed)=>{rect(0,0,W,78,family);rect(0,0,W,5,shade);rect(0,5,W,3,wood);line(0,8,239,8,copper);for(let x=4;x<W;x+=48){rect(x,9,2,66,shade);line(x+2,9,x+2,74,slate)}rect(0,65,W,13,wood);line(0,65,239,65,copper);floor(78,wet);frame(16,13,56,46,wood,glass);rect(20,17,48,14,cool);for(let i=0;i<5;i++){let x=21+i*9,h=10+(i*7+seed)%16;rect(x,54-h,8,h,shade);if(i%2===0)px(x+4,58-h,gold)}rect(43,16,2,39,wood);line(20,43,68,43,plaster);line(19,58,68,58,cream);for(let i=0;i<4;i++)line(23+i*11,24+i*4,22+i*11,28+i*4,blue)};
  const tree=(x,y,s=1)=>{ellipse(x,y+2,12*s,3,shade);rect(x-2*s,y-31*s,4*s,32*s,wood);line(x-1*s,y-24*s,x-10*s,y-34*s,wood);line(x+1*s,y-26*s,x+9*s,y-36*s,wood);ellipse(x,y-35*s,13*s,12*s,green);ellipse(x-10*s,y-31*s,10*s,8*s,leaf);ellipse(x+9*s,y-35*s,10*s,9*s,leaf);ellipse(x-4*s,y-42*s,9*s,7*s,leaf);ellipse(x-7*s,y-37*s,5*s,3*s,lime);ellipse(x+7*s,y-41*s,5*s,3*s,lime)};
  const bench=(x,y,w)=>{ellipse(x+w/2|0,y+11,w/2|0,3,shade);rect(x,y-11,w,4,wood);rect(x,y-5,w,3,copper);rect(x-2,y,w+4,4,wood);line(x,y,x+w-1,y,copper);rect(x+2,y+4,3,8,ink);rect(x+w-5,y+4,3,8,ink)};
  const table=(x,y,w)=>{ellipse(x+w/2|0,y+13,w/2|0,3,shade);rect(x-2,y,w+4,4,copper);rect(x,y+4,w,4,wood);rect(x+3,y+8,4,18,wood);rect(x+w-7,y+8,4,18,wood)};
  const person=(x,y,role,pose=0,scale=1)=>{ // original compact graphic-novel silhouettes
   const coat=[red,glass,green,wall,slate,cream][role%6], dark=[darkred,cool,green,brick,ink,stone][role%6], hl=[rose,sky,lime,copper,light,paper][role%6], hairc=role===1?hair:ink;
   const s=scale,q=(n)=>Math.round(n*s),R=(xx,yy,w,h,c)=>rect(x+q(xx),y+q(yy),Math.max(1,q(w)),Math.max(1,q(h)),c),L=(x1,y1,x2,y2,c)=>line(x+q(x1),y+q(y1),x+q(x2),y+q(y2),c);
   ellipse(x,y+1,q(13),q(2),shade);R(-7,-20,6,16,slate);R(2,-20,6,16,cool);R(-8,-4,9,4,ink);R(1,-4,9,4,ink);R(-10,-43,20,24,coat);R(5,-43,5,24,dark);R(-10,-42,3,20,hl);R(-5,-24,10,5,dark);L(-5,-42,1,-34,hl);L(4,-42,1,-34,dark);L(0,-33,0,-21,dark);
   R(-4,-48,8,7,skin);R(-7,-60,14,15,skin);R(-7,-60,14,5,hairc);R(-9,-57,3,8,hairc);R(6,-56,2,6,dark);R(-6,-55,4,9,lit);R(-4,-53,2,1,ink);R(3,-53,2,1,ink);R(-1,-47,5,1,wood);
   if(role===0){
    // Reference-based brown crown, connected auburn beard, collar headphones.
    R(-5,-63,9,3,hair);R(-8,-60,15,4,hair);
    L(-5,-61,0,-62,wood);L(1,-62,5,-60,wall);
    R(-8,-57,3,9,hair);R(5,-57,3,8,hair);
    L(-6,-51,-5,-47,hair);L(6,-51,5,-47,hair);
    R(-6,-49,12,3,hair);R(-5,-46,10,3,wall);
    R(-3,-44,6,2,copper);R(-3,-50,7,1,hair);
    R(-2,-48,5,1,darkred);
    L(-7,-44,-6,-40,ink);L(7,-44,6,-40,ink);
    R(-7,-41,4,5,ink);R(4,-41,4,5,ink);
    R(-6,-40,2,2,slate);R(5,-40,2,2,slate);
    L(-7,-43,8,-21,wood);L(-6,-43,9,-21,copper);R(6,-22,7,7,wood);R(7,-21,5,1,gold)
   }
   if(role===1){R(-8,-58,3,17,hairc);R(6,-58,3,18,hairc);R(-4,-42,8,3,sky)}
   if(role===3||role===4){R(-6,-55,12,1,ink);R(-1,-42,3,9,wood)}
   let hx=pose===1?17:pose===2?12:14,hy=pose===1?-42:pose===2?-32:-25;
   L(9,-40,13,-34,dark);L(13,-34,hx,hy,coat);R(hx-2,hy-1,4,4,skin);
   L(-9,-41,-13,-31,dark);R(-15,-29,4,5,skin);
  };
  const window=(x,y,w,h,litOn=true)=>{frame(x,y,w,h,wood,glass);if(litOn)rect(x+4,y+4,w-8,h-8,brick);rect(x+w/2|0,y+3,2,h-6,wood);rect(x+3,y+h/2|0,w-6,2,wood);if(litOn){rect(x+7,y+7,5,6,gold);rect(x+w-12,y+7,4,4,copper)}line(x+3,y+3,x+w-4,y+3,plaster)};
  const facade=(x,y,w,h,material=brick)=>{rect(x-3,y-4,w+6,h+7,shade);rect(x,y,w,h,material);rect(x-2,y-4,w+4,4,wood);line(x,y-4,x+w,y-4,copper);for(let yy=y+7;yy<y+h;yy+=8){line(x+1,yy,x+w-2,yy,material===plaster?stone:wall);for(let xx=x+7+(yy%3)*3;xx<x+w-4;xx+=17)line(xx,yy,xx,yy+5,material===plaster?stone:wall)}};
  const awning=(x,y,w)=>{rect(x,y,w,3,wood);for(let i=0;i<w;i+=10){poly([[x+i,y+3],[x+i+8,y+3],[x+i+11,y+11],[x+i,y+11]],(i/10&1)?copper:darkred);rect(x+i,y+11,9,2,shade)}};
  const book=(x,y,w,h,c)=>{poly([[x,y+4],[x+w/2|0,y],[x+w,y+4],[x+w,y+h],[x+w/2|0,y+h-3],[x,y+h]],wood);poly([[x+2,y+4],[x+w/2|0,y+2],[x+w/2|0,y+h-5],[x+2,y+h-2]],c);poly([[x+w/2|0,y+2],[x+w-2,y+4],[x+w-2,y+h-2],[x+w/2|0,y+h-5]],paper);for(let j=0;j<4;j++){let yy=y+7+j*4;if(yy<y+h-4){line(x+5,yy,x+w/2-3|0,yy-1,stone);line(x+w/2+3|0,yy-1,x+w-5,yy,stone)}}};
  const note=(x,y,w,h,mark)=>{rect(x+3,y+3,w,h,shade);rect(x,y,w,h,paper);rect(x+1,y+1,w-2,2,cream);for(let j=0;j<5;j++){let yy=y+6+j*4;if(yy<y+h-3)line(x+4,yy,x+w-6-(j&1)*4,yy,stone)}if(mark)rect(x+w-9,y+h-8,5,4,mark)};
  const cup=(x,y)=>{ellipse(x,y+7,9,3,plaster);rect(x-5,y-5,11,10,paper);ellipse(x,y-5,5,2,wood);rect(x+6,y-3,4,5,cream);px(x+7,y-1,wood)};
  const phone=(x,y,w,h)=>{frame(x,y,w,h,wood,glass);rect(x+4,y+6,w-8,6,ink);rect(x+5,y+5,4,11,ink);rect(x+w-10,y+5,4,11,ink);rect(x+7,y+16,w-14,h-21,slate);for(let i=0;i<3;i++)for(let j=0;j<3;j++)rect(x+9+j*4,y+19+i*4,2,2,cream);line(x+8,y+12,x+5,y+h-5,ink)};
  const crate=(x,y,w,h,fill)=>{rect(x,y,w,h,wood);rect(x+2,y+2,w-4,h-4,fill);line(x+1,y+2,x+w-2,y+2,copper);for(let i=0;i<w;i+=5)line(x+i,y+4,x+i,y+h-3,wood)};
  const radio=(x,y,w,h)=>{frame(x,y,w,h,wood,slate);ellipse(x+11,y+h/2|0,8,8,ink);ellipse(x+11,y+h/2|0,6,6,cool);for(let i=0;i<4;i++)line(x+7,y+8+i*3,x+15,y+8+i*3,stone);rect(x+w-21,y+6,15,8,glass);line(x+w-20,y+9,x+w-8,y+9,gold);ellipse(x+w-12,y+h-9,4,4,ink);px(x+w-12,y+h-11,paper)};
  const canvas=(x,y,w,h,warm)=>{frame(x,y,w,h,wood,paper);rect(x+4,y+4,w-8,h-8,warm?copper:glass);ellipse(x+w-12,y+11,5,5,lamp);for(let j=0;j<5;j++){let xx=x+5+j*(w-10)/5|0,hh=6+(j*5)%10;rect(xx,y+h-5-hh,Math.ceil((w-10)/5),hh,slate);px(xx+2,y+h-9,gold)}line(x+4,y+h-6,x+w-5,y+h-6,leaf)};
  return {a,px,rect,line,ellipse,poly,frame,floor,skyline,room,tree,bench,table,person,window,facade,awning,book,note,cup,phone,crate,radio,canvas};
 }
 // Each episode gets an authored wide establishing shot and two close
 // views of different physical actions. Branches are not recolored copies.
 function wide(id){
  const q=panel(),{rect:r,line:l,ellipse:e,poly,frame,floor,skyline,room,tree,bench,table,person,window,facade,awning,book,note,cup,phone,crate,radio,canvas}=q;
  const interior=[0,1,3,4,7].includes(id);
  if(interior)room(id===0?slate:id===1?brick:id===3?wall:id===4?cool:slate,id);
  else {skyline(id===2||id===8||id===12,id);floor(77,id===12?cool:road)}
  switch(id){
   case 0:
    for(let i=0;i<3;i++){let x=85+i*42;frame(x,25,37,50,cream,slate);e(x+18,54,13,14,ink);e(x+18,54,10,11,glass);e(x+18,55,7,7,i===1?red:plaster);r(x+4,30,27,5,stone);r(x+8,31,5,2,gold);l(x+28,54,x+28,67,plaster)}
    l(1,12,84,18,wood);for(let i=0;i<3;i++){r(7+i*25,17,15,15,i===1?red:cream);r(10+i*25,15,9,3,wood)}crate(14,76,34,17,plaster);person(73,100,0,1,0.72);person(216,100,2,0,0.68);break;
   case 1:
    for(let i=0;i<4;i++){let x=82+i*35;frame(x,17,30,27,wood,paper);r(x+5,22,20,14,i%2?glass:brick);e(x+15,27,4,4,skin);r(x+10,31,10,5,slate)}
    table(75,68,137);frame(105,45,43,27,wood,slate);e(127,58,9,9,ink);e(127,58,6,6,glass);r(109,42,10,5,stone);note(159,58,32,12,0);person(40,100,0,1,0.72);person(204,98,3,1,.68);break;
   case 2:
    r(0,65,240,9,wood);l(0,64,239,64,cream);for(let i=0;i<9;i++)r(i*29+8,65,2,16,wood);
    for(let i=0;i<6;i++){let x=13+i*12,y=96-i*6;r(x,y,42,5,plaster);l(x,y,x+42,y,cream)}l(20,84,88,48,copper);l(88,48,118,48,copper);canvas(150,33,56,37,false);l(160,70,153,95,wood);l(196,70,204,95,wood);person(124,99,0,2,.76);break;
   case 3:
    table(75,65,148);radio(87,42,54,27);frame(164,14,55,31,wood,slate);
    for(let i=0;i<6;i++){let x=169+i*7;l(x,19,x,38,copper);r(x-1,22,3,3,light)}r(151,50,29,6,ink);r(153,51,5,4,red);r(184,59,28,6,wood);person(37,100,0,1,.72);person(202,100,4,1,.65);break;
   case 4:
    facade(79,16,148,62,brick);window(91,28,43,41,true);window(143,28,72,41,true);r(82,68,144,8,wood);for(let i=0;i<7;i++){r(150+i*8,37,6,13,plaster);l(151+i*8,39,155+i*8,42,stone)}r(45,58,20,29,darkred);r(44,54,22,4,red);r(49,63,12,3,ink);person(72,100,0,1,.72);person(184,97,3,1,.60);break;
   case 5:
    facade(100,14,128,64,wall);window(113,25,31,29,true);frame(164,22,42,56,wood,slate);r(170,29,30,18,glass);r(172,54,26,17,wood);pxDoor(q,196,59);for(let j=0;j<3;j++)r(151-j*4,77+j*4,66+j*8,4,plaster);tree(40,91,1);person(132,100,0,1,.77);break;
   case 6:
    for(let i=0;i<2;i++){let x=10+i*116;facade(x,16,106,62,i?plaster:brick);awning(x+2,30,102);table(x+7,66,91);for(let j=0;j<3;j++){crate(x+10+j*29,52,24,15,leaf);for(let k=0;k<5;k++)e(x+14+j*29+k*4,53+(k&1)*3,2,2,(j===1)?rose:gold)}}
    person(110,100,0,1,.74);person(203,99,2,1,.68);break;
   case 7:
    for(let j=0;j<2;j++){let x=84+j*70;frame(x,14,62,49,wood,shade);for(let i=0;i<7;i++){r(x+5+i*8,20,6,35,i%3===0?wall:plaster);r(x+6+i*8,24,4,2,copper);r(x+6+i*8,48,4,2,cream)}l(x+4,57,x+59,57,wood)}table(76,68,145);book(108,45,64,26,plaster);person(41,101,0,1,.75);person(205,100,3,0,.7);break;
   case 8:
    facade(95,13,140,65,plaster);frame(124,25,65,53,slate,glass);r(154,25,3,48,stone);r(192,18,28,20,paper);r(204,21,5,14,red);r(199,26,15,5,red);tree(30,94,1);bench(35,83,60);person(106,100,0,2,.70);person(205,98,5,0,.65);break;
   case 9:
    facade(3,17,232,60,brick);awning(11,24,88);r(18,35,76,36,green);window(24,39,30,26,true);window(59,39,29,26,true);frame(151,28,64,49,wood,shade);crate(156,65,26,14,wall);crate(189,59,30,20,plaster);r(111,33,19,32,gold);l(120,33,120,62,wood);person(99,100,0,1,.74);person(204,100,2,1,.63);break;
   case 10:
    r(0,70,240,34,green);for(let i=0;i<5;i++){let x=5+i*48;tree(x+14,88,1)}poly([[0,103],[60,78],[178,78],[239,103]],plaster);bench(72,85,83);person(169,101,0,0,.74);person(91,99,5,0,.65);break;
   case 11:
    facade(2,17,138,60,wall);window(18,31,37,35,false);window(67,31,39,35,true);r(161,12,58,70,brick);r(165,16,50,64,glass);r(169,20,43,8,wood);phone(179,32,28,41);l(162,82,219,82,plaster);person(145,101,0,1,.78);break;
   case 12:
    r(0,60,240,44,glass);for(let i=0;i<22;i++){let x=(i*47+id*9)%239,y=62+(i*13)%40;l(x,y,Math.min(239,x+9+(i%4)*5),y,i%3?water:sky)}poly([[0,76],[240,66],[240,103],[0,103]],plaster);l(0,77,239,66,stone);for(let i=0;i<9;i++){let x=i*29,y=75-Math.floor(x/24);r(x,y-15,2,17,wood)}l(0,60,239,50,copper);l(0,64,239,54,wood);person(119,100,0,2,.76);break;
   case 13:
    facade(0,19,82,56,brick);window(12,29,32,34,true);window(46,29,27,34,false);r(105,26,114,5,wood);poly([[104,26],[220,26],[232,33],[95,33]],plaster);r(106,34,3,46,wood);r(214,34,3,46,wood);r(111,38,100,38,glass);l(113,40,209,40,sky);bench(120,74,80);frame(91,30,15,26,wood,paper);note(94,34,9,18,0);person(103,100,0,0,.74);person(197,100,3,0,.65);break;
  }
  r(0,0,240,2,ink);l(0,102,239,102,ink);return q.a;
 }
 function pxDoor(q,x,y){q.rect(x,y,3,3,gold);q.rect(x-1,y+1,5,1,copper)}
 function result(id,branch){
  const q=panel(),{rect:r,line:l,ellipse:e,poly,frame,floor,skyline,room,tree,bench,table,person,window,facade,awning,book,note,cup,phone,crate,radio,canvas}=q;
  const outside=[2,5,6,8,9,10,11,12,13].includes(id);
  if(outside){skyline(id===2||id===8||id===12,id+4);floor(74,wet)}
  else room(id===0?slate:id===1?brick:id===3?wall:id===4?cool:slate,id+4);
  // Character reaction left or right; an explicit still-life composition
  // carries the object/action selected by the player.
  const x=branch===1?109:18;
  const actX=branch===1?51:185;
  table(x,77,105);
  switch(id){
   case 0:
    if(branch===1){poly([[x+8,43],[x+32,34],[x+54,41],[x+83,34],[x+96,47],[x+82,65],[x+65,57],[x+63,78],[x+29,78],[x+29,57],[x+15,66]],red);l(x+45,42,x+45,75,darkred);for(let i=0;i<6;i++)l(x+33+i*5,62,x+34+i*5,65,cream);e(x+79,71,8,5,wood);l(x+73,71,x+89,59,copper)}else{crate(x+8,51,83,25,plaster);r(x+14,44,68,9,cream);r(x+22,35,55,12,glass);r(x+28,27,43,11,paper);l(x+28,27,x+70,27,cream)}
    break;
   case 1:
    if(branch===1){frame(x+12,23,77,52,wood,paper);r(x+18,30,65,35,glass);e(x+50,43,10,10,skin);r(x+40,33,20,9,hair);r(x+36,52,29,11,red);l(x+25,70,x+52,70,stone);l(x+54,71,x+78,68,copper)}else{frame(x+9,20,82,57,wood,slate);for(let i=0;i<3;i++){let xx=x+15+i*25;r(xx,27,21,34,cream);r(xx+3,32,15,24,glass);e(xx+10,43,6,6,skin);r(xx+4,48,13,6,wall)}r(x+16,66,70,5,shade)}break;
   case 2:
    if(branch===1){canvas(x+10,21,85,55,true);l(x+20,77,x+13,98,wood);l(x+85,77,x+91,98,wood);l(x+75,70,x+94,48,copper)}else{r(x+7,23,91,51,glass);for(let i=0;i<7;i++){let xx=x+8+i*13,hh=10+(i*13)%21;r(xx,72-hh,12,hh,slate);pxDoor(q,xx+4,68-hh)}l(x+7,48,x+98,48,wood);r(x+7,73,91,4,plaster);e(x+82,35,8,8,lamp)}break;
   case 3:
    if(branch===1){radio(x+7,28,91,44);r(x+15,69,34,6,wood);l(x+50,62,x+77,38,copper);e(x+77,38,3,3,cream);r(x+84,32,5,3,gold)}else{crate(x+12,33,81,40,plaster);radio(x+21,38,59,29);note(x+53,65,34,11,red)}break;
   case 4:
    if(branch===1){note(x+15,23,74,48,red);l(x+78,58,x+101,27,wood);l(x+79,58,x+102,27,copper);r(x+7,66,36,8,slate)}else{frame(x+19,29,67,41,wood,paper);l(x+23,34,x+52,53,copper);l(x+81,34,x+52,53,copper);r(x+49,50,7,5,darkred);r(x+16,70,73,4,wood)}break;
   case 5:
    if(branch===1){frame(x+21,17,64,62,wood,slate);r(x+28,25,50,25,glass);r(x+28,54,50,19,wood);e(x+71,59,3,3,gold);r(x+61,35,14,24,skin);r(x+65,31,9,6,lit);l(x+65,31,x+77,36,skin)}else{frame(x+12,19,84,57,wood,slate);r(x+19,26,70,15,glass);note(x+30,49,46,21,red);r(x+48,46,7,3,copper)}break;
   case 6:
    if(branch===1){for(let j=0;j<2;j++){crate(x+8+j*49,48,43,27,leaf);for(let i=0;i<5;i++)e(x+13+j*49+i*7,49+(i%2)*5,4,4,j?rose:gold)}r(x+47,30,11,18,skin);l(x+52,37,x+66,50,lit)}else{poly([[x+19,48],[x+91,48],[x+85,76],[x+24,76]],wood);l(x+27,49,x+45,30,copper);l(x+81,49,x+63,30,copper);for(let i=0;i<8;i++)e(x+25+i*8,47+(i%2)*5,6,5,i%2?rose:gold);note(x+60,60,28,13,0)}break;
   case 7:
    if(branch===1){book(x+7,29,91,42,plaster);note(x+62,48,24,17,red);l(x+20,24,x+55,51,copper)}else{frame(x+12,23,81,49,wood,paper);for(let j=0;j<3;j++){r(x+20+j*22,30,18,35,slate);r(x+22+j*22,36,14,4,cream);r(x+25+j*22,53,9,4,gold)}note(x+57,64,29,9,0)}break;
   case 8:
    if(branch===1){note(x+13,25,75,48,red);r(x+21,32,12,11,cream);r(x+24,33,4,9,red);r(x+21,36,10,3,red);for(let j=0;j<3;j++)r(x+45,44+j*7,30,2,stone);l(x+82,48,x+101,28,copper)}else{cup(x+35,62);r(x+63,44,29,28,wood);r(x+64,41,27,5,plaster);l(x+77,43,x+77,27,leaf);e(x+70,31,7,5,leaf);e(x+83,27,8,5,lime);note(x+11,70,31,9,0)}break;
   case 9:
    if(branch===1){poly([[x+12,43],[x+29,31],[x+49,37],[x+73,31],[x+91,43],[x+75,57],[x+67,50],[x+67,74],[x+29,74],[x+29,50],[x+19,56]],gold);r(x+30,64,36,8,green);l(x+37,38,x+37,64,copper);l(x+59,38,x+59,64,copper)}else{note(x+16,33,73,42,red);r(x+22,40,61,8,plaster);for(let i=0;i<5;i++)e(x+28+i*12,61-(i&1)*3,5,3,gold);r(x+14,72,78,4,wood)}break;
   case 10:
    if(branch===1){r(x+9,41,87,33,wood);l(x+10,44,x+94,44,copper);poly([[x+19,61],[x+50,36],[x+77,61]],plaster);poly([[x+36,61],[x+63,39],[x+91,61]],cream);e(x+79,38,8,4,skin);r(x+73,42,16,7,skin)}else{cup(x+32,63);book(x+49,37,43,33,plaster);e(x+9,46,8,5,leaf);l(x+9,50,x+9,70,wood)}break;
   case 11:
    if(branch===1){phone(x+26,18,52,58);r(x+70,26,8,25,ink);r(x+66,22,15,7,ink);r(x+67,49,14,7,ink);l(x+72,56,x+83,72,ink);r(x+75,41,11,8,skin)}else{phone(x+10,23,42,51);note(x+56,34,38,33,red);l(x+86,55,x+100,36,copper)}break;
   case 12:
    if(branch===1){canvas(x+8,24,90,50,false);l(x+18,62,x+92,55,stone);for(let j=0;j<5;j++){let xx=x+18+j*14;l(xx,61,xx,51,wood)}l(x+73,74,x+100,46,copper)}else{r(x+8,30,89,46,glass);for(let j=0;j<8;j++)l(x+11+(j*17)%78,35+j*4,x+19+(j*17)%78,35+j*4,water);poly([[x+21,56],[x+43,56],[x+53,73],[x+10,73]],wood);l(x+45,29,x+63,63,wood);r(x+59,61,13,9,plaster);r(x+74,63,12,10,green)}break;
   case 13:
    if(branch===1){frame(x+10,20,86,57,wood,paper);for(let j=0;j<4;j++){r(x+16,30+j*9,72,1,stone);r(x+16,33+j*9,25,2,glass);r(x+70,33+j*9,15,2,slate)}r(x+14,57,77,6,copper);r(x+17,59,5,2,paper)}else{bench(x+7,68,91);r(x+61,28,26,32,wood);r(x+64,31,20,25,glass);r(x+65,54,18,3,paper);r(x+14,55,29,12,plaster);r(x+19,52,19,4,cream)}break;
  }
  person(actX,102,0,branch===1?1:2,1.07);
  // Secondary figure, where applicable, is placed in a distinct far plane,
  // not repeated across every choice or drawn over the key prop.
  if([0,1,3,4,6,7,9,10,13].includes(id)){
   let other=branch===1?12:228;person(other,69,id===7||id===4?3:id===8?5:2,0,.48);
  }
  r(0,0,240,2,ink);l(0,102,239,102,ink);return q.a;
 }
 return Array.from({length:14},(_,i)=>[wide(i),result(i,1),result(i,2)]);
}
if(typeof module!=="undefined"&&require.main===module){
 const fs=require("fs"),path=require("path"),out=process.argv[2]||"assets/neighborhood";
 fs.mkdirSync(out,{recursive:true});makePanels().forEach((p,i)=>fs.writeFileSync(path.join(out,String(i).padStart(2,"0")+".bin"),Buffer.concat(p.map(a=>Buffer.from(a)))));
}
