#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-or-later
// Execute the supplied console's real Recoil registry atom and ID selector.
const assert=require('node:assert/strict'),fs=require('node:fs'),vm=require('node:vm');
const bytes=fs.readFileSync(process.argv[2]),start=bytes.readUInt32LE(0)===0xfb0bd1e5?0:bytes.readUInt32LE(0x1c);
const count=bytes.readUInt32LE(start+4),base=12+8*count;
assert.equal(bytes.readUInt32LE(start),0xfb0bd1e5);assert.equal(count,858);
const definitions=new Map(),cache=new Map(),requested=[];
const context=vm.createContext({__haul_application:{l:(id,fn)=>definitions.set(id,fn)}});
let value=true,error=null;
const stubs={
 4:{selector:x=>x,atomFamily:spec=>args=>spec.default(args)},
 37:{Registry:{get:async keys=>{requested.push(...keys);return {results:[{value,error}]};}}}
};
function load(id) {
 if(Object.hasOwn(stubs,id))return stubs[id];
 if(cache.has(id))return cache.get(id).exports;
 if(!definitions.has(id)) {
  assert(Number.isInteger(id)&&id>=0&&id<count);
  const at=start+base+bytes.readUInt32LE(start+12+8*id),n=bytes.readUInt32LE(start+16+8*id);
  assert(n&&at+n<=bytes.length&&bytes[at+n-1]===0);
  new vm.Script(bytes.subarray(at,at+n-1).toString('utf8')).runInContext(context,{timeout:1000});
 }
 assert(definitions.has(id),'Missing actual module '+id);
 const module={exports:{}};cache.set(id,module);definitions.get(id)(module,module.exports,load);return module.exports;
}
const deadline=setTimeout(()=>{console.error('Registry selector timed out');process.exit(1);},10000);
(async()=>{
 const selector=load(554).default;assert.equal(selector.key,'showTitleId');
 for(value of [true,false,1,0,null,undefined]) {
  const result=await selector.get({get:x=>x});
  assert.equal(Boolean(result),Boolean(value));
  assert.equal(requested.at(-1),'isHomeTitleIdDisplayEnabled');
 }
 error='registry unavailable';
 await assert.rejects(async()=>selector.get({get:x=>x}),e=>e===error);
 assert(requested.length===7&&requested.every(x=>x==='isHomeTitleIdDisplayEnabled'));
 console.log('PASS: actual 4.03 registry atom, async helper and showTitleId selector; true/false/integer/null results, exact registry key, error propagation (native Registry API mocked)');
})().catch(e=>{console.error(e);process.exitCode=1;}).finally(()=>clearTimeout(deadline));
