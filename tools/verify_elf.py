#!/usr/bin/env python3
"""Check actual ELF structure and embedding; this is not a console test."""
from pathlib import Path
import struct
import subprocess
import re

ROOT=Path(__file__).resolve().parent.parent
def check(path,shared):
    data=path.read_bytes()
    h=struct.unpack_from('<16sHHIQQQIHHHHHH',data)
    assert h[0][:5]==b'\x7fELF\x02' and h[1]==3 and h[2]==62
    entry,phoff,phsize,phnum=h[4],h[5],h[9],h[10]
    assert phsize==56
    executable=False
    for n in range(phnum):
        kind,flags,offset,vaddr,paddr,size,memsize,align=struct.unpack_from('<IIQQQQQQ',data,phoff+n*phsize)
        assert kind!=3,'Unexpected ELF interpreter'
        if kind==1:
            assert offset+size<=len(data)
            if flags&1 and vaddr<=entry<vaddr+memsize:executable=True
    assert executable,'Entry point must be executable'
    dynamic=subprocess.check_output(['readelf','-d',str(path)],text=True)
    assert 'libc.so.6' not in dynamic and 'GLIBC_' not in data.decode('latin1')
    if shared:assert '(PIE)' not in dynamic
    return data
controller_path=ROOT/'dist/App_Title_v0.2.15.elf'
controller=check(controller_path,False)
shared=check(ROOT/'build/App_Title_ShellUI.elf',True)
def needed(path):
    dynamic=subprocess.check_output(['readelf','-d',str(path)],text=True)
    return re.findall(r'\(NEEDED\).*?\[([^\]]+)\]',dynamic)
# These dependencies are opened by the SDK before main/logs/HTTP. RegMgr
# belongs only to the shared object injected into the existing ShellUI.
assert set(needed(controller_path))=={'libkernel_sys.sprx','libSceLibcInternal.sprx','libSceNet.sprx'},'Unexpected controller pre-main dependency'
assert {'libSceRegMgr.sprx','libmonosgen-2.0.sprx'}<=set(needed(ROOT/'build/App_Title_ShellUI.elf'))
controller_symbols=subprocess.check_output(['readelf','--dyn-syms',str(controller_path)],text=True)
assert 'sceRegMgrGetInt' not in controller_symbols and 'mono_runtime_invoke' not in controller_symbols
assert shared in controller,'Controller does not embed the exact shared object'
assert b'/data/AppTitle/AppTitle.log' in controller and b'/data/AppTitle/AppTitle_launch.log' in controller
assert b'/data/AppTitle/AppTitle_shellui.log' in shared
assert b'input_format=%u buffer=%u payload=%u modules=%u startup=%u failure=%u failed_module=%u module_size=%u' in controller
assert b'input_format=%u buffer=%u payload=%u modules=%u startup=%u failure=%u failed_module=%u module_size=%u' in shared
assert b'/data/AppTitle/AppTitle_Style_Probe.log' in controller
assert b'/api/export' in controller and b'Settings HTTP worker entered' in controller
assert b'/data/AppTitle.log\0' not in controller and b'/data/AppTitle_launch.log\0' not in controller
assert b'/data/AppTitle_shellui.log\0' not in shared
shared_symbols=subprocess.check_output(['readelf','-Ws',str(ROOT/'build/App_Title_ShellUI.elf')],text=True)
assert 'registry_trampoline' in shared_symbols and 'update_trampoline' in shared_symbols
assert 'ioctl_trampoline' in shared_symbols and 'patch_home_style' in shared_symbols
assert 'sha1_digest' in shared_symbols and 'build_title_style' in shared_symbols
assert 'open_trampoline' not in shared_symbols and 'stat_trampoline' not in shared_symbols
assert 'sceRegMgrGetInt' in shared_symbols and 'mono_runtime_invoke' in shared_symbols
assert 'mono_thread_attach' in shared_symbols and 'mono_thread_detach' in shared_symbols
assert b'sceAppInstUtilTerminate' in controller and b'sceUserServiceTerminate' in controller
assert bytes.fromhex('16c90278') in shared,'etaHEN title ID key missing'
assert b'NPXS40002' in shared and b'ReloadApp' in shared
symbols=subprocess.check_output(['objdump','-t',str(controller_path)],text=True)
addresses={line.split()[-1]:int(line.split()[0],16) for line in symbols.splitlines() if line.split() and line.split()[-1] in ['apptitle_stable_stager','apptitle_stable_stager_end']}
assert 0<addresses['apptitle_stable_stager_end']-addresses['apptitle_stable_stager']<1024
assert 'stbi_load' not in symbols,'PNG renderer linked into native controller'
print('PASS: minimal controller dependencies, RegMgr/Mono confined to shared object, native IDs, bounded HomeUI style module and exact embedding')

