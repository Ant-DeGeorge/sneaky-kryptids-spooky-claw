// Export X1 "Static Awakening" for GBDK-2020 from the JS reference renderer.
const fs=require('fs');
(function(){eval(fs.readFileSync(__dirname+'/core.js','utf8')+fs.readFileSync(__dirname+'/variants.js','utf8').replace("if(typeof module","if(false")+';globalThis.X_render=render;globalThis.X={Screen,emblem,stripes,corners,wordmark,tags,TAG1,TAG2,IDX,KEYS,COL,RGB,h,seg,eOut,lerp,clamp,W,H,VARIANTS};');})();
const {Screen,emblem,stripes,corners,wordmark,tags,TAG1,TAG2,IDX,KEYS,COL,h,seg,eOut,lerp,clamp,W,H}=globalThis.X;
const LEN=330, CX=80, CY=34; let SCXDATA;
const C=k=>IDX[k];

// ---------- per-frame timeline (mirrors X1.draw) ----------
const TL=[];
for(let f=0;f<LEN;f++){
  const my=lerp(72,34,eOut(seg(f,170,186)));
  const lid=eOut(seg(f,90,106)),look=f<112?0:f<126?-eOut(seg(f,112,120)):f<146?lerp(-1,1,eOut(seg(f,128,138))):lerp(1,0,seg(f,146,152));
  let lidB=lid;if(f>=282&&f<292)lidB=Math.abs(f-287)/5;
  const slit=f<152?lerp(3,2,seg(f,106,120)):1;
  const fade=f<4?-1+f/4:-seg(f,305,329);
  let flags=0;
  if(f===150||f===151)flags|=1;
  if(f>=180&&f<240&&h(f,3)<.35)flags|=2;
  if(f>=180&&f<240&&h(f,5)<.15)flags|=4;
  if(f===240||f===241)flags|=8;
  if(f>=170)flags|=16;
  let tearAmp=0;[154,158,162].forEach(ts=>{if(f>=ts&&f<ts+6)tearAmp=Math.round(8*(1-(f-ts)/6));});
  let jit=0;if(f>=255&&(f%47)<2)jit=Math.floor(h(f,9)*110)+10;
  TL.push({
    scy:(Math.round(34-my)+256)&255, myRound:Math.round(my),
    eye:{lid:Math.round(lidB*8)/8,lx:Math.round(look*5),slit:Math.round(slit*2)/2},
    ring:f>=186?Math.floor((((f*.01)%(Math.PI*2/40))/(Math.PI*2/40))*5)%5:255,
    slash:f>=154?Math.round(clamp((f-154)/4,0,3)*4):0,
    fade:Math.round(-fade*6), flags,
    pTop:Math.round(clamp(1-seg(f,40,85))*255), pLow:f<180?255:Math.round((1-seg(f,180,215))*255),
    waveA:(f>=36&&f<100)?Math.round((1-eOut(seg(f,36,100)))*255):0,
    wordA:(f>=180&&f<240)?Math.round((1-eOut(seg(f,180,238)))*255):0,
    tearAmp, tearC:Math.round(my), jit,
    tagChars:Math.floor(seg(f,250,285)*(TAG1.length+TAG2.length))
  });
}

// ---------- layers ----------
function layer(fn){const S=new Screen();S.reset();fn(S);return S;}
const eyeKey=e=>`${e.lid}|${e.lx}|${e.slit}`;
function eyeBG(e){ // emblem with iris composited into BG (no slashes, no ring)
  const S=layer(S=>emblem(S,CX,CY,{lid:e.lid,look:e.lx/5,slit:e.slit}));
  const out=new Uint8Array(S.bg);for(let i=0;i<out.length;i++)if(S.obj[i]!==255)out[i]=S.obj[i];return out;}
const stripesOnly=layer(S=>stripes(S.bg)).bg;
const ringLayer=ph=>layer(S=>{stripes(S.bg);emblem(S,CX,CY,{lid:1,ring:true,ringRot:ph*(Math.PI*2/40)/5,ringClip:56});}).bg;
const finalBG=(()=>{const S=layer(S=>{stripes(S.bg);corners(S.bg);emblem(S,CX,CY,{lid:1,ring:true,ringRot:0,ringClip:56});wordmark(S,60);tags(S,60,1);});
  const b=new Uint8Array(S.bg);for(let i=0;i<b.length;i++)if(S.obj[i]!==255)b[i]=S.obj[i];return b;})(); // obj = iris at neutral
const withoutTags=layer(S=>{stripes(S.bg);corners(S.bg);emblem(S,CX,CY,{lid:1,ring:true,ringRot:0,ringClip:56});wordmark(S,60);}).bg;
const withoutWord=layer(S=>{stripes(S.bg);corners(S.bg);emblem(S,CX,CY,{lid:1,ring:true,ringRot:0,ringClip:56});}).bg;
const withoutCorner=layer(S=>{stripes(S.bg);emblem(S,CX,CY,{lid:1,ring:true,ringRot:0,ringClip:56});}).bg;

const tilePx=(buf,tx,ty)=>{const a=[];for(let j=0;j<8;j++)for(let i=0;i<8;i++)a.push(buf[(ty*8+j)*W+tx*8+i]);return a;};
const same=(a,b)=>a.every((v,i)=>v===b[i]);

// eye states used
const eyeStates=[],eyeIdx={};
TL.forEach(t=>{const k=eyeKey(t.eye);if(!(k in eyeIdx)){eyeIdx[k]=eyeStates.length;eyeStates.push(t.eye);}t.eyeId=eyeIdx[k];});
const eyeBufs=eyeStates.map(eyeBG);
const ringBufs=[0,1,2,3,4].map(ringLayer);

// ---------- classify 20x18 positions ----------
const POS=[];
for(let ty=0;ty<18;ty++)for(let tx=0;tx<20;tx++){
  const fin=tilePx(finalBG,tx,ty);
  const eyeT=eyeBufs.map(b=>tilePx(b,tx,ty));
  const ringT=ringBufs.map(b=>tilePx(b,tx,ty));
  let kind='static',vis=0;
  if(!eyeT.every(t=>same(t,eyeT[0])))kind='eye';
  else if(!ringT.every(t=>same(t,ringT[0])))kind='ring';
  if(kind==='static'){
    const st=tilePx(stripesOnly,tx,ty);
    if(same(fin,st))kind='blank';
    else if(!same(fin,tilePx(withoutTags,tx,ty))){ // tag tile: reveal when its last char is typed
      const y=ty*8,line=(y<60+66+5&&y+8>60+66)?1:2;
      const s=line===1?TAG1:TAG2,x0=Math.round((W-(s.length*4-1))/2);
      const lastChar=Math.min(s.length-1,Math.floor((tx*8+7-x0)/4));
      const need=(line===1?0:TAG1.length)+lastChar+1;
      vis=TL.findIndex(t=>t.tagChars>=need);kind='tag';
    }else if(!same(fin,tilePx(withoutWord,tx,ty))){kind='word';vis=182;}
    else if(!same(fin,tilePx(withoutCorner,tx,ty))){kind='corner';vis=170;}
    else vis=0; // emblem static
  }
  if(kind==='ring')vis=186;
  POS.push({tx,ty,kind,vis,fin,eyeT,ringT});
}

// ---------- palettes ----------
const setOf=px=>new Set(px);
const NOISE_SET=new Set([C('BG'),C('LN'),C('GR')]);
const reqs=[];
POS.forEach(p=>{let s;
  if(p.kind==='eye'){s=new Set();p.eyeT.forEach(t=>t.forEach(c=>s.add(c)));}
  else if(p.kind==='ring'){s=new Set();p.ringT.forEach(t=>t.forEach(c=>s.add(c)));}
  else s=setOf(p.fin);
  if(s.size>4)throw new Error(`pos ${p.tx},${p.ty} ${p.kind} has ${s.size} colours`);
  p.set=s;reqs.push(s);});
reqs.push(new Set([C('BG'),C('LN')]));
const pals=[];
[...reqs].sort((a,b)=>b.size-a.size).forEach(s=>{for(const p of pals){const u=new Set([...p,...s]);if(u.size<=4){s.forEach(c=>p.add(c));return;}}pals.push(new Set(s));});
if(pals.length>7)throw new Error('too many palettes '+pals.length);
const PAL=pals.map(p=>[...p]);
// put BG colour first where present (index 0) for tidiness
PAL.forEach(p=>{const i=p.indexOf(C('BG'));if(i>0){p.splice(i,1);p.unshift(C('BG'));}while(p.length<4)p.push(p[0]);});
const NOISE_PAL=PAL.length;PAL.push([C('BG'),C('LN'),C('GR'),C('BG')]);
while(PAL.length<8)PAL.push([C('BG'),C('BG'),C('BG'),C('BG')]);
const palFor=s=>{const i=PAL.findIndex((p,k)=>k!==NOISE_PAL&&[...s].every(c=>p.includes(c)));if(i<0)throw new Error('no pal');return i;};
const BLANK_PAL=palFor(new Set([C('BG'),C('LN')]));

// ---------- tile encoding ----------
const enc=(px,pal)=>{const out=[];for(let j=0;j<8;j++){let lo=0,hi=0;for(let i=0;i<8;i++){const v=pal.indexOf(px[j*8+i]);if(v<0)throw new Error('colour not in pal');lo|=(v&1)<<(7-i);hi|=((v>>1)&1)<<(7-i);}out.push(lo,hi);}return out;};
const banks=[[],[]],tmap=[new Map(),new Map()];
function addTile(bank,bytes){const k=bytes.join(',');if(tmap[bank].has(k))return tmap[bank].get(k);const i=banks[bank].length;banks[bank].push(bytes);tmap[bank].set(k,i);return i;}
// bank0: 3 stripe tiles, 8 noise slots, then static
const STRIPE=[0,1,2].map(r=>addTile(0,enc(tilePx(stripesOnly,0,r),PAL[BLANK_PAL])));
const NOISE0=banks[0].length;for(let k=0;k<8;k++)banks[0].push(new Array(16).fill(k)); // placeholders (unique)
POS.forEach(p=>{p.pal=palFor(p.set);
  if(p.kind==='eye')p.tiles=p.eyeT.map(t=>addTile(1,enc(t,PAL[p.pal])));
  else if(p.kind==='ring')p.tiles=p.ringT.map(t=>addTile(1,enc(t,PAL[p.pal])));
  else p.tile=addTile(0,enc(p.fin,PAL[p.pal]));});
// noise pool (16 tiles, noise palette)
const NP=PAL[NOISE_PAL],noisePool=[];for(let k=0;k<16;k++){const px=[];for(let i=0;i<64;i++){const r=h(k,i,77);px.push(r<.55?C('BG'):r<.85?C('LN'):C('GR'));}noisePool.push(...enc(px,NP));}

// ---------- slash sprites (8x16, VRAM bank0) ----------
const slashStates=[];for(let s=0;s<=12;s++)slashStates.push(layer(S=>emblem(S,CX,CY,{lid:0,slash:s/4})).obj);
const fin=slashStates[12];let minx=W,miny=H;for(let i=0;i<W*H;i++)if(fin[i]!==255){minx=Math.min(minx,i%W);miny=Math.min(miny,Math.floor(i/W));}
const OPAL=[255,C('RD'),C('RK')];
const cells=[];for(let y=miny;y<H;y+=16)for(let x=minx;x<W;x+=8){let any=false;for(let j=0;j<16;j++)for(let i=0;i<8;i++){const X=x+i,Y=y+j;if(X<W&&Y<H&&fin[Y*W+X]!==255)any=true;}if(any)cells.push({x,y});}
// align sprite tile pairs: tile index must be even in 8x16 mode
const spr=[],sprMap=new Map();
const sprTile=bytes=>{const k=bytes.join(',');if(sprMap.has(k))return sprMap.get(k);while(banks[0].length%2)banks[0].push(new Array(16).fill(0));const i=banks[0].length;banks[0].push(bytes.slice(0,16),bytes.slice(16));sprMap.set(k,i);return i;};
const encObj=(buf,x,y)=>{const out=[];let any=false;for(let j=0;j<16;j++){let lo=0,hi=0;for(let i=0;i<8;i++){const X=x+i,Y=y+j;const c=(X<W&&Y<H)?buf[Y*W+X]:255;const v=c===255?0:OPAL.indexOf(c);if(v<0)throw new Error('obj colour');if(v)any=true;lo|=(v&1)<<(7-i);hi|=((v>>1)&1)<<(7-i);}out.push(lo,hi);}return any?out:null;};
const slashTiles=slashStates.map(buf=>cells.map(c=>{const b=encObj(buf,c.x,c.y);return b?sprTile(b):255;}));
// scanline sprite check
let maxPer=0;for(let y=0;y<H;y++){let n=0;cells.forEach(c=>{if(y>=c.y&&y<c.y+16)n++;});maxPer=Math.max(maxPer,n);}

if(banks[0].length>256||banks[1].length>256)throw new Error(`VRAM tiles: bank0 ${banks[0].length} bank1 ${banks[1].length}`);

// ---------- sound: record calls and convert to register stream ----------
const ev=[];const rec={sq:(fr,m,d,v=.1,duty=.5,slide=0)=>ev.push({t:'sq',fr,m,d,v,duty,slide}),tr:(fr,m,d,v=.3,slide=0)=>ev.push({t:'tr',fr,m,d,v,slide}),nz:(fr,d,v=.2,f0=3000,f1=3000)=>ev.push({t:'nz',fr,d,v,f0,f1})};
globalThis.X.VARIANTS.find(v=>v.id==='X1').sfx(rec);
const mf=m=>440*Math.pow(2,(m-69)/12);
const writes=new Map();const W_=(fr,reg,val)=>{if(!writes.has(fr))writes.set(fr,[]);writes.get(fr).push([reg&255,val&255]);};
const pulseX=f=>clamp(Math.round(2048-131072/f),0,2047),waveX=f=>clamp(Math.round(2048-65536/f),0,2047);
const env=(V,d)=>{const n=clamp(Math.round(d*64/Math.max(1,V)),1,7);return (V<<4)|n;};
function noisePoly(fc){const target=fc*3;let best=null;for(let s=0;s<14;s++)for(let r=0;r<8;r++){const rate=524288/(r===0?.5:r)/Math.pow(2,s+1);const e=Math.abs(Math.log(rate/target));if(!best||e<best.e)best={e,v:(s<<4)|r};}return best.v;}
const chFree=[0,0];let chLast=[-1,-1];const offs=[];let noiseOwner=null;
ev.sort((a,b)=>a.fr-b.fr||b.v-a.v);
const dropped=[];
ev.forEach(e=>{const frames=Math.max(1,Math.ceil(e.d*60));
  if(e.t==='sq'){let ch=chFree[0]<=e.fr?0:chFree[1]<=e.fr?1:-1;
    if(ch<0){ // steal channel that started earliest unless this is a quieter chord tone at the same frame
      ch=chLast[0]<=chLast[1]?0:1; if(chLast[ch]===e.fr){dropped.push(e);return;}}
    chFree[ch]=e.fr+frames;chLast[ch]=e.fr;offs.push({ch,from:e.fr,end:e.fr+frames});
    const base=ch===0?0x10:0x15;const V=clamp(Math.round(e.v*100),1,15);const duty={0.125:0,0.25:1,0.5:2}[e.duty]??2;
    const useLen=e.d<V/64*1.0;const L=clamp(64-Math.round(e.d*256),0,63);
    const x0=pulseX(mf(e.m));
    if(ch===0)W_(e.fr,0x10,0);
    W_(e.fr,base+1,(duty<<6)|(useLen?L:0));W_(e.fr,base+2,useLen?((V<<4)|1):env(V,e.d));W_(e.fr,base+3,x0);W_(e.fr,base+4,0x80|(useLen?0x40:0)|(x0>>8));
    if(e.slide){for(let k=1;k<frames;k++){const x=pulseX(mf(e.m)*Math.pow(2,e.slide*(k/frames)/12));W_(e.fr+k,base+3,x);W_(e.fr+k,base+4,(useLen?0x40:0)|(x>>8));}}
  }else if(e.t==='tr'){const x0=waveX(mf(e.m));const vol=e.v>=.2?0x20:0x40;
    W_(e.fr,0x1A,0x80);W_(e.fr,0x1B,0);W_(e.fr,0x1C,vol);W_(e.fr,0x1D,x0);W_(e.fr,0x1E,0x80|(x0>>8));
    for(let k=1;k<frames;k++){const t=k/frames;if(e.slide){const x=waveX(mf(e.m)*Math.pow(2,e.slide*t/12));W_(e.fr+k,0x1D,x);W_(e.fr+k,0x1E,x>>8);}
      if(Math.abs(t-.7)<.5/frames)W_(e.fr+k,0x1C,vol===0x20?0x40:0x60);if(Math.abs(t-.85)<.5/frames)W_(e.fr+k,0x1C,0x60);}
    W_(e.fr+frames,0x1C,0x00);
  }else{offs.push({ch:4,from:e.fr,end:e.fr+frames});const V=clamp(Math.round(e.v*60),1,15);const useLen=e.d<V/64;const L=clamp(64-Math.round(e.d*256),0,63);
    W_(e.fr,0x20,useLen?L:0);W_(e.fr,0x21,useLen?((V<<4)|1):env(V,e.d));W_(e.fr,0x22,noisePoly(e.f0));W_(e.fr,0x23,0x80|(useLen?0x40:0));
    if(e.f0!==e.f1)for(let k=1;k<frames;k++)W_(e.fr+k,0x22,noisePoly(e.f0*Math.pow(e.f1/e.f0,k/frames)));}
});
// explicit note-off: retrigger at volume 0 (silent, DAC stays on: no pop) unless a newer note owns the channel
offs.forEach(o=>{const newer=offs.some(p=>p!==o&&p.ch===o.ch&&p.from>o.from&&p.from<=o.end);if(newer)return;
  if(o.ch===4){W_(o.end,0x21,0x08);W_(o.end,0x23,0x80);}else{const base=o.ch===0?0x10:0x15;W_(o.end,base+2,0x08);W_(o.end,base+4,0x80);}});
// wave ch: fix overlapping notes (a later NR32=0 must not cut a newer note) -> drop mute writes that land inside a newer tr note
const snd=[];[...writes.keys()].sort((a,b)=>a-b).forEach(fr=>{if(fr>=LEN)return;const w=writes.get(fr);snd.push(fr&255,fr>>8,w.length);w.forEach(([r,v])=>snd.push(r,v));});snd.push(255,255);


// ---- exact per-frame scanline offsets from the reference (stored sparse) ----
{const v=globalThis.X.VARIANTS.find(v=>v.id==='X1');const S=new Screen();const blob=[],offs=[];
 for(let f=0;f<LEN;f++){S.reset();v.draw(S,f);const a=[...S.scx];let y0=a.findIndex(x=>x!==0),y1=H-1;while(y1>=0&&a[y1]===0)y1--;
   if(y0<0){offs.push(0xFFFF);continue;}offs.push(blob.length);blob.push(y0,y1-y0+1,...a.slice(y0,y1+1).map(x=>x&255));}
 SCXDATA={blob,offs};}

// ---------- emit C ----------
const hex=a=>a.map(v=>'0x'+(v&255).toString(16).padStart(2,'0')).join(',');
const rows=(a,n=24)=>{let s='';for(let i=0;i<a.length;i+=n)s+='  '+hex(a.slice(i,i+n))+',\n';return s;};
const rgb555=k=>{const n=parseInt(COL[k].slice(1),16);const r=Math.round((n>>16&255)*31/255),g=Math.round((n>>8&255)*31/255),b=Math.round((n&255)*31/255);return r|(g<<5)|(b<<10);};
const eyePos=POS.filter(p=>p.kind==='eye'),ringPos=POS.filter(p=>p.kind==='ring');
const KIND={blank:0,static:1,corner:1,word:1,tag:1,eye:2,ring:3};
const posTile=[],posAttr=[],posVis=[],posKind=[];
POS.forEach(p=>{posKind.push(KIND[p.kind]);posVis.push(Math.min(255,Math.ceil(p.vis/2)));
  if(p.kind==='eye'){posTile.push(eyePos.indexOf(p));posAttr.push(p.pal|0x08);}
  else if(p.kind==='ring'){posTile.push(ringPos.indexOf(p));posAttr.push(p.pal|0x08);}
  else if(p.kind==='blank'){posTile.push(STRIPE[p.ty%3]);posAttr.push(BLANK_PAL);}
  else{posTile.push(p.tile);posAttr.push(p.pal);}});
const eyeTab=[];eyeStates.forEach((_,s)=>eyePos.forEach(p=>eyeTab.push(p.tiles[s])));
const ringTab=[];for(let ph=0;ph<5;ph++)ringPos.forEach(p=>ringTab.push(p.tiles[ph]));
const RT=[];for(let i=0;i<256;i++)RT.push(Math.round((h(i,4242)-.5)*254)&255);
const SIN=[];for(let i=0;i<256;i++)SIN.push(Math.round(Math.sin(i/256*Math.PI*2)*127)&255);
const per=k=>TL.map(t=>t[k]);
let out=`// AUTO-GENERATED by tools/export.js from the JS reference (X1 Static Awakening). Do not edit.
#define SC_LEN ${LEN}
#define SC_NOISE0 ${NOISE0}
#define SC_BLANK_PAL ${BLANK_PAL}
#define SC_NOISE_PAL ${NOISE_PAL}
#define SC_EYE_N ${eyePos.length}
#define SC_RING_N ${ringPos.length}
#define SC_CELLS ${cells.length}
#define SC_TILES0 ${banks[0].length}
#define SC_TILES1 ${banks[1].length}
static const uint8_t sc_stripe[3]={${STRIPE.join(',')}};
static const uint8_t sc_noise_pool[]={\n${rows(noisePool)}};
static const uint8_t sc_pos_tile[360]={\n${rows(posTile,20)}};
static const uint8_t sc_pos_attr[360]={\n${rows(posAttr,20)}};
static const uint8_t sc_pos_vis[360]={\n${rows(posVis,20)}}; // frame/2 when the tile may show
static const uint8_t sc_pos_kind[360]={\n${rows(posKind,20)}}; // 0 blank 1 static 2 eye 3 ring
static const uint8_t sc_eye_x[${eyePos.length}]={${eyePos.map(p=>p.tx).join(',')}};
static const uint8_t sc_eye_y[${eyePos.length}]={${eyePos.map(p=>p.ty).join(',')}};
static const uint8_t sc_eye_tab[]={\n${rows(eyeTab,eyePos.length)}};
static const uint8_t sc_ring_x[${ringPos.length}]={${ringPos.map(p=>p.tx).join(',')}};
static const uint8_t sc_ring_y[${ringPos.length}]={${ringPos.map(p=>p.ty).join(',')}};
static const uint8_t sc_mod3[32]={${[...Array(32).keys()].map(i=>i%3).join(',')}};
static const uint16_t sc_row20[18]={${[...Array(18).keys()].map(i=>i*20).join(',')}};
static const uint8_t sc_ring_tab[]={\n${rows(ringTab,ringPos.length)}};
static const uint8_t sc_cell_x[${cells.length}]={${cells.map(c=>c.x).join(',')}};
static const uint8_t sc_cell_y[${cells.length}]={${cells.map(c=>c.y).join(',')}};
static const uint8_t sc_slash_tab[]={\n${rows(slashTiles.flat(),cells.length)}};
// palettes: logical colour ids (0..${KEYS.length-1}) per BG palette
static const uint8_t sc_pal_logic[32]={${PAL.flat().join(',')}};
static const uint16_t sc_rgb[${KEYS.length}]={${KEYS.map(rgb555).join(',')}}; // ${KEYS.join(' ')}
enum {${KEYS.map(k=>'SC_'+k).join(',')}};
static const int8_t sc_rt[256]={${RT.map(v=>v>127?v-256:v).join(',')}};
`;

// ---- precomputed reveal orders (C reveals while thr >= p) ----
const thrC=(x,y)=>((RT[(y*29+x*13+7)&255]+128)&255);
function revList(rowsList){const L=[];rowsList.forEach(y=>{for(let x=0;x<32;x++)L.push({x,y,t:thrC(x,y)});});L.sort((a,b)=>b.t-a.t);return L;}
const rev1=revList([27,28,29,30,31,0,1,2,3,4,5,6,7,8,9,10,11,12,13]);
const rev2=revList([7,8,9,10,11,12,13,14,15,16,17]);
const visEv=[];POS.forEach((p,i)=>{if(KIND[p.kind]&&p.kind!=='ring'&&p.vis>=170)visEv.push({f:Math.ceil(p.vis/2)*2,i});});visEv.sort((a,b)=>a.f-b.f);
const fadeTab=[];for(let k=0;k<=6;k++)for(let c=0;c<32;c++)fadeTab.push(Math.floor(c*(6-k)/6));
out+=`#define SC_REV1_N ${rev1.length}
#define SC_REV2_N ${rev2.length}
#define SC_VIS_N ${visEv.length}
static const uint8_t sc_rev1_x[]={${rev1.map(r=>r.x).join(',')}};
static const uint8_t sc_rev1_y[]={${rev1.map(r=>r.y).join(',')}};
static const uint8_t sc_rev1_t[]={${rev1.map(r=>r.t).join(',')}};
static const uint8_t sc_rev2_x[]={${rev2.map(r=>r.x).join(',')}};
static const uint8_t sc_rev2_y[]={${rev2.map(r=>r.y).join(',')}};
static const uint8_t sc_rev2_t[]={${rev2.map(r=>r.t).join(',')}};
static const uint8_t sc_vis_f2[]={${visEv.map(v=>v.f>>1).join(',')}};
static const uint8_t sc_vis_x[]={${visEv.map(v=>v.i%20).join(',')}};
static const uint8_t sc_vis_y[]={${visEv.map(v=>Math.floor(v.i/20)).join(',')}};
static const uint8_t sc_fade_tab[7*32]={${fadeTab.join(',')}};
`;

['scy','eyeId','ring','slash','fade','flags','pTop','pLow'].forEach(k=>{out+=`static const uint8_t sc_f_${k}[SC_LEN]={\n${rows(per(k))}};\n`;});
out+=`static const uint8_t sc_snd[]={\n${rows(snd)}};\n`;
fs.mkdirSync(process.argv[2]||'.',{recursive:true});
fs.writeFileSync((process.argv[2]||'.')+'/studio_card_data.h',out);
fs.writeFileSync((process.argv[2]||'.')+'/studio_card_counts.h',`// AUTO-GENERATED\n#define SC_TILES0 ${banks[0].length}\n#define SC_TILES1 ${banks[1].length}\n`);
fs.writeFileSync((process.argv[2]||'.')+'/studio_card_scx.h',`// AUTO-GENERATED by tools/export.js: per-frame scanline SCX (sparse rows)\nconst uint16_t sc_scx_off[${LEN}]={${SCXDATA.offs.join(',')}};\nconst uint8_t sc_scx_blob[${SCXDATA.blob.length}]={\n${rows(SCXDATA.blob)}};\n`);
console.log('scx bytes',SCXDATA.blob.length+LEN*2);
fs.writeFileSync((process.argv[2]||'.')+'/studio_card_gfx.h',`// AUTO-GENERATED by tools/export.js. VRAM tile data (lives in its own ROM bank).\nconst uint8_t sc_tiles0[]={\n${rows(banks[0].flat())}};\nconst uint8_t sc_tiles1[]={\n${rows(banks[1].flat())}};\n`);
console.log(JSON.stringify({tiles0:banks[0].length,tiles1:banks[1].length,pals:PAL.length,noisePal:NOISE_PAL,eyeStates:eyeStates.length,eyeN:eyePos.length,ringN:ringPos.length,cells:cells.length,maxSprPerLine:maxPer,sndBytes:snd.length,dropped:dropped.length,bytes:out.length}));
// also dump reference frames for comparison
if(process.argv[3]){const v=globalThis.X.VARIANTS.find(v=>v.id==='X1');const S=new Screen();const list=process.argv[3].split(',').map(Number);const buf=[];list.forEach(f=>{S.reset();v.draw(S,f);const d=new Uint8ClampedArray(W*H*4);globalThis.X_render(S,d);buf.push(Buffer.from(d));});fs.writeFileSync((process.argv[2]||'.')+'/ref.raw',Buffer.concat(buf));}
