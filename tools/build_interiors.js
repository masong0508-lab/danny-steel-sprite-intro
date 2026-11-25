// Reproducible room paintings and collision maps; node tools/build_interiors.js [out]
const configs=[
  [
    "LAUNDRY",
    "SORT",
    "WASH",
    "FOLD",
    43,
    34,
    50
  ],
  [
    "PHOTO",
    "FRAME",
    "FOCUS",
    "PRINT",
    40,
    39,
    44
  ],
  [
    "STAIR",
    "MEASURE",
    "SKETCH",
    "SHADE",
    37,
    34,
    63
  ],
  [
    "REPAIR",
    "CHECK",
    "TUNE",
    "TEST",
    39,
    32,
    50
  ],
  [
    "POST",
    "ADDRESS",
    "STAMP",
    "FILE",
    41,
    43,
    63
  ],
  [
    "STOOP",
    "FIND",
    "WRITE",
    "SHELVE",
    40,
    54,
    46
  ],
  [
    "MARKET",
    "SORT",
    "WEIGH",
    "PACK",
    51,
    37,
    53
  ],
  [
    "ANNEX",
    "LABEL",
    "SPLICE",
    "SLEEVE",
    40,
    39,
    50
  ],
  [
    "GARDEN",
    "SEED",
    "WATER",
    "PLAN",
    51,
    37,
    48
  ],
  [
    "SHIFT",
    "CLEAN",
    "FOLD",
    "SIGN OUT",
    51,
    39,
    52
  ],
  [
    "PARK",
    "REPAIR",
    "PAINT",
    "SHARE",
    54,
    37,
    53
  ],
  [
    "PHONE",
    "LIST",
    "DIAL",
    "MESSAGE",
    34,
    39,
    48
  ],
  [
    "CANAL",
    "SORT",
    "CLEAN",
    "SKETCH",
    39,
    34,
    50
  ],
  [
    "SHELTER",
    "CHECK",
    "MARK",
    "POST",
    43,
    37,
    63
  ]
];
function layouts(id){
 const a=[
 ["##..>...##","C........#","...##.....","..........","..#....#..","....D....."],
 ["#...>....#","..#...##.C","..........",".##.......",".....##...","....D....."],
 ["#...>...##","...C......","##.....#..","..........","..#..#...#","....D....."],
 ["##..>....#",".........C","...#..#...","..........",".##....#..","....D....."]];
 const b=[
 ["#1...#..2#","..........","..##......",".....#....","#.......3#","....<....."],
 ["#...1....#","..#....#..","2.........","....##....",".......3.#","....<....."],
 ["#..#....1#","..........","2....#....","..#.......",".....#3..#","....<....."],
 ["#...2....#",".......#..","..#.......","1......#..","....#...3#","....<....."]];
 // The doorway positions stay fixed. Additional side fixtures don't close corridors.
 const f=a[id%4].map(s=>s.split("")),w=b[(id*3+1)%4].map(s=>s.split(""));
 if(id>=4){let xx=[0,9,1][Math.floor(id/4)-1],yy=[3,2,4][Math.floor(id/4)-1];if(f[yy][xx]===".")f[yy][xx]="#";}
 return [f.map(s=>s.join("")),w.map(s=>s.join(""))];
}
function generate(configs,layouts){
 const W=240,H=140;
 function canvas(h){
  const a=new Uint8Array(W*h);a.fill(56);
  function rect(x,y,w,hh,c){let x0=Math.max(0,x),x1=Math.min(W,x+w);if(x1<=x0)return;for(let yy=Math.max(0,y);yy<Math.min(h,y+hh);yy++)a.fill(c,yy*W+x0,yy*W+x1)}
  function px(x,y,c){if(x>=0&&y>=0&&x<W&&y<h)a[y*W+x]=c}
  function line(x,y,X,Y,c){let dx=Math.abs(X-x),sx=x<X?1:-1,dy=-Math.abs(Y-y),sy=y<Y?1:-1,e=dx+dy;for(;;){px(x,y,c);if(x===X&&y===Y)break;let q=2*e;if(q>=dy){e+=dy;x+=sx}if(q<=dx){e+=dx;y+=sy}}}
  function ell(x,y,rx,ry,c){for(let j=-ry;j<=ry;j++){let k=Math.floor(rx*Math.sqrt(Math.max(0,1-j*j/(ry*ry||1))));rect(x-k,y+j,2*k+1,1,c)}}
  function box(x,y,w,hh,outer,inner){rect(x,y,w,hh,45);rect(x+1,y+1,w-2,hh-2,outer);rect(x+3,y+3,w-6,hh-6,inner);line(x+2,y+2,x+w-3,y+2,44)}
  function fixture(x,y,id,station,back,accent){
   rect(x+1,y+2,20,17,45);rect(x+2,y+1,18,15,54);rect(x+2,y+1,18,2,accent);rect(x+3,y+4,16,10,back);
   // Equipment belongs to the building's trade. Station variants have visible differences.
   switch(id){
    case 0: if(station%3===0){for(let i=0;i<3;i++)rect(x+5,y+5+i*3,12,2,[55,48,42][i]);}else if(station%3===1){box(x+4,y+3,14,12,43,39);ell(x+11,y+10,5,5,56);ell(x+11,y+10,3,3,47);px(x+15,y+5,46);}else{rect(x+4,y+7,14,6,55);line(x+4,y+7,x+17,y+7,44);rect(x+7,y+5,8,2,48)}break;
    case 1: if(station%3===0){box(x+5,y+3,12,12,54,47);rect(x+7,y+7,8,5,39);px(x+14,y+6,46);}else if(station%3===1){rect(x+4,y+6,12,8,39);ell(x+10,y+9,4,4,56);ell(x+10,y+9,2,2,48);rect(x+5,y+4,4,3,43);}else{rect(x+5,y+4,12,10,55);rect(x+7,y+6,8,5,47);line(x+7,y+10,x+14,y+10,39)}break;
    case 2: box(x+4,y+3,14,10,54,47);rect(x+6,y+8,10,3,39);px(x+14,y+5,46);line(x+6,y+12,x+4,y+16,54);line(x+16,y+12,x+18,y+16,54);if(station%3===0)line(x+4,y+12,x+17,y+4,63);if(station%3===2)rect(x+5,y+6,4,4,50);break;
    case 3: box(x+4,y+3,15,11,54,39);ell(x+8,y+9,3,3,56);rect(x+12,y+5,5,3,47);line(x+12,y+6,x+16,y+6,46);if(station%3===1)ell(x+15,y+11,2,2,44);else line(x+5,y+13,x+16,y+11,50);break;
    case 4: rect(x+5,y+4,13,10,55);line(x+6,y+7,x+15,y+7,37);line(x+6,y+9,x+14,y+9,37);rect(x+13,y+10,3,3,58);if(station%3===2)rect(x+3,y+3,6,12,54);break;
    case 5: rect(x+4,y+3,14,12,45);for(let k=0;k<4;k++){rect(x+5+k*3,y+4+(k&1),2,8,[41,47,52,43][k]);px(x+5+k*3,y+5,55)}line(x+4,y+13,x+17,y+13,50);if(station%3===1)rect(x+8,y+7,8,6,55);break;
    case 6: rect(x+4,y+8,14,7,54);for(let k=0;k<5;k++)ell(x+6+3*(k%4),y+7+(k&1),2,2,k&1?58:63);if(station%3===1){rect(x+8,y+3,7,3,43);px(x+11,y+4,56);}break;
    case 7: box(x+4,y+3,15,11,43,39);ell(x+8,y+8,3,3,56);ell(x+15,y+8,3,3,56);px(x+8,y+8,50);px(x+15,y+8,50);line(x+8,y+12,x+15,y+12,55);if(station%3===2)rect(x+12,y+4,5,9,55);break;
    case 8: for(let k=0;k<3;k++){let xx=x+5+k*5;rect(xx,y+10,4,5,41);line(xx+2,y+5,xx+2,y+10,52);ell(xx+1,y+6,2,2,53);ell(xx+3,y+4,2,2,52)}if(station%3===1)rect(x+3,y+9,5,5,47);break;
    case 9: rect(x+4,y+3,14,1,50);for(let k=0;k<3;k++){let xx=x+5+k*4;rect(xx,y+5,3,8,k===1?63:52);px(xx+1,y+4,43)}line(x+4,y+14,x+17,y+14,39);if(station%3===2)rect(x+9,y+8,8,5,55);break;
    case 10: rect(x+4,y+7,14,5,50);rect(x+6,y+12,2,3,56);rect(x+15,y+12,2,3,56);rect(x+8,y+4,7,3,55);if(station%3===1)line(x+4,y+6,x+17,y+6,52);break;
    case 11: box(x+5,y+3,13,12,39,47);rect(x+7,y+5,9,3,56);rect(x+7,y+4,2,5,56);rect(x+14,y+4,2,5,56);for(let j=0;j<2;j++)for(let k=0;k<3;k++)px(x+8+k*2,y+10+j*2,55);break;
    case 12: rect(x+5,y+8,7,7,47);line(x+6,y+7,x+10,y+7,43);line(x+14,y+3,x+11,y+13,54);rect(x+9,y+13,6,2,63);if(station%3===2)box(x+4,y+3,13,8,54,48);break;
    case 13: box(x+4,y+3,14,12,54,55);for(let k=0;k<3;k++)line(x+6,y+6+k*3,x+15,y+6+k*3,37);rect(x+12,y+12,3,1,58);if(station%3===1)line(x+6,y+12,x+15,y+5,50);break;
   }
  }
  return {a,rect,px,line,ell,box,fixture};
 }
 function room(id,which,layout){
  const c=configs[id],wall=c[4],floor=c[5],accent=c[6];
  const q=canvas(H),{a,rect:r,line:l,px,ell,box,fixture}=q;
  r(0,0,W,H,45);r(4,3,232,134,wall);r(6,21,228,115,54);r(10,22,220,114,floor);
  for(let y=22;y<136;y+=19)for(let x=10;x<230;x+=22){
    let col=(Math.floor((x-10)/22)+Math.floor((y-22)/19)+id+which)&1;
    r(x,y,22,19,col?floor:(floor===39?32:floor===54?40:floor===37?39:floor===34?33:floor));
    l(x,y,x+21,y,45);l(x,y,x,y+18,45);l(x+2,y+2,x+19,y+2,col?37:floor);
    if((x+y+id*7+which*5)%5===0){l(x+4,y+12,x+11,y+11,35);px(x+14,y+5,37)}
  }
  r(6,3,228,16,wall);r(6,3,228,2,45);r(6,18,228,2,accent);
  for(let k=0;k<4;k++){let x=16+k*57;box(x,5,42,11,54,k%2?47:39);l(x+5,8,x+36,8,37);if((k+id+which)%3===0)r(x+25,9,6,3,46)}
  r(6,20,3,116,45);r(231,20,3,116,45);l(8,21,8,136,accent);l(232,21,232,136,54);
  for(let y=26;y<135;y+=24){r(3,y,2,9,accent);r(235,y,2,9,54)}
  for(let y=0;y<6;y++)for(let x=0;x<10;x++){
   const t=layout[y][x],X=10+22*x,Y=22+19*y;
   if(t==="#")fixture(X,Y,id,(x+y+which*3+4)%7,wall,accent);
   else if(t==="C"){fixture(X,Y,id,0,wall,accent);r(X+5,Y+6,12,7,55);l(X+6,Y+8,X+15,Y+8,37);l(X+6,Y+10,X+12,Y+10,37)}
   else if(t>="1"&&t<="3"){fixture(X,Y,id,Number(t)-1,wall,accent);r(X+1,Y+1,6,7,56);}
   else if(t==="D"||t===">"||t==="<"){
    r(X+1,Y+1,20,17,45);r(X+3,Y+3,16,13,54);r(X+5,Y+5,12,9,39);l(X+3,Y+3,X+18,Y+3,accent);r(X+5,Y+14,12,2,accent);
    if(t===">"){l(X+7,Y+10,X+11,Y+6,46);l(X+11,Y+6,X+15,Y+10,46)}else{l(X+7,Y+7,X+11,Y+11,46);l(X+11,Y+11,X+15,Y+7,46)}
   }else if((x*3+y*7+id+which)%11===0)l(X+3,Y+15,X+17,Y+15,35);
  }
  r(3,137,234,3,45);return a;
 }
 function strip(id){
  const q=canvas(32),{a,rect:r,line:l,fixture}=q,c=configs[id];
  r(0,0,240,32,c[4]);r(0,0,240,2,45);r(0,29,240,3,45);
  for(let i=0;i<3;i++){const x=12+i*78;r(x,3,60,24,45);r(x+2,5,56,20,c[5]);fixture(x+20,6,id,i,c[4],c[6]);l(x+3,24,x+56,24,c[6]);if(i<2){l(x+63,15,x+73,15,c[6]);l(x+70,12,x+73,15,c[6]);l(x+70,18,x+73,15,c[6])}}
  return a;
 }
 return configs.map((c,i)=>{const map=layouts(i);return [room(i,0,map[0]),room(i,1,map[1]),strip(i)]});
}
const fs=require("fs"),path=require("path"),out=process.argv[2]||"assets/interiors";fs.mkdirSync(out,{recursive:true});
const art=generate(configs,layouts);art.forEach((p,i)=>fs.writeFileSync(path.join(out,String(i).padStart(2,"0")+".bin"),Buffer.concat(p.map(a=>Buffer.from(a)))));
const rooms=configs.map((_,i)=>layouts(i));fs.writeFileSync(path.join(out,"interior_layout.h"),"#pragma once\n/* Generated by tools/build_interiors.js; same map as art. */\nstatic const char interior_layouts[14][2][61]={\n"+rooms.map(m=>" {"+m.map(q=>JSON.stringify(q.join(""))).join(",")+"},").join("\n")+"\n};\n");
