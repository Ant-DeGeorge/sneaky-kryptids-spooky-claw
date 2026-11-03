// ---- variants ----
const VARIANTS=[];

// ===== SET 1: Animated reveal + jingle =====
VARIANTS.push({id:'A1',set:'reveal',name:'Assembly Line',len:270,
 desc:'The name rides a conveyor in, gets scanned, fails QC, and a press stamps BY THE MACHINES under it.',
 hw:'Conveyor and letters: BG redraw + SCX. Press: BG tile rows. Shake: SCY. Fades: palette rewrites.',
 draw(S,f){const bg=S.bg;S.fade=f<10?-(1-f/10):-seg(f,245,269);
  R(bg,79,0,2,10,'GR');R(bg,70,10,20,6,'GR');R(bg,72,16,16,2,'DK');
  let light='TD';if(f>=95&&f<128)light=(f>>2)%2?'TL':'TD';if(f>=128&&f<150)light=(f>>2)%2?'RD':'RK';if(f>=150)light='RK';
  R(bg,77,11,6,3,light);
  if(f>=95&&f<128)text3(bg,'SCANNING',94,11,'TL');
  if(f>=128&&f<150&&(f>>3)%2===0)text3(bg,'FAIL!',94,11,'RD');
  const t=seg(f,10,90)*4,k=Math.floor(t),pr=k>=4?1:(k+eOut(clamp((t-k)/.6)))/4;
  const off=Math.round(lerp(-160,0,pr));
  R(bg,0,48,160,1,'GR');R(bg,0,49,160,6,'DK');R(bg,0,55,160,1,'GR');
  for(let x=((off%8)+8)%8-8;x<160;x+=8)R(bg,x,51,3,2,'LN');
  const shk=(f>=128&&f<150)?((f>>1)%2?1:-1):0;
  drawWord(bg,REJ,Math.round((W-REJ.w)/2)+off+shk,26,'CR',2,2);
  if(f>=95&&f<128)R(bg,8,26+Math.round(seg(f,95,127)*21),144,1,'TL');
  if(f>=160){R(bg,12,64,136,2,'RD');R(bg,12,110,136,2,'RD');R(bg,12,64,2,48,'RD');R(bg,146,64,2,48,'RD');
    drawWord(bg,BYT,Math.round((W-BYT.w)/2),70,'RD',1,1);
    const mx=Math.round((W-MACH.w)/2);
    if(f<190)drawWord(bg,MACH,mx,84,'RD',2,2);else{const sh=f<193?1:2;drawWord(bg,MACH,mx+sh,84+sh,'RD',2,2);drawWord(bg,MACH,mx,84,'TL',2,2);}}
  let top=-70;if(f>=150&&f<156)top=lerp(-70,62,seg(f,150,156));else if(f>=156&&f<172)top=62;else if(f>=172&&f<186)top=lerp(62,-70,eOut(seg(f,172,186)));
  if(top>-70){R(bg,74,0,12,Math.max(0,top),'GR');R(bg,8,top,144,52,'RK');R(bg,8,top,144,2,'GR');R(bg,8,top+50,144,2,'GR');}
  S.sy=(f>=156&&f<164)?((f>>1)%2?2:-2):0;
  if(f>=195){const n=Math.floor(seg(f,195,235)*(TAG1.length+TAG2.length));text3c(bg,TAG1.slice(0,n),118,'GR',TAG1);if(n>TAG1.length)text3c(bg,TAG2.slice(0,n-TAG1.length),126,'TL',TAG2);}
 },
 sfx(A){for(let k=0;k<4;k++){const fr=10+k*20;A.nz(fr,.12,.18,300,120);A.sq(fr,36,.1,.12,.5);}
  for(let fr=95;fr<128;fr+=8)A.sq(fr,88,.04,.08,.125);
  A.sq(128,38,.35,.12,.5);A.sq(128,39,.35,.1,.25);A.sq(140,38,.2,.12,.5);A.sq(140,39,.2,.1,.25);
  A.nz(150,.1,.2,4000,800);A.nz(156,.35,.35,900,90);A.tr(156,40,.4,.35,-12);
  [64,67,71,76].forEach((m,i)=>A.sq(166+i*6,m,.14,.1,.25));
  A.sq(190,64,.9,.1,.5);A.sq(190,71,.9,.08,.25);A.sq(190,76,.9,.08,.25);A.tr(190,40,.9,.3);
 }});

VARIANTS.push({id:'A2',set:'reveal',name:'Letter Drop',len:230,
 desc:'Each letter of REJECTED drops and bounces with its own note; MACHINES pops up underneath and the chord resolves. The most kid-friendly of the set.',
 hw:'Per-letter drops: OBJ sprites during motion, baked into BG on landing. Sparkles: OBJ. Fades: palette.',
 draw(S,f){const bg=S.bg;S.fade=f<10?-(1-f/10):-seg(f,205,229);stripes(bg);corners(bg);
  if(f>=4){const s='A GAME FROM',n=Math.floor(seg(f,4,18)*s.length);text3c(bg,s.slice(0,n),40,'GR',s);}
  const sh=f<122?0:f<125?1:2;
  wordmark(S,52,{by:eOut(seg(f,64,78)),sh,
   rejFn:i=>{const st=12+6*i;if(f<st)return{on:false};return{dy:lerp(-80,0,bounce(seg(f,st,st+24)))};},
   machFn:i=>{const st=80+4*i;if(f<st)return{on:false};return{dy:lerp(40,0,eBack(seg(f,st,st+12)))};}});
  if(f>=122&&f<190){[[18,44],[140,44],[8,82],[152,82],[8,104],[152,104]].forEach(([x,y],i)=>{const ph=((f-122)+i*7)%24;if(ph<12){const r=ph<4||ph>8?1:2;R(S.obj,x-r,y,2*r+1,1,'CR');R(S.obj,x,y-r,1,2*r+1,'CR');}});}
  tags(S,52,seg(f,135,180));
 },
 sfx(A){[60,62,64,65,67,69,71,72].forEach((m,i)=>A.sq(12+6*i+9,m,.12,.1,.5));
  A.nz(64,.15,.08,6000,2000);
  [72,76,79,84,79,84,88,91].forEach((m,i)=>A.sq(84+4*i,m,.08,.07,.25));
  A.sq(122,72,1,.1,.5);A.sq(122,76,1,.08,.25);A.sq(122,79,1,.08,.25);A.tr(122,48,1,.3);A.sq(146,84,.4,.05,.125);
 }});

// ===== SET 3: Dark glitch / cyberpunk =====
VARIANTS.push({id:'G1',set:'glitch',name:'Boot Sequence',len:330,
 desc:'A machine boot terminal evaluates the studio and returns REJECTED, then the screen tears and the logo resolves out of the glitch.',
 hw:'Terminal: BG text. Tears: per-scanline SCX writes in the LYC/HBlank interrupt. Colour flicker: palette swaps.',
 draw(S,f){const bg=S.bg;S.fade=f<8?-(1-f/8):-seg(f,305,329);
  if(f<195){const L=[[5,'RBTM-OS V1.0   (C) 2026','GR'],[30,'> LOAD STORY.BIN ...... OK','TL'],[62,'> HUMAN HANDS DETECTED: 2','TL'],[94,'> ALGORITHM MATCH: 0%','TL'],[126,'> VERDICT:','TL']];
    let cur=null;L.forEach(([t,s,c],i)=>{if(f<t)return;const n=Math.min(s.length,f-t);text3(bg,s.slice(0,n),6,24+i*10,c);cur={x:6+n*4,y:24+i*10};});
    if(f>=142){if(!(f<172&&(f>>3)%2))text3(bg,'REJECTED',50,64,'RD');cur={x:84,y:64};}
    if(cur&&(f>>4)%2===0)R(bg,cur.x,cur.y,3,5,'TL');
    if(f>=172){const p=seg(f,172,195);for(let y=0;y<H;y++)S.scx[y]=Math.round((h(y>>2,f>>1)-.5)*200*p*p);if(f%4<2)sw(S,{TL:'RD',RD:'TL',GR:'CR'});}
  }else{stripes(bg);wordmark(S,52);tags(S,52,seg(f,228,268));
    const p=1-eOut(seg(f,195,228));for(let y=0;y<H;y++)S.scx[y]=Math.round((h(y>>1,f)-.5)*160*p);
    if(f<228&&f%5<2)sw(S,{TL:'RD',RD:'TL',CR:'TL'});
    if(f>=275&&f<278)for(let y=88;y<100;y++)S.scx[y]=6;}
 },
 sfx(A){const L=[[5,23],[30,26],[62,25],[94,21],[126,10]];L.forEach(([t,n])=>{for(let k=0;k<n;k+=2)A.sq(t+k,96,.015,.04,.125);});
  A.sq(30+24,84,.05,.06,.25);A.sq(30+26,88,.08,.06,.25);
  [142,150,158,166].forEach(fr=>{A.sq(fr,40,.12,.12,.5);A.sq(fr,41,.12,.1,.25);});
  A.nz(172,.4,.25,6000,300);
  A.sq(196,45,1.4,.1,.5);A.sq(196,57,1.4,.07,.25);A.sq(200,64,1.2,.06,.125);A.tr(196,33,1.4,.35);
  for(let fr=196;fr<228;fr+=6)A.nz(fr,.03,.12,3000,1500);
  A.nz(275,.05,.15,5000,2000);A.sq(275,93,.05,.05,.125);
 }});

VARIANTS.push({id:'G2',set:'glitch',name:'Signal Tear',len:260,
 desc:'Dead-channel static that a torn, colour-flickering signal locks into the logo. It ends with small random jitters so it never sits fully still.',
 hw:'Static: 8 noise tiles shuffled in the BG map. Wave/tear: per-line SCX. Lock flash and flicker: palette swaps.',
 draw(S,f){const bg=S.bg;S.fade=f<4?-1+f/4:-seg(f,235,259);
  if(f>=40){stripes(bg);wordmark(S,52);tags(S,52,seg(f,125,165));}
  const pn=1-seg(f,40,85);
  for(let ty=0;ty<18;ty++)for(let tx=0;tx<20;tx++)if(h(tx,ty,f>>1)<pn)noiseTile(bg,tx,ty,Math.floor(h(tx+99,ty,f)*8));
  if(f>=40){const A=1-eOut(seg(f,40,112));for(let y=0;y<H;y++)S.scx[y]=Math.round(((h(y,f>>1)-.5)*90+Math.sin(y/7+f/3)*12)*A);}
  if(f>=40&&f<114&&h(f,3)<.35)sw(S,{TL:'RD',RD:'TL'});
  if(f>=40&&f<114&&h(f,5)<.15)sw(S,{CR:'TL'});
  if(f===114||f===115)sw(S,{BG:'TD',LN:'TD'});
  if(f>=150&&(f%47)<2){const b=Math.floor(h(f,9)*110)+10;for(let y=b;y<b+4;y++)S.scx[y]=Math.round((h(y,f)-.5)*12);}
 },
 sfx(A){A.nz(0,1.4,.14,7000,2500);A.sq(0,33,1.9,.04,.5);
  for(let fr=40;fr<114;fr+=5)if(h(fr,3)<.5)A.sq(fr,60+Math.floor(h(fr,8)*30),.03,.05,.125);
  A.nz(114,.08,.25,3000,3000);
  A.sq(116,64,.9,.09,.25);A.sq(116,71,.9,.08,.25);A.sq(116,76,.9,.07,.5);A.tr(116,40,.9,.35);A.sq(150,88,.3,.04,.125,-24);
  [197,244].forEach(fr=>A.nz(fr,.04,.12,4000,4000));
 }});

// ===== Using the logo art =====
VARIANTS.push({id:'L1',set:'logo',name:'Logo Reveal',len:210,
 desc:'A direct port of your logo. The eye emblem bounces in, REJECTED and MACHINES slide in from opposite sides, the eye opens, and three claw slashes rip across it.',
 hw:'Eye iris + slashes: OBJ sprites (7 palettes free). Hex, wordmark, stripes: BG. Shake: SCY.',
 draw(S,f){const bg=S.bg;S.fade=f<16?-(1-f/16):-seg(f,186,209);stripes(bg);corners(bg);
  if(f>=10){const lab='STUDIO PRESENTATION // CEL 001-A',n=Math.floor(seg(f,10,50)*lab.length);R(bg,8,4,5,1,'TL');text3(bg,lab.slice(0,n),16,2,'TL');}
  if(f>=8){const cy=lerp(-30,34,bounce(seg(f,8,40)));let lid=seg(f,80,90);if(f>=155&&f<165)lid=Math.abs(f-160)/5;
    const look=f<120?0:f<130?-seg(f,120,126):f<142?lerp(-1,1,seg(f,130,136)):lerp(1,0,seg(f,142,148));
    emblem(S,80,cy,{lid,look,slash:f>=100?(f-100)/4:0,ring:f>=40,ringRot:f*.01,ringClip:56});}
  wordmark(S,60,{rej:f>=30,rejX:lerp(-150,0,eOut(seg(f,30,48))),mach:f>=44,machX:lerp(160,0,eOut(seg(f,44,62))),sh:f<62?0:f<65?1:2,by:eOut(seg(f,62,76))});
  tags(S,60,seg(f,112,150));
  S.sy=(f>=100&&f<114)?((f>>1)%2?1:-1):0;
 },
 sfx(A){A.sq(8,60,.1,.08,.25,12);A.nz(30,.18,.15,2500,6000);A.nz(44,.18,.15,2500,6000);
  A.sq(62,40,.14,.14,.5);A.nz(62,.08,.2,800,200);
  [57,60,64,69].forEach((m,i)=>A.sq(64+i*4,m,.1,.08,.25));
  A.sq(80,72,.18,.06,.125,12);
  [100,104,108].forEach((fr,i)=>{A.nz(fr,.09,.25,7000,1500);A.sq(fr,88-i*2,.08,.07,.5,-24);});
  A.sq(112,69,.8,.1,.5);A.sq(112,76,.8,.08,.25);A.sq(112,81,.8,.06,.25);A.tr(112,45,.8,.35);A.sq(130,88,.25,.05,.125);
 }});

VARIANTS.push({id:'L2',set:'logo',name:'The Eye Wakes',len:240,
 desc:'Your logo with the glitch treatment. In the dark only the eye emblem shows; it opens, scans the room and locks on the viewer, then claws tear the screen and the wordmark glitches in.',
 hw:'Look/blink: re-draw the iris sprites. Red flash and colour flicker: palette swaps. Tears: per-line SCX.',
 draw(S,f){const bg=S.bg;S.fade=f<30?-(1-f/30):-seg(f,215,239);
  if(f>=118){stripes(bg);corners(bg);}
  const my=lerp(72,34,eOut(seg(f,118,134)));
  const lid=eOut(seg(f,40,56)),look=f<60?0:f<76?-eOut(seg(f,60,70)):f<98?lerp(-1,1,eOut(seg(f,78,88))):lerp(1,0,seg(f,98,104));
  const slit=f<100?lerp(3,2,seg(f,56,70)):1;
  emblem(S,80,my,{lid,look,slit,slash:f>=102?(f-102)/4:0,ring:f>=134,ringRot:f*.01,ringClip:56});
  if(f===100||f===101)sw(S,{BG:'RK',LN:'RK',DK:'RK'});
  [102,106,110].forEach(ts=>{if(f>=ts&&f<ts+6)for(let y=Math.round(my)-22;y<my+22;y++)if(y>=0&&y<H)S.scx[y]+=Math.round((h(y,f)-.5)*16*(1-(f-ts)/6));});
  if(f>=130){const p=seg(f,130,172);wordmark(S,60);tags(S,60,seg(f,168,205));
    for(let y=58;y<124;y++)if(h(y>>1,7)>p*1.15)S.scx[y]=Math.round((h(y,f)-.5)*120*(1-p))+40;
    if(f<176&&f%6<2)sw(S,{TL:'RD',RD:'TL'});}
 },
 sfx(A){A.tr(0,33,1.7,.3);A.sq(40,50,.3,.05,.125,7);
  [[62,79],[80,83],[100,76]].forEach(([fr,m])=>A.sq(fr,m,.07,.07,.25));
  A.nz(100,.06,.25,2000,400);
  [102,106,110].forEach((fr,i)=>{A.nz(fr,.09,.25,7000,1200);A.sq(fr,86-i*3,.08,.07,.5,-24);});
  for(let fr=130;fr<172;fr+=6)A.nz(fr,.03,.1,3500,1500);
  A.sq(172,57,1.1,.1,.5);A.sq(172,60,1.1,.08,.25);A.sq(172,64,1.1,.07,.25);A.tr(172,45,1.1,.35);A.sq(190,76,.4,.04,.125,-12);
 }});

VARIANTS.push({id:'L3',set:'logo',name:'Quick Sting (2 s)',len:120,
 desc:'A short version for games where a long intro gets in the way: blink, slash, name, done.',
 hw:'Same sprites as Logo Reveal. Name uses the 1x font so it fits in 4 BG tile rows.',
 draw(S,f){const bg=S.bg;S.fade=f<6?-(1-f/6):-seg(f,100,119);stripes(bg);corners(bg);
  const lid=f<6?0:f<14?eOut(seg(f,6,14)):1;
  emblem(S,80,44,{lid,slash:f>=22?(f-22)/2.5:0});
  if(f===32)sw(S,{BG:'CR',LN:'CR'});
  if(f>=32){drawWord(bg,REJ1,Math.round((W-REJ1.w)/2),76,'CR',1,1,i=>({on:f>=32+i}));
    const bx=Math.round((W-BTM1.w)/2);const fn=i=>({on:f>=38+i});drawWord(bg,BTM1,bx+1,93,'RD',1,1,fn,true);drawWord(bg,BTM1,bx,92,'TL',1,1,fn);}
  S.sy=(f>=22&&f<32)?((f>>1)%2?1:-1):0;
 },
 sfx(A){A.sq(6,76,.08,.06,.125,12);[22,24,26].forEach((fr,i)=>{A.nz(fr,.07,.25,7000,1500);A.sq(fr,88-i*2,.06,.07,.5,-24);});
  A.sq(32,72,.15,.1,.25);A.sq(40,79,.8,.1,.5);A.sq(40,84,.8,.07,.25);A.tr(40,43,.8,.35);
 }});

if(typeof module!=='undefined')module.exports={VARIANTS};

// ===== COMBINED: G2 + L2 =====
VARIANTS.push({id:'X1',set:'combo',name:'Static Awakening (G2 + L2)',len:330,
 desc:'Dead-channel static clears to reveal the eye in the dark. It opens, scans, locks on you and claws through the signal. The wordmark then tears and flickers in and locks with a flash, and the screen keeps a small glitch twitch until it fades.',
 hw:'Static: 8 shuffled noise tiles. Wave/tears: per-line SCX. Iris + slashes: OBJ. Red flash, lock flash, flicker: palette swaps. Emblem lift: SCY band or tilemap redraw.',
 draw(S,f){const bg=S.bg;S.fade=f<4?-1+f/4:-seg(f,305,329);
  const lifted=f>=170;
  if(lifted){stripes(bg);corners(bg);}
  const my=lerp(72,34,eOut(seg(f,170,186)));
  const lid=eOut(seg(f,90,106)),look=f<112?0:f<126?-eOut(seg(f,112,120)):f<146?lerp(-1,1,eOut(seg(f,128,138))):lerp(1,0,seg(f,146,152));
  let lidB=lid;if(f>=282&&f<292)lidB=Math.abs(f-287)/5;
  const slit=f<152?lerp(3,2,seg(f,106,120)):1;
  if(f>=36)emblem(S,80,my,{lid:lidB,look,slit,slash:f>=154?(f-154)/4:0,ring:f>=186,ringRot:f*.01,ringClip:56});
  if(f>=180){wordmark(S,60);tags(S,60,seg(f,250,285));}
  // static: full screen early, then only over the wordmark area while it locks in
  const pnTop=1-seg(f,40,85),pnLow=f<180?pnTop:1-seg(f,180,215);
  for(let ty=0;ty<18;ty++){const p=(ty>=7&&f>=180)?pnLow:pnTop;if(p<=0)continue;for(let tx=0;tx<20;tx++)if(h(tx,ty,f>>1)<p)noiseTile(bg,tx,ty,Math.floor(h(tx+99,ty,f)*8));}
  // emblem resolving through the tear
  if(f>=36&&f<100){const A=1-eOut(seg(f,36,100));for(let y=0;y<H;y++)S.scx[y]=Math.round(((h(y,f>>1)-.5)*60+Math.sin(y/7+f/3)*10)*A);}
  if(f===150||f===151)sw(S,{BG:'RK',LN:'RK',DK:'RK'});
  [154,158,162].forEach(ts=>{if(f>=ts&&f<ts+6)for(let y=Math.round(my)-22;y<my+22;y++)if(y>=0&&y<H)S.scx[y]+=Math.round((h(y,f)-.5)*16*(1-(f-ts)/6));});
  // wordmark signal lock (G2)
  if(f>=180&&f<240){const A=1-eOut(seg(f,180,238));for(let y=58;y<H;y++)S.scx[y]=Math.round(((h(y,f>>1)-.5)*90+Math.sin(y/7+f/3)*12)*A);
    if(h(f,3)<.35)sw(S,{TL:'RD',RD:'TL'});if(h(f,5)<.15)sw(S,{CR:'TL'});}
  if(f===240||f===241)sw(S,{BG:'TD',LN:'TD'});
  if(f>=255&&(f%47)<2){const b=Math.floor(h(f,9)*110)+10;for(let y=b;y<b+4;y++)S.scx[y]=Math.round((h(y,f)-.5)*12);}
 },
 sfx(A){A.nz(0,1.5,.14,7000,2500);A.sq(0,33,2.6,.035,.5);A.tr(40,33,2.2,.22);
  A.sq(90,50,.3,.05,.125,7);
  [[114,79],[130,83],[148,76]].forEach(([fr,m])=>A.sq(fr,m,.07,.07,.25));
  A.nz(150,.06,.25,2000,400);
  [154,158,162].forEach((fr,i)=>{A.nz(fr,.09,.25,7000,1200);A.sq(fr,86-i*3,.08,.07,.5,-24);});
  A.nz(180,.6,.1,6000,2500);
  for(let fr=180;fr<240;fr+=5)if(h(fr,3)<.5)A.sq(fr,60+Math.floor(h(fr,8)*30),.03,.05,.125);
  A.nz(240,.08,.25,3000,3000);
  A.sq(242,57,1.1,.1,.5);A.sq(242,64,1.1,.08,.25);A.sq(242,69,1.1,.07,.25);A.tr(242,45,1.2,.35);A.sq(262,81,.4,.04,.125,-12);
  [255,282,302].forEach(fr=>A.nz(fr,.04,.12,4000,4000));A.sq(282,84,.05,.04,.125);
 }});
