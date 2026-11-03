// Rejected By The Machines — GBC studio card previews (160x144, GBC-legal-ish renderer)
const W=160,H=144;
const COL={BG:'#0c1316',LN:'#152024',CR:'#f2ecdf',BE:'#d6cdb9',DK:'#080b0d',TL:'#36e0c8',TD:'#1d8a7c',RD:'#e04a3c',RK:'#7a241d',GR:'#7f9496'};
const KEYS=Object.keys(COL),IDX={};KEYS.forEach((k,i)=>IDX[k]=i);
function q5(hex){const n=parseInt(hex.slice(1),16);return [n>>16&255,n>>8&255,n&255].map(c=>Math.round(Math.round(c*31/255)*255/31));}
const RGB=KEYS.map(k=>q5(COL[k]));

const clamp=(v,a=0,b=1)=>Math.max(a,Math.min(b,v));
const seg=(f,a,b)=>clamp((f-a)/(b-a));
const lerp=(a,b,t)=>a+(b-a)*t;
const eOut=t=>1-Math.pow(1-t,3);
const eBack=t=>{const c=1.70158;return 1+(c+1)*Math.pow(t-1,3)+c*Math.pow(t-1,2);};
function bounce(t){const n=7.5625,d=2.75;if(t<1/d)return n*t*t;if(t<2/d)return n*(t-=1.5/d)*t+.75;if(t<2.5/d)return n*(t-=2.25/d)*t+.9375;return n*(t-=2.625/d)*t+.984375;}
function h(a,b=0,c=0){let n=(Math.imul(a|0,374761393)+Math.imul(b|0,668265263)+Math.imul(c|0,1442695041))|0;n=Math.imul(n^(n>>>13),1274126177);n^=n>>>16;return (n>>>0)/4294967296;}

class Screen{constructor(){this.bg=new Uint8Array(W*H);this.obj=new Uint8Array(W*H);this.scx=new Int16Array(H);this.reset();}
  reset(){this.bg.fill(IDX.BG);this.obj.fill(255);this.scx.fill(0);this.sx=0;this.sy=0;this.fade=0;this.swap=null;}}
function sw(S,m){S.swap=S.swap||{};for(const k in m)S.swap[IDX[k]]=IDX[m[k]];}
function P(L,x,y,c){x=Math.round(x);y=Math.round(y);if(x<0||y<0||x>=W||y>=H)return;L[y*W+x]=IDX[c];}
function R(L,x,y,w,hh,c){x=Math.round(x);y=Math.round(y);for(let j=0;j<hh;j++)for(let i=0;i<w;i++)P(L,x+i,y+j,c);}

// ---- big condensed font (7x11, scaled) ----
const BIG={
R:["######.","#######","##...##","##...##","##...##","######.","#####..","##.##..","##..##.","##..###","##...##"],
E:["#######","#######","##.....","##.....","######.","######.","##.....","##.....","##.....","#######","#######"],
J:[".....##",".....##",".....##",".....##",".....##",".....##",".....##","##...##","##...##","#######",".#####."],
C:[".#####.","#######","##...##","##.....","##.....","##.....","##.....","##.....","##...##","#######",".#####."],
T:["#######","#######","..###..","..###..","..###..","..###..","..###..","..###..","..###..","..###..","..###.."],
D:["#####..","######.","##..###","##...##","##...##","##...##","##...##","##...##","##..###","######.","#####.."],
M:["##...##","###.###","#######","##.#.##","##.#.##","##...##","##...##","##...##","##...##","##...##","##...##"],
A:["..###..",".#####.","##...##","##...##","##...##","#######","#######","##...##","##...##","##...##","##...##"],
H:["##...##","##...##","##...##","##...##","##...##","#######","#######","##...##","##...##","##...##","##...##"],
I:["..###..","..###..","..###..","..###..","..###..","..###..","..###..","..###..","..###..","..###..","..###.."],
N:["##...##","###..##","###..##","####.##","##.#.##","##.####","##..###","##..###","##...##","##...##","##...##"],
S:[".#####.","#######","##...##","##.....","######.",".######",".....##",".....##","##...##","#######",".#####."],
B:["######.","#######","##...##","##...##","######.","######.","##...##","##...##","##...##","#######","######."],
Y:["##...##","##...##","##...##","###.###",".#####.","..###..","..###..","..###..","..###..","..###..","..###.."]};
function gInfo(ch){if(ch===' ')return{w:3,l:0};const g=BIG[ch];let l=7,r=-1;g.forEach(row=>{for(let i=0;i<7;i++)if(row[i]==='#'){l=Math.min(l,i);r=Math.max(r,i);}});return{w:r-l+1,l};}
function bigLayout(str,sx,gap){let x=0;const chars=[];for(const ch of str){const gi=gInfo(ch);chars.push({ch,x,w:gi.w*sx,l:gi.l});x+=gi.w*sx+gap;}return{chars,w:x-gap};}
function drawGlyph(L,ch,x,y,c,sx,sy){const g=BIG[ch],{l,w}=gInfo(ch);for(let r=0;r<11;r++)for(let i=l;i<l+w;i++)if(g[r][i]==='#')R(L,x+(i-l)*sx,y+r*sy,sx,sy,c);}
function drawWord(L,lay,x,y,c,sx,sy,fn,shadow){lay.chars.forEach((ch,i)=>{if(ch.ch===' ')return;let dx=0,dy=0,col=c;if(fn){const r=fn(i,shadow);if(r.on===false)return;dx=r.dx||0;dy=r.dy||0;if(r.c&&!shadow)col=r.c;}drawGlyph(L,ch.ch,Math.round(x+ch.x+dx),Math.round(y+dy),col,sx,sy);});}
const REJ=bigLayout('REJECTED',2,2),MACH=bigLayout('MACHINES',2,2),BYT=bigLayout('BY THE',1,1);
const REJ1=bigLayout('REJECTED',1,1),BTM1=bigLayout('BY THE MACHINES',1,1);

// ---- small 3x5 font ----
const F3src={A:".#.#.####.##.#",B:"##.#.###.#.###.",C:".###..#..#...##",D:"##.#.##.##.###.",E:"####..##.#..###",F:"####..##.#..#..",G:".###..#.##.#.##",H:"#.##.####.##.#",I:"###.#..#..#.###",J:"..#..#..##.#.#.",K:"#.##.###.#.##.#",L:"#..#..#..#..###",M:"#.#######.##.#",N:"##.#.##.##.##.#",O:".#.#.##.##.#.#.",P:"##.#.###.#..#..",Q:".#.#.##.###..##",R:"##.#.###.#.##.#",S:".###...#...###.",T:"###.#..#..#..#.",U:"#.##.##.##.#.##",V:"#.##.##.#.#..#.",W:"#.##.#######.#",X:"#.##.#.#.#.##.#",Y:"#.##.#.#..#..#.",Z:"###..#.#.#..###",
"0":"####.##.##.####","1":".#.##..#..#.###","2":"##...#.#.#..###","3":"##...#.#...###.","4":"#.##.####..#..#","5":"####..##...###.","6":".###..####.####","7":"###..#.#..#..#.","8":"####.#####.####","9":"####.####..###.",
".":"............#.","/":"..#..#.#.#..#..","-":"......###......","+":"....#.###.#....",":":"....#.....#....",">":"#...#...#.#.#..","!":".#..#..#.....#.","?":"##...#.#.....#.","'":".#..#..........","(":".#.#..#..#...#.",")":".#...#..#..#.#.","%":"#.#..#.#.#..#.#"," ":"..............."};
const F3={};for(const k in F3src){let s=F3src[k];while(s.length<15)s+='.';F3[k]=s;}
// fix A,H,M,U,W which were written 14 long above by accident -> rebuild explicitly
F3.A=".#."+"#.#"+"###"+"#.#"+"#.#";F3.H="#.#"+"#.#"+"###"+"#.#"+"#.#";F3.M="#.#"+"###"+"###"+"#.#"+"#.#";F3.U="#.#"+"#.#"+"#.#"+"#.#"+".##";F3.W="#.#"+"#.#"+"###"+"###"+"#.#";F3.N="##."+"#.#"+"#.#"+"#.#"+"#.#";F3.K="#.#"+"#.#"+"##."+"#.#"+"#.#";F3.X="#.#"+"#.#"+".#."+"#.#"+"#.#";F3.Y="#.#"+"#.#"+".#."+".#."+".#.";F3.V="#.#"+"#.#"+"#.#"+".#."+".#.";F3.O=".#."+"#.#"+"#.#"+"#.#"+".#.";F3.D="##."+"#.#"+"#.#"+"#.#"+"##.";F3.R="##."+"#.#"+"##."+"#.#"+"#.#";F3.P="##."+"#.#"+"##."+"#.."+"#..";F3.Q=".#."+"#.#"+"#.#"+"##."+".##";F3.G=".##"+"#.."+"#.#"+"#.#"+".##";F3.B="##."+"#.#"+"##."+"#.#"+"##.";F3.C=".##"+"#.."+"#.."+"#.."+".##";F3.E="###"+"#.."+"##."+"#.."+"###";F3.F="###"+"#.."+"##."+"#.."+"#..";F3.I="###"+".#."+".#."+".#."+"###";F3.J="..#"+"..#"+"..#"+"#.#"+".#.";F3.L="#.."+"#.."+"#.."+"#.."+"###";F3.S=".##"+"#.."+".#."+"..#"+"##.";F3.T="###"+".#."+".#."+".#."+".#.";F3.Z="###"+"..#"+".#."+"#.."+"###";
F3["."]="..."+"..."+"..."+"..."+".#.";F3[":"]="..."+".#."+"..."+".#."+"...";F3[">"]="#.."+".#."+"..#"+".#."+"#..";F3["%"]="#.#"+"..#"+".#."+"#.."+"#.#";
function text3(L,s,x,y,c){for(let k=0;k<s.length;k++){const g=F3[s[k]]||F3[' '];for(let r=0;r<5;r++)for(let i=0;i<3;i++)if(g[r*3+i]==='#')P(L,x+k*4+i,y+r,c);}}
const t3w=s=>s.length*4-1;
function text3c(L,s,y,c,full){text3(L,s,Math.round((W-t3w(full||s))/2),y,c);}

// ---- shared pieces ----
function stripes(L){for(let y=0;y<H;y++)if(y%6<2)for(let x=0;x<W;x++)L[y*W+x]=IDX.LN;}
function corners(L){[[153,7]].forEach(([x,y])=>{R(L,x-3,y,7,1,'GR');R(L,x,y-3,1,7,'GR');});}
function emblem(S,cx,cy,o={}){
  const bg=S.bg,ob=S.obj;cx=Math.round(cx);cy=Math.round(cy);const a=23,b=20;
  const x0=Math.floor((cx-a-3)/8)*8,x1=Math.ceil((cx+a+3)/8)*8,y0=Math.floor((cy-b-2)/8)*8,y1=Math.ceil((cy+b+2)/8)*8;
  R(bg,x0,y0,x1-x0,y1-y0,'BG');
  if(o.ring){for(let k=0;k<40;k++){const an=k/40*Math.PI*2+(o.ringRot||0);const x=Math.round(cx+Math.cos(an)*34),y=Math.round(cy+Math.sin(an)*27);if(y<(o.ringClip??999)&&(x<x0||x>=x1||y<y0||y>=y1))P(bg,x,y,'TD');}}
  for(let y=-b;y<=b;y++)for(let x=-a;x<=a;x++)if(Math.abs(x)<=a-Math.abs(y)*(a/2)/b)P(bg,cx+x,cy+y,'BE');
  [[-(a-4),0],[a-4,0],[-10,-(b-4)],[10,-(b-4)],[-10,b-4],[10,b-4]].forEach(([bx,by])=>{R(bg,cx+bx-1,cy+by-1,3,3,'DK');P(bg,cx+bx,cy+by,'CR');});
  const lid=o.lid??1,R1=16,R2=13.5;
  const vis=(x,y)=>Math.abs(y)<=lid*R2*(1-(x/R2)**2);
  for(let y=-R1;y<=R1;y++)for(let x=-R1;x<=R1;x++){const d=Math.hypot(x,y);if(d>R1+.3)continue;let c='DK';if(d<=R2&&vis(x,y))c='CR';P(bg,cx+x,cy+y,c);}
  if(lid>0.05){const lx=Math.round((o.look||0)*5),ir=8,slit=o.slit??2;
    for(let y=-ir;y<=ir;y++)for(let x=-ir;x<=ir;x++){const d=Math.hypot(x,y);if(d>ir+.3)continue;const X=x+lx;if(Math.hypot(X,y)>R2-.5||!vis(X,y))continue;
      let c=d>ir-1.2?'DK':'TL';if(Math.abs(x)<=slit*Math.sqrt(Math.max(0,1-(y/ir)**2)))c='DK';P(ob,cx+X,cy+y,c);}
    if(lid>.6)R(ob,cx+lx-5,cy-5,2,2,'CR');}
  const sl=o.slash||0;
  for(const pass of [0,1])for(let i=0;i<3;i++){const t=clamp(sl-i);if(t<=0)continue;const off=(i-1)*9;
    const xa=cx-26,ya=cy+11+off,xb=cx+32,yb=cy-16+off,n=90;
    for(let k=0;k<=n*t;k++){const s=k/n,x=Math.round(xa+(xb-xa)*s),y=Math.round(ya+(yb-ya)*s),w=(s<.12||s>.88)?1:2;
      if(pass===0)P(ob,x,y+1,'RK');else R(ob,x,y-(w-1),1,w,'RD');}}
}
// wordmark: REJECTED at y, BY THE at y+24, MACHINES at y+36 (y must be ≡4 mod 8 for clean tiles)
function wordmark(S,y,o={}){
  const bg=S.bg;
  if(o.rej!==false)drawWord(bg,REJ,Math.round((W-REJ.w)/2+(o.rejX||0)),y,o.rejC||'CR',2,2,o.rejFn);
  const by=o.by??1,byY=y+24,bx=Math.round((W-BYT.w)/2);
  if(by>0){if(by>.3)drawWord(bg,BYT,bx,byY,'CR',1,1);const len=Math.round((bx-12)*by);R(bg,bx-5-len,byY+5,len,1,'CR');R(bg,bx+BYT.w+5,byY+5,len,1,'CR');}
  const my=y+36,mx=Math.round((W-MACH.w)/2+(o.machX||0)),sh=o.sh??2;
  if(o.mach!==false){if(sh>0)drawWord(bg,MACH,mx+sh,my+sh,o.shC||'RD',2,2,o.machFn,true);drawWord(bg,MACH,mx,my,o.machC||'TL',2,2,o.machFn,false);}
}
const TAG1='STORIES THE ALGORITHM COULD NOT APPROVE',TAG2='ANIMATION + COMICS + GAMES - EST. 2026';
function tags(S,y,p){const n=Math.floor(p*(TAG1.length+TAG2.length));text3c(S.bg,TAG1.slice(0,n),y+66,'GR',TAG1);if(n>TAG1.length)text3c(S.bg,TAG2.slice(0,n-TAG1.length),y+76,'TL',TAG2);}
const NOISE=[];for(let k=0;k<8;k++){const t=[];for(let i=0;i<64;i++){const r=h(k,i,77);t.push(r<.55?'BG':r<.85?'LN':'GR');}NOISE.push(t);}
function noiseTile(L,tx,ty,k){const t=NOISE[k];for(let i=0;i<64;i++)P(L,tx*8+(i&7),ty*8+(i>>3),t[i]);}

// ---- render to RGBA ----
function render(S,d){
  const fq=Math.round(S.fade*6)/6;
  for(let y=0;y<H;y++){const by=((y-S.sy)%H+H)%H,off=S.scx[y];
    for(let x=0;x<W;x++){const ox=x-S.sx,oy=y-S.sy;let c=255;if(ox>=0&&ox<W&&oy>=0&&oy<H)c=S.obj[oy*W+ox];
      if(c===255){const bx=(((x-S.sx+off)%W)+W)%W;c=S.bg[by*W+bx];}
      if(S.swap&&S.swap[c]!==undefined)c=S.swap[c];
      let [r,g,b]=RGB[c];if(fq<0){r*=1+fq;g*=1+fq;b*=1+fq;}else if(fq>0){r+=(255-r)*fq;g+=(255-g)*fq;b+=(255-b)*fq;}
      const p=(y*W+x)*4;d[p]=r;d[p+1]=g;d[p+2]=b;d[p+3]=255;}}
}
// ---- hardware check ----
function check(S){
  let bad=[],sets=new Map();
  for(let ty=0;ty<18;ty++)for(let tx=0;tx<20;tx++){const s=new Set();for(let j=0;j<8;j++)for(let i=0;i<8;i++)s.add(S.bg[(ty*8+j)*W+tx*8+i]);if(s.size>4)bad.push([tx,ty]);const key=[...s].sort().join(',');sets.set(key,s);}
  const arr=[...sets.values()].sort((a,b)=>b.size-a.size),pals=[];
  for(const s of arr){let ok=false;for(const p of pals){const u=new Set([...p,...s]);if(u.size<=4){s.forEach(c=>p.add(c));ok=true;break;}}if(!ok)pals.push(new Set(s));}
  let maxLine=0;for(let y=0;y<H;y++){const cells=new Set();for(let x=0;x<W;x++)if(S.obj[y*W+x]!==255)cells.add(x>>3);maxLine=Math.max(maxLine,cells.size);}
  return{bad,pals:pals.length,maxLine};
}
