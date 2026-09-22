"use strict";
const $=id=>document.getElementById(id);
const S={bpm:120,cutBars:4,punch:0.85,sens:1.3,agit:0.7,beams:0.85,relief:1.1,gap:18,scroll:0.7,parts:0.7,bloom:0.8,trails:0.45,hue:0,glitch:0.12,grain:0.4,auto:false,paused:false};
const D2=v=>v.toFixed(2),D0=v=>v.toFixed(0);
function bind(id,key,disp){const el=$(id),out=$(id+'V');if(!el)return;el.addEventListener('input',()=>{S[key]=parseFloat(el.value);if(out)out.textContent=disp?disp(S[key]):el.value;});}
bind('bpm','bpm',D0);bind('cut','cutBars',D0);bind('punch','punch',D2);bind('sens','sens',D2);bind('agit','agit',D2);bind('beams','beams',D2);
bind('relief','relief',D2);bind('gap','gap',D0);bind('scroll','scroll',D2);bind('parts','parts',D2);bind('bloom','bloom',D2);bind('trails','trails',D2);bind('hue','hue');bind('glitch','glitch',D2);bind('grain','grain',D2);
function setParam(id,key,v,disp){S[key]=v;const el=$(id);if(el)el.value=String(v);const o=$(id+'V');if(o)o.textContent=disp?disp(v):String(v);}

/* ---- Perlin noise ---- */
const Perlin=(()=>{const p=new Uint8Array(512),pm=[...Array(256).keys()];for(let i=255;i>0;i--){const j=(Math.random()*(i+1))|0;[pm[i],pm[j]]=[pm[j],pm[i]];}for(let i=0;i<512;i++)p[i]=pm[i&255];
  const fd=t=>t*t*t*(t*(t*6-15)+10),lp=(t,a,b)=>a+t*(b-a),gr=(h,x,y,z)=>{const u=h<8?x:y,v=h<4?y:(h===12||h===14?x:z);return((h&1)===0?u:-u)+((h&2)===0?v:-v);};
  return(x,y,z)=>{const X=Math.floor(x)&255,Y=Math.floor(y)&255,Z=Math.floor(z)&255;x-=Math.floor(x);y-=Math.floor(y);z-=Math.floor(z);const u=fd(x),v=fd(y),w=fd(z),A=p[X]+Y,AA=p[A]+Z,AB=p[A+1]+Z,B=p[X+1]+Y,BA=p[B]+Z,BB=p[B+1]+Z;return lp(w,lp(v,lp(u,gr(p[AA],x,y,z),gr(p[BA],x-1,y,z)),lp(u,gr(p[AB],x,y-1,z),gr(p[BB],x-1,y-1,z))),lp(v,lp(u,gr(p[AA+1],x,y,z-1),gr(p[BA+1],x-1,y,z-1)),lp(u,gr(p[AB+1],x,y-1,z-1),gr(p[BB+1],x-1,y-1,z-1))));};})();

/* ---- BoC palette ---- */
const MOODS=[{h0:8,h1:36,sat:0.50,acc:176},{h0:18,h1:44,sat:0.42,acc:184},{h0:12,h1:40,sat:0.30,acc:190}];
let moodIdx=0;
function palHue(t){const m=MOODS[moodIdx];return m.h0+(m.h1-m.h0)*t+S.hue;}
function palSat(){return MOODS[moodIdx].sat;}
function accHue(){return MOODS[moodIdx].acc;}

/* ---- constants ---- */
const COLS=96,ROWS=120,WIDTH=64,DEPTH=140,FRONT=40,MARGIN=4,NBEAM=28;
let renderer,scene,camera,rt,accum,ap=0,bloomA,bloomB,fsScene,fsCam,fsQuad,fbMat,brightMat,blurMat,finalMat;
const baseFov=60;
const env={bass:0,mid:0,high:0};const spec=new Float32Array(COLS);
const bandE=new Float32Array(COLS),slowE=new Float32Array(COLS),spike=new Float32Array(COLS);
const spr={bass:0,mid:0,high:0,vb:0,vm:0,vh:0};
let energy=0,glowTex,_rgb=[0,0,0];
let beatEnv=0,cutFlash=0,datamosh=0,beatFloat=0,lastBeatInt=-1,lastBar=-1,curBeatInBar=0,barCount=0,roll=0,rollTarget=0;
let particles,ppos,pseed,PN=2800;let safeMode=false;
let wsTimingActive=false; // true when transport messages drive the beat clock

/* ---- color helper ---- */
function hsl(h,s,l,out,o){h=((h%360)+360)%360/360;let r,g,b;if(s===0){r=g=b=l;}else{const q=l<0.5?l*(1+s):l+s-l*s,p=2*l-q;const hk=t=>{t=t<0?t+1:t>1?t-1:t;return t<1/6?p+(q-p)*6*t:t<1/2?q:t<2/3?p+(q-p)*(2/3-t)*6:p;};r=hk(h+1/3);g=hk(h);b=hk(h-1/3);}out[o]=r;out[o+1]=g;out[o+2]=b;}

/* ---- texture helpers ---- */
function makeGlowTexture(){const w=64,h=256,c=document.createElement('canvas');c.width=w;c.height=h;const x=c.getContext('2d'),img=x.createImageData(w,h);for(let yy=0;yy<h;yy++){const fy=Math.min(1,Math.min(yy,h-yy)/26);for(let xx=0;xx<w;xx++){const dx=(xx-31.5)/6.5,gx=Math.exp(-dx*dx);const a=Math.max(0,Math.min(1,gx*fy))*255,o=(yy*w+xx)*4;img.data[o]=255;img.data[o+1]=255;img.data[o+2]=255;img.data[o+3]=a;}}x.putImageData(img,0,0);return new THREE.CanvasTexture(c);}
function dotGlow(){const s=64,c=document.createElement('canvas');c.width=c.height=s;const x=c.getContext('2d');const g=x.createRadialGradient(s/2,s/2,0,s/2,s/2,s/2);g.addColorStop(0,'rgba(255,255,255,1)');g.addColorStop(0.5,'rgba(255,255,255,.4)');g.addColorStop(1,'rgba(255,255,255,0)');x.fillStyle=g;x.fillRect(0,0,s,s);return new THREE.CanvasTexture(c);}

/* ---- worlds ---- */
const worlds=[];let activeWorld=0;

function buildTunnel(){
  const group=new THREE.Group();
  const geo=new THREE.PlaneGeometry(WIDTH,DEPTH,COLS-1,ROWS-1);
  const posArr=geo.attributes.position.array,colArr=new Float32Array(posArr.length);
  geo.setAttribute('color',new THREE.BufferAttribute(colArr,3));for(let i=0;i<posArr.length;i+=3)posArr[i+2]=0;
  const baseH=new Float32Array(ROWS*COLS),specSmooth=new Float32Array(COLS);
  const mat=new THREE.MeshBasicMaterial({wireframe:true,vertexColors:true,fog:true,transparent:true,opacity:0.95,blending:THREE.AdditiveBlending});
  const floor=new THREE.Mesh(geo,mat);floor.rotation.x=-Math.PI/2;floor.position.y=-S.gap;group.add(floor);
  const ceil=new THREE.Mesh(geo,mat);ceil.rotation.x=-Math.PI/2;ceil.scale.z=-1;ceil.position.y=+S.gap;group.add(ceil);
  const beams=[];for(let i=0;i<NBEAM;i++){const m=new THREE.SpriteMaterial({map:glowTex,color:0xffffff,transparent:true,opacity:0,blending:THREE.AdditiveBlending,depthWrite:false,fog:true});const sp=new THREE.Sprite(m);sp.visible=false;group.add(sp);beams.push({sp,active:false,life:0,x:0,z:0,inten:0});}
  let scrollAcc=0;
  function spawnBeam(ix,st){for(let b=0;b<NBEAM;b++){if(!beams[b].active){const bm=beams[b];bm.active=true;bm.life=1;bm.inten=Math.min(1.4,st);bm.x=(ix/(COLS-1)-0.5)*WIDTH;bm.z=56;hsl(accHue(),0.55,0.62,_rgb,0);bm.sp.material.color.setRGB(_rgb[0],_rgb[1],_rgb[2]);bm.sp.visible=true;return;}}}
  function update(dt,clock){
    scrollAcc+=(S.scroll*(0.9+spr.bass*0.8))*(dt/16.67);
    while(scrollAcc>=1){baseH.copyWithin(0,COLS,ROWS*COLS);const last=(ROWS-1)*COLS;for(let ix=0;ix<COLS;ix++){specSmooth[ix]+=(spec[ix]-specSmooth[ix])*0.5;baseH[last+ix]=specSmooth[ix];}scrollAcc-=1;}
    if(S.beams>0){let sp=0;for(let ix=1;ix<COLS-1&&sp<3;ix++){if(spike[ix]>0.28&&spike[ix]>=spike[ix-1]&&spike[ix]>=spike[ix+1]){spawnBeam(ix,spike[ix]);sp++;ix+=4;}}}
    const recede=S.scroll*1.3*(dt/16.67);
    for(let b=0;b<NBEAM;b++){const bm=beams[b];if(!bm.active)continue;bm.z-=recede;bm.life-=dt*0.0017;if(bm.life<=0||bm.z<-80){bm.active=false;bm.sp.visible=false;continue;}bm.sp.position.set(bm.x,0,bm.z);bm.sp.scale.set((0.9+bm.inten*1.6),2*S.gap,1);bm.sp.material.opacity=Math.max(0,bm.life*bm.inten)*S.beams;}
    const reliefScale=8.5*S.relief,bloom=energy*0.1+beatEnv*S.punch*0.12+cutFlash*0.2,effGap=S.gap*(1-beatEnv*S.punch*0.05),cap=effGap-MARGIN,sat=palSat();
    for(let iy=0;iy<ROWS;iy++){const near=iy/ROWS,frontW=Math.max(0,(iy-(ROWS-1-FRONT))/FRONT);
      for(let ix=0;ix<COLS;ix++){const i=iy*COLS+ix;let h=baseH[i]*reliefScale;
        if(S.agit>0&&frontW>0){const wig=0.7+0.3*Math.sin(clock*9+ix*0.7+iy*0.5);h+=(bandE[ix]*0.55+spike[ix]*1.6)*S.agit*frontW*6*wig;}
        h+=Math.sin(ix*0.18+clock*2+iy*0.3)*Math.cos(iy*0.23-clock*1.3)*0.5*near;
        if(h>cap)h=cap;if(h<-cap)h=-cap;posArr[i*3+2]=h;
        const tn=Math.min(1,Math.abs(h)/(reliefScale*0.55));hsl(palHue(ix/(COLS-1)),sat,Math.min(0.92,0.13+tn*0.55+bloom),colArr,i*3);}}
    geo.attributes.position.needsUpdate=true;geo.attributes.color.needsUpdate=true;floor.position.y=-effGap;ceil.position.y=+effGap;}
  return{group,update,cam:{pos:[0,0,70],look:[0,0,-40]}};
}

function buildSphere(){
  const group=new THREE.Group();const GW=9,GH=7,cellW=6.6,cellH=6.6,bow=0.07,cz=(x,y)=>-bow*(x*x+y*y);
  const glyGeo=new THREE.BufferGeometry(),gly=new THREE.LineSegments(glyGeo,new THREE.LineBasicMaterial({vertexColors:true,transparent:true,opacity:0.95,blending:THREE.AdditiveBlending}));group.add(gly);
  const frGeo=new THREE.BufferGeometry(),fr=new THREE.LineSegments(frGeo,new THREE.LineBasicMaterial({vertexColors:true,transparent:true,opacity:0.5,blending:THREE.AdditiveBlending}));group.add(fr);
  const nodeMat=new THREE.MeshBasicMaterial({transparent:true,opacity:0.9,blending:THREE.AdditiveBlending});
  const nodes=new THREE.InstancedMesh(new THREE.OctahedronGeometry(0.7,0),nodeMat,GW*GH);group.add(nodes);const dummy=new THREE.Object3D();
  const star=new THREE.LineSegments(new THREE.BufferGeometry().setFromPoints([new THREE.Vector3(-3,0,.4),new THREE.Vector3(3,0,.4),new THREE.Vector3(0,-3,.4),new THREE.Vector3(0,3,.4),new THREE.Vector3(-1.6,-1.6,.4),new THREE.Vector3(1.6,1.6,.4),new THREE.Vector3(-1.6,1.6,.4),new THREE.Vector3(1.6,-1.6,.4)]),new THREE.LineBasicMaterial({color:0xffffff,transparent:true,opacity:0.9,blending:THREE.AdditiveBlending}));group.add(star);
  let glyCol,glyCell,glyN=0,cellFreq;const nodePos=[];
  function regen(){const P=[],C=[],CL=[],FP=[],FC=[];nodePos.length=0;
    for(let gy=0;gy<GH;gy++)for(let gx=0;gx<GW;gx++){const ci=gy*GW+gx,cx=(gx-(GW-1)/2)*cellW,cy=(gy-(GH-1)/2)*cellH;nodePos.push([cx,cy,cz(cx,cy)]);
      const hw=cellW*0.42,hh=cellH*0.42,corn=[[cx-hw,cy-hh],[cx+hw,cy-hh],[cx+hw,cy+hh],[cx-hw,cy+hh]];
      for(let k=0;k<4;k++){const a=corn[k],b=corn[(k+1)%4];FP.push(a[0],a[1],cz(a[0],a[1]),b[0],b[1],cz(b[0],b[1]));FC.push(0,0,0,0,0,0);}
      const n=4,step=cellW*0.6/(n-1),ox=cx-cellW*0.3,oy=cy-cellH*0.3,nSeg=4+(Math.random()*5|0),dirs=[[1,0],[0,1],[1,1],[1,-1]];
      for(let s=0;s<nSeg;s++){let axi=Math.random()*n|0,ayi=Math.random()*n|0;const d=dirs[Math.random()*dirs.length|0];let bxi=Math.min(n-1,Math.max(0,axi+d[0])),byi=Math.min(n-1,Math.max(0,ayi+d[1]));const ax=ox+axi*step,ay=oy+ayi*step,bx=ox+bxi*step,by=oy+byi*step;P.push(ax,ay,cz(ax,ay),bx,by,cz(bx,by));C.push(0,0,0,0,0,0);CL.push(ci,ci);}}
    glyCol=new Float32Array(C);glyCell=CL;glyN=CL.length;
    glyGeo.setAttribute('position',new THREE.BufferAttribute(new Float32Array(P),3));glyGeo.setAttribute('color',new THREE.BufferAttribute(glyCol,3));
    frGeo.setAttribute('position',new THREE.BufferAttribute(new Float32Array(FP),3));frGeo.setAttribute('color',new THREE.BufferAttribute(new Float32Array(FC),3));
    cellFreq=new Int16Array(GW*GH);for(let i=0;i<GW*GH;i++)cellFreq[i]=Math.min(COLS-1,(i/(GW*GH)*COLS)|0);}
  regen();
  function update(dt,clock){const sat=palSat();
    for(let v=0;v<glyN;v++){const ci=glyCell[v],f=cellFreq[ci];let b=0.12+bandE[f]*1.3+spike[f]*1.6+beatEnv*S.punch*0.35+cutFlash*0.4;b*=(0.85+0.3*Math.sin(clock*3+ci));b=Math.min(1.1,b);hsl(palHue(ci/(GW*GH)),sat,Math.min(0.9,0.1+b*0.5),_rgb,0);glyCol[v*3]=_rgb[0];glyCol[v*3+1]=_rgb[1];glyCol[v*3+2]=_rgb[2];}
    glyGeo.attributes.color.needsUpdate=true;
    const fc=frGeo.attributes.color.array;for(let i=0;i<GW*GH;i++){const f=cellFreq[i],b=0.18+bandE[f]*0.7+spike[f]*0.6;hsl(accHue(),sat*0.7,Math.min(0.6,0.12+b*0.4),_rgb,0);for(let k=0;k<8;k++){const o=(i*8+k)*3;fc[o]=_rgb[0];fc[o+1]=_rgb[1];fc[o+2]=_rgb[2];}}
    frGeo.attributes.color.needsUpdate=true;
    for(let i=0;i<nodePos.length;i++){const f=cellFreq[i],e=bandE[f]+spike[f]*1.2,sc=0.5+e*1.8+beatEnv*S.punch*0.6,np=nodePos[i];dummy.position.set(np[0],np[1],np[2]+e*2);dummy.scale.setScalar(sc*(0.5+S.relief*0.4));dummy.rotation.set(clock+i,clock*0.7,0);dummy.updateMatrix();nodes.setMatrixAt(i,dummy.matrix);}
    nodes.instanceMatrix.needsUpdate=true;hsl(palHue(0.5),sat,0.55,_rgb,0);nodeMat.color.setRGB(_rgb[0],_rgb[1],_rgb[2]);star.material.opacity=0.5+energy*0.5+beatEnv*S.punch*0.5;
    group.rotation.z=Math.sin(clock*0.12)*0.05;group.position.z=Math.sin(clock*0.3)*0.6;}
  return{group,update,enter:regen,cam:{pos:[0,0,42],look:[0,0,0]}};
}

/* ---- particles ---- */
function buildParticles(){
  ppos=new Float32Array(PN*3);pseed=new Float32Array(PN);const R=80;
  for(let i=0;i<PN;i++){ppos[i*3]=(Math.random()-0.5)*R*2;ppos[i*3+1]=(Math.random()-0.5)*R*1.4;ppos[i*3+2]=(Math.random()-0.5)*R*2;pseed[i]=Math.random()*10;}
  const g=new THREE.BufferGeometry();g.setAttribute('position',new THREE.BufferAttribute(ppos,3));
  const mat=new THREE.PointsMaterial({map:dotGlow(),color:0xffffff,size:1.1,transparent:true,opacity:0.45,blending:THREE.AdditiveBlending,depthWrite:false,sizeAttenuation:true});
  particles=new THREE.Points(g,mat);particles.frustumCulled=false;scene.add(particles);
}
function updateParticles(dt,clock){if(!particles)return;const mat=particles.material;
  mat.size=(0.6+spr.bass*1.6+beatEnv*S.punch*0.8)*(0.5+S.parts);mat.opacity=(0.18+energy*0.5)*Math.min(1,S.parts);
  hsl(accHue(),palSat()*0.6,0.7,_rgb,0);mat.color.setRGB(_rgb[0],_rgb[1],_rgb[2]);
  if(S.parts<=0.001){particles.visible=false;return;}particles.visible=true;
  const sp=(0.4+spr.mid*2.4+beatEnv*S.punch*1.5)*(dt/16.67),sc=0.012;
  for(let i=0;i<PN;i++){let x=ppos[i*3],y=ppos[i*3+1],z=ppos[i*3+2];
    const th=Perlin(x*sc,y*sc,z*sc+clock*0.1+pseed[i])*Math.PI*4,ph=Perlin(x*sc+4,y*sc,z*sc-clock*0.1)*Math.PI*2;
    x+=Math.sin(ph)*Math.cos(th)*sp;y+=Math.sin(ph)*Math.sin(th)*sp;z+=Math.cos(ph)*sp;
    if(x*x+y*y+z*z>6400){const k=Math.random()*0.3;x*=k;y*=k;z*=k;}
    ppos[i*3]=x;ppos[i*3+1]=y;ppos[i*3+2]=z;}
  particles.geometry.attributes.position.needsUpdate=true;particles.rotation.y+=dt*0.00004;
}

/* ---- post-processing pipeline ---- */
function makeRT(w,h){return new THREE.WebGLRenderTarget(w,h,{minFilter:THREE.LinearFilter,magFilter:THREE.LinearFilter,depthBuffer:true});}
const PASS_V='varying vec2 vUv;void main(){vUv=uv;gl_Position=vec4(position,1.0);}';

function initThree(){
  if(!window.THREE){$('err').style.display='flex';$('err').textContent='Three.js not loaded — check that three.min.js is next to index.html.';return;}
  renderer=new THREE.WebGLRenderer({canvas:$('stage'),antialias:true});
  renderer.setPixelRatio(Math.min(window.devicePixelRatio||1,1.25));renderer.setSize(innerWidth,innerHeight);renderer.setClearColor(0x000000,1);
  scene=new THREE.Scene();scene.fog=new THREE.Fog(0x000000,26,210);
  camera=new THREE.PerspectiveCamera(baseFov,innerWidth/innerHeight,0.1,500);
  glowTex=makeGlowTexture();
  worlds.push(buildTunnel(),buildSphere());worlds.forEach((w,i)=>{w.group.visible=(i===0);scene.add(w.group);});applyWorldCam(0);
  buildParticles();
  const W=innerWidth,H=innerHeight,hw=Math.max(2,W>>1),hh=Math.max(2,H>>1);
  rt=makeRT(W,H);accum=[makeRT(W,H),makeRT(W,H)];bloomA=makeRT(hw,hh);bloomB=makeRT(hw,hh);
  fsCam=new THREE.OrthographicCamera(-1,1,1,-1,0,1);fsScene=new THREE.Scene();fsQuad=new THREE.Mesh(new THREE.PlaneGeometry(2,2),new THREE.MeshBasicMaterial());fsScene.add(fsQuad);
  fbMat=new THREE.ShaderMaterial({uniforms:{uScene:{value:null},uPrev:{value:null},uDecay:{value:0},uZoom:{value:0.004}},vertexShader:PASS_V,
    fragmentShader:'precision highp float;uniform sampler2D uScene,uPrev;uniform float uDecay,uZoom;varying vec2 vUv;void main(){vec2 c=vUv-0.5;vec2 puv=0.5+c*(1.0-uZoom);vec3 prev=texture2D(uPrev,puv).rgb*uDecay;vec3 scn=texture2D(uScene,vUv).rgb;gl_FragColor=vec4(max(scn,prev),1.0);}'});
  brightMat=new THREE.ShaderMaterial({uniforms:{tDiffuse:{value:null},uThresh:{value:0.28}},vertexShader:PASS_V,
    fragmentShader:'precision highp float;uniform sampler2D tDiffuse;uniform float uThresh;varying vec2 vUv;void main(){vec3 c=texture2D(tDiffuse,vUv).rgb;float l=dot(c,vec3(0.299,0.587,0.114));float k=max(0.0,l-uThresh);gl_FragColor=vec4(c*k*2.2,1.0);}'});
  blurMat=new THREE.ShaderMaterial({uniforms:{tDiffuse:{value:null},uDir:{value:new THREE.Vector2()}},vertexShader:PASS_V,
    fragmentShader:'precision highp float;uniform sampler2D tDiffuse;uniform vec2 uDir;varying vec2 vUv;void main(){float w0=0.227,w1=0.194,w2=0.121,w3=0.054,w4=0.016;vec3 s=texture2D(tDiffuse,vUv).rgb*w0;s+=texture2D(tDiffuse,vUv+uDir).rgb*w1;s+=texture2D(tDiffuse,vUv-uDir).rgb*w1;s+=texture2D(tDiffuse,vUv+uDir*2.0).rgb*w2;s+=texture2D(tDiffuse,vUv-uDir*2.0).rgb*w2;s+=texture2D(tDiffuse,vUv+uDir*3.0).rgb*w3;s+=texture2D(tDiffuse,vUv-uDir*3.0).rgb*w3;s+=texture2D(tDiffuse,vUv+uDir*4.0).rgb*w4;s+=texture2D(tDiffuse,vUv-uDir*4.0).rgb*w4;gl_FragColor=vec4(s,1.0);}'});
  finalMat=new THREE.ShaderMaterial({uniforms:{tDiffuse:{value:null},tBloom:{value:null},uGlitch:{value:0},uTime:{value:0},uGrain:{value:0.4},uCut:{value:0},uBloom:{value:0.8}},vertexShader:PASS_V,
    fragmentShader:['precision highp float;uniform sampler2D tDiffuse,tBloom;uniform float uGlitch,uTime,uGrain,uCut,uBloom;varying vec2 vUv;',
      'float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}',
      'void main(){vec2 d=vUv-0.5;float r2=dot(d,d);vec2 uv=0.5+d*(1.0+0.12*r2);float g=uGlitch;',
      ' uv.x+=sin(uv.y*8.0+uTime*2.0)*0.0008+(hash(vec2(floor(uTime*9.0),floor(uv.y*60.0)))-0.5)*0.0016*(0.4+g);',
      ' float slice=floor(uv.y*22.0);float t=floor(uTime*12.0);float trig=step(0.80,hash(vec2(slice,t)));',
      ' uv.x+=(hash(vec2(slice,t*1.7))-0.5)*g*0.22*trig;',
      ' float ca=0.0035+g*0.025*trig+r2*0.06;',
      ' float rr=texture2D(tDiffuse,uv+vec2(ca,0.0)).r;float gg=texture2D(tDiffuse,uv).g;float bb=texture2D(tDiffuse,uv-vec2(ca,0.0)).b;',
      ' vec3 col=vec3(rr,gg,bb);col+=texture2D(tBloom,uv).rgb*uBloom;',
      ' float blk=step(0.984,hash(vec2(floor(uv.x*40.0),t)))*g;col*=(1.0-blk*0.85);',
      ' col+=(hash(uv*vec2(uTime*60.0+1.0,uTime*60.0+2.0))-0.5)*(0.05+uGrain*0.18);',
      ' col*=1.0-0.05*step(0.5,fract(uv.y*240.0))*(0.4+uGrain);',
      ' col+=uCut*0.5;col*=1.0-r2*0.5;gl_FragColor=vec4(col,1.0);}'].join('\n')});
  addEventListener('resize',()=>{const W=innerWidth,H=innerHeight,hw=Math.max(2,W>>1),hh=Math.max(2,H>>1);renderer.setSize(W,H);camera.aspect=W/H;camera.updateProjectionMatrix();rt.setSize(W,H);accum[0].setSize(W,H);accum[1].setSize(W,H);bloomA.setSize(hw,hh);bloomB.setSize(hw,hh);});
}
function pass(mat,target){fsQuad.material=mat;renderer.setRenderTarget(target);renderer.render(fsScene,fsCam);}
let camPos=[0,0,70],camLook=[0,0,-40];
function applyWorldCam(i){const c=worlds[i].cam;camPos=c.pos.slice();camLook=c.look.slice();}
function setWorld(i){activeWorld=(i+worlds.length)%worlds.length;worlds.forEach((w,k)=>w.group.visible=(k===activeWorld));applyWorldCam(activeWorld);if(worlds[activeWorld].enter)worlds[activeWorld].enter();updateSceneLbl();}

/* ---- audio ---- */
let inputMode='demo',analyser=null,freqData=null,sampleRate=44100,bandIdx=[];
const follow=(t,c)=>t>c?t:c*0.90;
function computeBandIndex(){bandIdx=[];const fLo=32,fHi=15000,hz=sampleRate/2/analyser.frequencyBinCount;for(let i=0;i<COLS;i++){const a=fLo*Math.pow(fHi/fLo,i/COLS),b=fLo*Math.pow(fHi/fLo,(i+1)/COLS);bandIdx.push([Math.max(1,(a/hz)|0),Math.max(2,Math.ceil(b/hz))]);}}
async function enableMic(){try{const st=await navigator.mediaDevices.getUserMedia({audio:{echoCancellation:false,noiseSuppression:false,autoGainControl:false}});const ac=new (window.AudioContext||window.webkitAudioContext)();sampleRate=ac.sampleRate;const src=ac.createMediaStreamSource(st);analyser=ac.createAnalyser();analyser.fftSize=2048;analyser.smoothingTimeConstant=0.5;src.connect(analyser);freqData=new Uint8Array(analyser.frequencyBinCount);computeBandIndex();inputMode='mic';setConn('mic');$('audioBtn').textContent='Mic live';$('audioBtn').classList.remove('primary');$('hint').classList.add('gone');}catch(e){$('audioBtn').textContent='Mic blocked';$('hint').textContent='Browser blocked mic — demo mode active.';}}

function readAudio(){
  if(inputMode==='mic'){
    analyser.getByteFrequencyData(freqData);const avg=(a,b)=>{let s=0;for(let i=a;i<b;i++)s+=freqData[i];return (s/(b-a))/255;};const g=S.sens;
    env.bass=Math.min(1,follow(avg(1,12)*g,env.bass));env.mid=Math.min(1,follow(avg(12,90)*g,env.mid));env.high=Math.min(1,follow(avg(90,360)*g,env.high));
    for(let i=0;i<COLS;i++){const r=bandIdx[i];let v=avg(r[0],r[1])*g*1.1;v=Math.pow(Math.min(1,v),0.85);spec[i]=v;}
  }else if(inputMode==='ws'){
    // spec[], spike[], env filled by WS handler — just smooth bandE
  }else{
    const tt=performance.now()*0.0009;for(let i=0;i<COLS;i++){const f=i/COLS;spec[i]=(0.08+0.10*Math.abs(Math.sin(tt*1.2+f*9)))*(1-f*0.4);}env.bass=0.18+0.16*Math.abs(Math.sin(tt));env.mid=0.16;env.high=0.10+0.10*Math.abs(Math.sin(tt*2.3));
  }
  for(let i=0;i<COLS;i++){const a=spec[Math.max(0,i-2)],b=spec[i],c=spec[Math.min(COLS-1,i+2)];const tg=(a+b*2+c)/4;bandE[i]=tg>bandE[i]?tg:bandE[i]*0.88;slowE[i]+=(bandE[i]-slowE[i])*0.12;spike[i]=Math.max(spike[i]*0.85,Math.max(0,bandE[i]-slowE[i])*3.2);}
  energy+=(((env.bass+env.mid+env.high)/3)-energy)*0.02;
  spr.vb+=(env.bass-spr.bass)*0.22;spr.vb*=0.62;spr.bass+=spr.vb;
  spr.vm+=(env.mid-spr.mid)*0.22;spr.vm*=0.62;spr.mid+=spr.vm;
  spr.vh+=(env.high-spr.high)*0.3;spr.vh*=0.5;spr.high+=spr.vh;
}

/* ---- WebSocket client ---- */
const WS_URL='ws://localhost:8765';
let ws=null,wsRetryTimer=null,wsFallbackTimer=null;
function setConn(state){const el=$('conn');if(!el)return;el.classList.remove('s-ws','s-mic','s-demo');el.classList.add('s-'+state);$('connT').textContent=state.toUpperCase();}
function connectWS(){
  try{ws=new WebSocket(WS_URL);}catch(e){scheduleReconnect();return;}
  ws.onopen=()=>{
    if(wsFallbackTimer){clearTimeout(wsFallbackTimer);wsFallbackTimer=null;}
    if(inputMode!=='mic'){inputMode='ws';setConn('ws');$('hint').classList.add('gone');}
  };
  ws.onmessage=(ev)=>{
    let msg;try{msg=JSON.parse(ev.data);}catch{return;}
    if(!msg||typeof msg!=='object')return;
    switch(msg.type){
      case 'audio':
        if(inputMode==='mic')return;
        if(inputMode!=='ws'){inputMode='ws';setConn('ws');}
        if(typeof msg.bass==='number')env.bass=msg.bass;
        if(typeof msg.mid==='number')env.mid=msg.mid;
        if(typeof msg.high==='number')env.high=msg.high;
        if(Array.isArray(msg.spec)){const n=Math.min(COLS,msg.spec.length);for(let i=0;i<n;i++){const v=+msg.spec[i];spec[i]=isFinite(v)?v:0;}}
        if(Array.isArray(msg.spike)){const n=Math.min(COLS,msg.spike.length);for(let i=0;i<n;i++){const v=+msg.spike[i];spike[i]=isFinite(v)?v:0;}}
        break;
      case 'ctrl':
        if('relief' in msg)setParam('relief','relief',+msg.relief,D2);
        if('agit'   in msg)setParam('agit','agit',+msg.agit,D2);
        if('beams'  in msg)setParam('beams','beams',+msg.beams,D2);
        if('hue'    in msg)setParam('hue','hue',+msg.hue);
        if('glitch' in msg)setParam('glitch','glitch',+msg.glitch,D2);
        if('gap'    in msg)setParam('gap','gap',+msg.gap,D0);
        if('scroll' in msg)setParam('scroll','scroll',+msg.scroll,D2);
        if('sens'   in msg)setParam('sens','sens',+msg.sens,D2);
        if('punch'  in msg)setParam('punch','punch',+msg.punch,D2);
        if('bloom'  in msg)setParam('bloom','bloom',+msg.bloom,D2);
        if('trails' in msg)setParam('trails','trails',+msg.trails,D2);
        if('parts'  in msg)setParam('parts','parts',+msg.parts,D2);
        if('grain'  in msg)setParam('grain','grain',+msg.grain,D2);
        break;
      case 'beat':
        wsTimingActive=true;
        if(typeof msg.bpm==='number'&&isFinite(msg.bpm))setParam('bpm','bpm',msg.bpm,D0);
        if(msg.beat===1){triggerCut();}
        else{onBeat(msg.beat);}
        break;
      case 'punch':
        beatEnv=Math.min(1.5,beatEnv+(isFinite(msg.strength)?msg.strength:0.5)*S.punch);
        break;
      case 'cut':
        triggerCut();
        break;
    }
  };
  const onDown=()=>{if(inputMode==='ws'){inputMode='demo';setConn('demo');wsTimingActive=false;}scheduleReconnect();};
  ws.onerror=onDown;ws.onclose=onDown;
}
function scheduleReconnect(){if(wsRetryTimer)return;wsRetryTimer=setTimeout(()=>{wsRetryTimer=null;connectWS();},3000);}
function startWS(){setConn('demo');connectWS();wsFallbackTimer=setTimeout(()=>{if(inputMode==='demo')$('hint').textContent='No bridge — demo mode. Press Enable mic for local audio.';},2000);}

/* ---- beat / trigger system ---- */
const WN=['GRID','SPHERE'];
function updateSceneLbl(){$('scene').textContent=WN[activeWorld]+' · '+['INFERNO','OCHRE','FADED'][moodIdx];}
function onBeat(n){curBeatInBar=((n-1)%4);beatEnv=curBeatInBar===0?1.0:0.55;const dots=$('beatdots').querySelectorAll('i');dots.forEach(d=>d.classList.toggle('on',+d.dataset.b===curBeatInBar));$('barlbl').textContent='BAR '+barCount;}
function triggerCut(){moodIdx=(moodIdx+1)%MOODS.length;cutFlash=1;datamosh=1;barCount++;rollTarget+=Math.PI/2*(Math.random()<0.5?1:-1);setWorld(S.auto?(Math.random()*worlds.length|0):activeWorld+1);if(S.auto)autoRandomize();$('barlbl').textContent='BAR '+barCount;const dots=$('beatdots').querySelectorAll('i');dots.forEach(d=>d.classList.remove('on'));dots[0].classList.add('on');}
const autoR=(a,b)=>a+Math.random()*(b-a);
function autoRandomize(){setParam('relief','relief',+autoR(0.7,1.9).toFixed(2),D2);setParam('agit','agit',+autoR(0.3,1.2).toFixed(2),D2);setParam('gap','gap',+autoR(12,32).toFixed(0),D0);setParam('scroll','scroll',+autoR(0.4,1.1).toFixed(2),D2);setParam('trails','trails',+autoR(0.2,0.7).toFixed(2),D2);setParam('bloom','bloom',+autoR(0.5,1.1).toFixed(2),D2);setParam('parts','parts',+autoR(0.3,1.1).toFixed(2),D2);setParam('glitch','glitch',+autoR(0.05,0.35).toFixed(2),D2);}
let dataT=0;function updateData(dt){dataT+=dt;if(dataT<90)return;dataT=0;$('data').innerHTML='RANDOM | <b>'+(Math.random()*2-1).toFixed(10)+'</b><br>RANDOM 2 | <b>'+(Math.random()*2-1).toFixed(10)+'</b><br>BPM | <b>'+S.bpm.toFixed(2)+'</b> · BAR '+barCount;}

/* ---- render loop ---- */
let t0=performance.now(),fpsAcc=0,fpsN=0,clock=0;
function render(now){
  requestAnimationFrame(render);if(!window.THREE||!renderer)return;
  const dt=Math.min(50,now-t0);t0=now;clock+=dt*0.001;
  readAudio();
  // fallback internal beat clock — only when WS transport is not driving
  if(!wsTimingActive){
    beatFloat+=(dt/1000)/(60/S.bpm);const nb=Math.floor(beatFloat);
    if(nb>lastBeatInt){lastBeatInt=nb;const n=nb%4;curBeatInBar=n;beatEnv=n===0?1.0:0.55;
      if(n===0){lastBar++;barCount=lastBar;if(lastBar>0&&lastBar%S.cutBars===0)triggerCut();}
      const dots=$('beatdots').querySelectorAll('i');dots.forEach(d=>d.classList.toggle('on',+d.dataset.b===n));$('barlbl').textContent='BAR '+barCount;}
  }
  beatEnv*=Math.exp(-dt*0.012);cutFlash*=Math.exp(-dt*0.006);datamosh*=Math.exp(-dt*0.004);roll+=(rollTarget-roll)*Math.min(1,dt*0.006);
  if(!S.paused){worlds[activeWorld].update(dt,clock);updateParticles(dt,clock);}
  camera.position.set(camPos[0]+Math.sin(clock*0.2)*2.4,camPos[1]+Math.sin(clock*0.15)*1.4,camPos[2]-cutFlash*8-beatEnv*S.punch*3);
  camera.fov=baseFov-spr.bass*4-beatEnv*S.punch*5-cutFlash*6;camera.updateProjectionMatrix();camera.lookAt(camLook[0],camLook[1],camLook[2]);camera.rotateZ(roll);
  scene.fog.far=210+energy*45;
  if(safeMode){renderer.setRenderTarget(null);renderer.render(scene,camera);return;}
  try{
    renderer.setRenderTarget(rt);renderer.render(scene,camera);
    fbMat.uniforms.uScene.value=rt.texture;fbMat.uniforms.uPrev.value=accum[ap].texture;fbMat.uniforms.uDecay.value=0.86*S.trails;
    pass(fbMat,accum[1-ap]);const main=accum[1-ap];
    brightMat.uniforms.tDiffuse.value=main.texture;pass(brightMat,bloomA);
    const tx=1/(innerWidth>>1),ty=1/(innerHeight>>1),spread=1.4;
    blurMat.uniforms.tDiffuse.value=bloomA.texture;blurMat.uniforms.uDir.value.set(tx*spread,0);pass(blurMat,bloomB);
    blurMat.uniforms.tDiffuse.value=bloomB.texture;blurMat.uniforms.uDir.value.set(0,ty*spread);pass(blurMat,bloomA);
    finalMat.uniforms.tDiffuse.value=main.texture;finalMat.uniforms.tBloom.value=bloomA.texture;
    finalMat.uniforms.uGlitch.value=Math.min(1,S.glitch+env.high*env.high*0.8+cutFlash*0.5+datamosh*0.6);
    finalMat.uniforms.uGrain.value=S.grain;finalMat.uniforms.uCut.value=cutFlash;finalMat.uniforms.uBloom.value=S.bloom;finalMat.uniforms.uTime.value=clock;
    fsQuad.material=finalMat;renderer.setRenderTarget(null);renderer.render(fsScene,fsCam);
    ap=1-ap;
  }catch(err){console.error('POST PIPELINE FAILED — falling back to direct render:',err);safeMode=true;}
  updateData(dt);
  $('mBass').style.width=(env.bass*100)+'%';$('mMid').style.width=(env.mid*100)+'%';$('mHigh').style.width=(env.high*100)+'%';
  fpsAcc+=1000/Math.max(1,dt);fpsN++;if(fpsN>=20){$('fps').textContent='FPS '+(fpsAcc/fpsN).toFixed(0)+(safeMode?' (safe)':'');fpsAcc=0;fpsN=0;}
}

/* ---- init ---- */
try{initThree();if(renderer){updateSceneLbl();requestAnimationFrame(render);startWS();}}catch(e){console.error('INIT FAILED',e);$('err').style.display='flex';$('err').textContent='Init error — open F12 Console. (Three.js must be local — no internet needed for shows)';}

$('audioBtn').addEventListener('click',enableMic);
$('fsBtn').addEventListener('click',()=>{if(!document.fullscreenElement)document.documentElement.requestFullscreen?.();else document.exitFullscreen?.();});
$('autoBtn').addEventListener('click',()=>{S.auto=!S.auto;$('autoBtn').classList.toggle('on',S.auto);if(S.auto)autoRandomize();});
$('worldBtn').addEventListener('click',()=>setWorld(activeWorld+1));
addEventListener('keydown',e=>{const k=e.key.toLowerCase();if(k==='h')$('ui').classList.toggle('hidden');else if(k==='f'){if(!document.fullscreenElement)document.documentElement.requestFullscreen?.();else document.exitFullscreen?.();}else if(k===' '){e.preventDefault();S.paused=!S.paused;}else if(k==='a')$('autoBtn').click();else if(k==='w')setWorld(activeWorld+1);else if(k==='g'){const el=$('glitch');el.value=(parseFloat(el.value)>0.5?0.12:0.85);el.dispatchEvent(new Event('input'));}});
setTimeout(()=>{const h=$('hint');if(h&&inputMode!=='demo')h.classList.add('gone');},9000);
