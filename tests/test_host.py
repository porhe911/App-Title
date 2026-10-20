#!/usr/bin/env python3
"""Exercise real rendering and HTTP code on the host. No PS5 claims."""
import hashlib
import http.client
import json
from pathlib import Path
import subprocess
import time
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT/'build-host'
BUILD.mkdir(exist_ok=True)
def run(*args):
    subprocess.run(args, cwd=ROOT, check=True)
run('make','host-test')
run('g++','-std=c++17','-O2','-Iinclude','-Ivendor/stb','tests/render_cli.cpp','src/core.cpp','src/paths.cpp','src/icons.cpp','-o','build-host/render_cli')
run('python3','tools/embed.py','assets/index.html','build-host/html.cpp','apptitle_settings_html')
run('g++','-std=c++17','-O2','-DAPP_TITLE_HOST_TEST','-Iinclude','tests/settings_host.cpp','src/core.cpp','src/paths.cpp','src/settings.cpp','src/style_export.cpp','build-host/html.cpp','-pthread','-o','build-host/settings_host')

font='/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
src=BUILD/'source.png'
im=Image.new('RGBA',(512,512),(46,94,120,255));im.save(src)
before=hashlib.sha256(src.read_bytes()).hexdigest()
for pos,color in [('top','ffffff'),('bottom','65d6ff'),('top','000000')]:
    out=BUILD/f'{pos}-{color}.png'
    run(str(BUILD/'render_cli'),str(src),str(out),'PPSA03846',pos,color,font)
    result=Image.open(out).convert('RGBA')
    assert result.size==(512,512)
    diff=ImageChops.difference(result.convert('RGB'),im.convert('RGB')).getbbox()
    assert diff and ((diff[1]==0 and diff[3]<110) if pos=='top' else (diff[1]>400 and diff[3]==512)),diff
    rgb=tuple(bytes.fromhex(color))
    assert sum(p[:3]==rgb for p in list(result.get_flattened_data() if hasattr(result,'get_flattened_data') else result.getdata()))>100,'Text color was not rendered'
assert hashlib.sha256(src.read_bytes()).hexdigest()==before,'Original was changed'
print('PASS: PNG dimensions, top/bottom positions, three colors, contrast and original SHA256')

def request(method,path,body=None,extra=None):
    c=http.client.HTTPConnection('127.0.0.1',39031,timeout=3)
    c.request(method,path,body,headers=extra or {})
    response=c.getresponse();data=response.read();c.close();return response.status,data

(BUILD/'config.ini').unlink(missing_ok=True)
server=subprocess.Popen([str(BUILD/'settings_host')],cwd=ROOT,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
try:
    for attempt in range(50):
        try:
            status,data=request('GET','/api/state')
            state=json.loads(data)
            if status==200 and state['pid']==-12345:break
        except (OSError,ValueError):pass
        time.sleep(.05)
    else:raise RuntimeError('Host settings server did not start')
    assert len(state['token'])==32 and state['placement']=='outside'
    assert state['version']=='0.2.11' and state['queries']==7 and state['overrides']==5 and state['style_supported'] is True
    assert state['position']=='bottom' and state['color']=='ffffff' and state['style_state']==2 and state['style_patches']==1
    duplicate=subprocess.run([str(BUILD/'settings_host')],cwd=ROOT,timeout=3,
                             stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    assert duplicate.returncode==1,'Settings startup must report a port conflict'
    good={'X-App-Title-Token':state['token'],'Origin':'http://127.0.0.1:39031'}
    body='enabled=1&position=top&color=65d6ff'
    assert request('POST','/api/settings',body)[0]==403
    assert request('POST','/api/settings',body,{**good,'Origin':'https://example.com'})[0]==403
    assert request('GET','/api/state',extra={'Host':'example.com'})[0]==403
    assert request('POST','/api/settings',body+'&enabled=0',good)[0]==400
    assert request('POST','/api/settings',body+'\0ignored=1',good)[0]==400
    assert request('POST','/api/settings',body,good)[0]==200
    saved=json.loads(request('GET','/api/state')[1]);assert saved['position']=='top' and saved['color']=='65d6ff'
    assert 'color=65d6ff' in (BUILD/'config.ini').read_text()
    print('PASS: real HTTP GET/POST, persistence, token, Host/Origin checks and malformed settings rejection')
    assert request('POST','/api/export','export=1')[0]==403
    assert request('POST','/api/export','bad',good)[0]==400
    run('node','tests/test_ui.js')
    assert request('POST','/api/export','export=1',good)[0]==409
    assert json.loads(request('GET','/api/state')[1])['export_state']=='queued'
    print('PASS: authenticated manual export, malformed/duplicate request rejection and status')
finally:
    server.terminate();server.wait(timeout=5)

server=subprocess.Popen([str(BUILD/'settings_host')],cwd=ROOT,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
try:
    time.sleep(.2)
    state=json.loads(request('GET','/api/state')[1]);assert state['enabled']==0 and state['position']=='bottom' and state['color']=='ffffff' and state['placement']=='outside'
    print('PASS: saved settings survive service restart')
finally:
    server.terminate();server.wait(timeout=5)

