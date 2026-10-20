#!/usr/bin/env python3
"""Package only release files and source; exclude local tools and test outputs."""
from pathlib import Path
import re
import shutil
import zipfile

ROOT=Path(__file__).resolve().parent.parent
OUT=ROOT.parent/'deliverables'
OUT.mkdir(exist_ok=True)
version=re.search(r'kVersion\[\] = "([^"]+)"',(ROOT/'include/app_title.hpp').read_text()).group(1)
name=f'App_Title_v{version}.elf'
elf=OUT/name
shutil.copyfile(ROOT/'dist'/name,elf)
files={name:elf.read_bytes()}
reports=sorted(ROOT.glob(f'VALIDATION_v{version}_*.md'))
assert reports,'Validate this version before packaging'
report=reports[-1].name
for doc in ['CHANGELOG.md',report,'LICENSE','LICENSE-HDE64.txt','THIRD_PARTY_NOTICES.md']:
    files[doc]=(ROOT/doc).read_bytes()
files['README_RU.md']=(f'# App Title {version}\n\n'
    f'Тестовая версия с исправлением обработки raw/padded RAM для 4.03; полная проверка исходного пакета и профиль 11.60 сохранены. На консоль нужен только {name}.\n\n'
    'Замени прежний ELF в autoload и полностью перезагрузи PS5. Выполни jailbreak, загрузи ELF один раз и подожди 20–30 секунд.\n\n'
    'Открой «Мультимедиа → App Title». Для первой проверки выбери «Белые ID под иконкой» и нажми «Сохранить» или OPTIONS.\n\n'
    'Исходники, лицензии и инструкция сборки: [source/README_RU.md](source/README_RU.md).\n'
    'Проверки исходного и raw/padded HomeUI пройдены. Успех на консоли 4.03 пока не подтверждён. Короткий AppTitle.log содержит параметры фактического буфера; постоянного журнала и автоматических дампов нет.\n').encode()
for doc in ['Makefile','BUILDING.md','README.md','README_RU.md','CHANGELOG.md','.gitignore',report,'VALIDATION_v0.2.10_20261020.md','VALIDATION_v0.2.9_20261020.md','VALIDATION_v0.2.8_20261019.md','DEPENDENCIES.json','LICENSE','LICENSE-HDE64.txt','THIRD_PARTY_NOTICES.md']:
    files['source/'+doc]=(ROOT/doc).read_bytes()
for directory in ['src','include','assets','tools','tests','vendor','docs']:
    for path in sorted((ROOT/directory).rglob('*')):
        if path.is_file() and '__pycache__' not in path.parts and path.suffix!='.pyc':
            files['source/'+path.relative_to(ROOT).as_posix()]=path.read_bytes()
archive=OUT/f'App_Title_v{version}.zip'
with zipfile.ZipFile(archive,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
    for path,data in sorted(files.items()):z.writestr(path,data)
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    assert z.read(name)==elf.read_bytes()
    assert set(z.namelist())==set(files)
    for path,data in files.items():assert z.read(path)==data
print(f'ELF: {elf.name}, {elf.stat().st_size} bytes')
print(f'ZIP: {archive.name}, {archive.stat().st_size} bytes, {len(files)} entries; contents verified')

