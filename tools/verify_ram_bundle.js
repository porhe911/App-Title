#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-or-later
// Execute the local console's actual patched JavaScript modules. No Sony
// module source is included in this script or in release archives.
const assert=require('node:assert/strict'),fs=require('node:fs'),path=require('node:path'),vm=require('node:vm');
const root=path.resolve(__dirname,'..');
const originalPath=process.argv[2],outputPrefix=process.argv[3]||'ram_style_';
if(!originalPath)throw new Error('Usage: node tools/verify_ram_bundle.js /local/4.03/HomeUI_decrypted.bin');
const original=fs.readFileSync(originalPath);
function modules(bytes) {
 const start=bytes.readUInt32LE(0x1c),plain=bytes.readUInt32LE(0x24),ram=bytes.subarray(start,start+plain);
 assert.equal(ram.readUInt32LE(0),0xfb0bd1e5);
 const count=ram.readUInt32LE(4),base=12+count*8,out=new Map();
 for(let id=0;id<count;id++) {
  const offset=base+ram.readUInt32LE(12+id*8),size=ram.readUInt32LE(16+id*8);
  if(!size)continue;
  assert(offset+size<=ram.length);assert.equal(ram[offset+size-1],0);
  out.set(id,ram.subarray(offset,offset+size-1).toString('utf8').trimEnd());
 }
 return out;
}
const before=modules(original),normalized=x=>JSON.parse(JSON.stringify(x));
let parsed=0,trees=0,geometryCases=0;
const samples=[];
for(let mode=0;mode<12;mode++) {
 const outside=mode<4||(mode>=8&&mode<10),top=!!(mode&1),color=mode>=8?'#000000':mode%4<2?'#ffffff':'#12abef';
 const current=modules(fs.readFileSync(path.join(root,'build-host',outputPrefix+mode+'.bin')));
 for(const [id,source] of current) {
  new vm.Script(source,{filename:'ram-module-'+id+'.js'});parsed++;
  if(!(outside?[25,102,219,527,531,552]:[219]).includes(id))assert.equal(source,before.get(id),'Unrelated module changed: '+id);
 }
 if(outside) {
  assert.equal(current.get(527).replace('[z.strandContainer,T.default.strandContainer]','[T.default.strandContainer,z.strandContainer]'),before.get(527));
  for(const name of ['t.compare=function','t.removeItem=function']) {
   const tail=s=>s.slice(s.indexOf(name));
   assert.equal(tail(current.get(531)),tail(before.get(531)),'Data/focus helper changed');
  }
 }
 const definitions=new Map(),cache=new Map();
 const context=vm.createContext({__haul_application:{l:(id,fn)=>definitions.set(id,fn)}});
 for(const id of [25,102,219,531,552])new vm.Script(current.get(id)).runInContext(context,{timeout:1000});
 const state={visible:true,load:'hasValue'},mounts=[];
 const react={
  __esModule:true,memo:f=>f,useMemo:f=>f(),
  createElement:(type,props,...children)=>({type,props:{...(props||{}),children:children.length===1?children[0]:children}})
 };react.default=react;
 const native={StyleSheet:{create:x=>x},Image:'Image',View:'View',Text:'Text',Animated:{View:'Animated.View'}};
 const ps={FontSizePS:{Size4XSmall:20,SizeNormal:20},IconPS:{getImageSource:uri=>({uri:'fallback:'+uri})}};
 const stubs={
  0:x=>x&&x.__esModule?x:{default:x},
  1:native,2:react,3:x=>x&&x.__esModule?x:{...x,default:x},
  4:{useRecoilValueLoadable:()=>({state:state.load,contents:state.visible})},
  5:ps,8:(...xs)=>Object.assign({},...xs),14:x=>Array.from(x),16:{},
  74:{usePerformanceMount:x=>mounts.push(x)},553:{resize4k:x=>x},554:{__esModule:true,default:'showTitleId'},
  556:{__esModule:true,default:'Telemetry'}
 };
 function load(id) {
  if(Object.hasOwn(stubs,id))return stubs[id];
  if(cache.has(id))return cache.get(id);
  const result={};cache.set(id,result);
  assert(definitions.has(id),'Unexpected dependency '+id);
  definitions.get(id)({},result,load);return result;
 }
 const styles=load(219).default,row=load(25),title=load(102),geometry=load(531),tile=load(552);
 assert.equal(styles.titleId.color,color);assert.equal(styles.titleId[top?'top':'bottom'],0);
 assert.equal(styles.titleId[top?'bottom':'top'],undefined);
 assert.equal(styles.titleId.position,'absolute');assert.equal(styles.titleId.left,0);assert.equal(styles.titleId.right,0);
 assert.equal(row.EXPERIENCE_SIZE,106);assert.equal(row.SCALED_EXP_SIZE,168);assert.equal(row.EXPERIENCE_SCALE,168/106);
 assert.equal(row.default.container.height,outside?336:168);assert.equal(row.default.strandContainer.height,outside?336:168);
 assert.equal(title.TITLE_Y,outside?214:106);assert.equal(title.TITLE_X,356);assert.equal(title.TITLE_MARGIN_LEFT,16);
 assert.equal(title.MINIMIZED_TITLE_MARGIN_LEFT,44);assert.equal(title.MINIMIZED_TITLE_MARGIN_TOP,9);
 assert.equal(title.default.experienceTitleContainer.top,title.TITLE_Y);assert.equal(title.default.itemContainer.height,62);
 for(const size of [80,106,168]) {
  const full=size+(outside?96:0),base=geometry.createStyles({itemSize:size});
  assert.deepEqual(normalized(base),{tile:{position:'absolute',width:size,height:full},container:{marginTop:-full/2}});
  const args={itemSize:size,itemMargin:8,maxItems:2,offsets:[0,114],positions:[0,114],sizes:[1,168/106],selectedValue:{interpolate:x=>x}};
  const animation=geometry.createAnimatedStyles(args);
  for(let i=0;i<2;i++) {
   assert.deepEqual(normalized(animation['selected'+i].transform),[{scale:[1,168/106][i]},{translateY:full/2},{translateX:[0,114][i]}]);
   assert.deepEqual(normalized(animation['position'+i]),{transform:[{translateX:[0,114][i]}]});
  }
  assert.deepEqual(normalized(animation.scroll.transform[0].translateX.outputRange),[-size/2,0,-(size+8)]);
  geometryCases++;
 }
 let example;
 for(const capturePerformance of [false,true])for(const visible of [false,true])for(const loadState of ['hasValue','loading']) {
  state.visible=visible;state.load=loadState;
  const mat={opacity:.25};
  const props={capturePerformance,fallbackIconPath:{uri:'fallback.png'},iconPath:{uri:'icon.png'},matStyle:mat,loadComponents:()=>{},titleId:'PPSA02046',testID:'test-tile'};
  const tree=tile.Tile(props),frame=tree.props.children;
  assert.equal(tree.type,capturePerformance?'Telemetry':'View');
  const image=outside?frame.props.children[0]:frame;
  const label=outside?frame.props.children[1]:image.props.children.props.children;
  assert.equal(image.type,'Image');assert.equal(image.props.source.uri,'icon.png');assert.equal(image.props.fallbackSource.uri,'fallback:fallback.png');
  assert.equal(image.props.children.type,'Animated.View');assert.equal(image.props.children.props.style[0],mat);
  if(visible&&loadState==='hasValue') {
   assert.equal(label.type,'Text');assert.equal(label.props.children,props.titleId);
   assert.equal(label.props.style,styles.titleId);
  } else assert.equal(label,false);
  if(outside) {
   assert.equal(frame.type,'View');assert.equal(frame.props.style.width,106);assert.equal(frame.props.style.height,202);
   assert.deepEqual(normalized(image.props.style),[{width:106,height:106,borderRadius:16},{marginTop:48}]);
   assert.deepEqual(normalized(tree.props.style),[{width:106,height:202}]);
  } else {
   assert.equal(image.props.style.width,106);assert.equal(image.props.style.height,106);
  }
  if(!capturePerformance&&visible&&loadState==='hasValue')example=normalized(frame);
  trees++;
 }
 assert.equal(mounts.length,8);
 samples.push({outside,top,styles:normalized(styles),row:normalized(row),title:normalized(title),tree:example});
}
// Keep the process alive while the asynchronous WASM compiler is working;
// an unresolved Promise alone must not turn this check into a silent success.
const verificationTimeout=setTimeout(()=>{console.error('Yoga verification timed out');process.exit(1);},30000);
(async()=>{
 const {loadYoga}=require(path.join(root,'build-ui/node_modules/yoga-layout/src/entrypoint/wasm-async-node.js'));
 const Yoga=await loadYoga();let layouts=0;
 for(const sample of samples.filter(x=>x.outside))for(const line of [20,28,40])for(const clipped of [false,true]) {
  const frame=Yoga.Node.create(),image=Yoga.Node.create(),label=Yoga.Node.create();
  frame.setWidth(106);frame.setHeight(202);frame.setOverflow(clipped?Yoga.OVERFLOW_HIDDEN:Yoga.OVERFLOW_VISIBLE);
  image.setWidth(106);image.setHeight(106);image.setMargin(Yoga.EDGE_TOP,48);
  label.setPositionType(Yoga.POSITION_TYPE_ABSOLUTE);label.setPosition(Yoga.EDGE_LEFT,0);label.setPosition(Yoga.EDGE_RIGHT,0);
  label.setPosition(sample.top?Yoga.EDGE_TOP:Yoga.EDGE_BOTTOM,0);label.setMeasureFunc(()=>({width:96,height:line}));
  frame.insertChild(image,0);frame.insertChild(label,1);frame.calculateLayout(undefined,undefined,Yoga.DIRECTION_LTR);
  const text=label.getComputedLayout(),picture=image.getComputedLayout();
  assert(text.top>=0&&text.top+text.height<=202);
  assert(sample.top?text.top+text.height<picture.top:text.top>picture.top+picture.height);
  assert.equal(sample.title.TITLE_Y-202,12);assert(sample.title.TITLE_Y+sample.title.default.itemContainer.height<=sample.row.default.container.height);
  const scale=168/106,half=202/2,worldTop=-half+half-scale*half+scale*half;
  assert(Math.abs(worldTop)<.001);assert(worldTop+scale*202<=sample.row.default.strandContainer.height);
  frame.freeRecursive();layouts++;
 }
 console.log('PASS: '+parsed+' complete RAM module parses, '+trees+' actual Tile executions, '+geometryCases+' geometry cases, '+layouts+' Yoga layouts; colour/top/bottom/inside/outside and native visibility/performance branches; launch modules unchanged (host models, not a console test)');
})().catch(error=>{console.error(error);process.exitCode=1;}).finally(()=>clearTimeout(verificationTimeout));
