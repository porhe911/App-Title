# Sources and licenses

App Title application code: Copyright (C) 2026 porhe911,
GPL-3.0-or-later. See `LICENSE`.

The 13.60 profile metadata and emitter adaptations in `src/home_style.cpp` and
`src/home_style_1360.inc` are contributed by estate737, from App Title PR #3,
commit `613df61a39ecc8cf0f57df00ff63aad8646d2c6a` (GPL-3.0-or-later).
Its console verification is reported by that contributor; integration tests
and their limitations are recorded in this version's validation report.

- The project idea and local appmeta location were inspected in
  [MasterPS0/PS5-app_title](https://github.com/MasterPS0/PS5-app_title), commit
  `4cbd5a783bec46d7d8bb7f0eb5621a8bd32667ec`. Its `main.c` was not copied.
  Only the separately licensed stb headers were copied from that repository.
- `vendor/stb`: stb_image 2.30, stb_image_write and stb_truetype by Sean Barrett
  and contributors. Each header carries its MIT/public-domain license options.
- `src/shellui.cpp`: native Title ID behavior adapted from
  [etaHEN/etaHEN](https://github.com/etaHEN/etaHEN), commit
  `dafa13b562ddb137a4b4a97b9aaa287c0c57cc9c`,
  Copyright (C) 2025 etaHEN / LightningMods, GPL-3.0-or-later.
  The isolated registry override and home reload were independently confirmed
  in the user-provided etaHEN 2.5B binary. No etaHEN binary is redistributed.
- `vendor/loader`: adapted from
  [porhe911/Common-FPS-for-PS5](https://github.com/porhe911/Common-FPS-for-PS5),
  commit `15018e794cfd3f6d793de5c5d51e701a80e8289a`, GPL-3.0-or-later.
  Namespace, symbols and section names were changed for isolation. Original
  author notices are retained. The original FPS renderer and sampler are not used; Application.Update is used only to schedule the home refresh on the UI thread.
- `vendor/shsrv`: pt/ELF loader primitives from
  [ps5-payload-dev/shsrv](https://github.com/ps5-payload-dev/shsrv), v0.20,
  commit `6f320637d56d344a0e7797753099e33238bbf146`, GPL-3.0-or-later.
  `pt.c`/`pt.h` add two wrappers for stopped-thread enumeration.
- `src/media_launcher.cpp`: adapted from Common FPS at the above revision;
  registration reference is
  [drakmor/ShadowMountPlus](https://github.com/drakmor/ShadowMountPlus), commit
  `98c524890176c7b474fcb03387bec88c4a559861`, GPLv3.
- `vendor/hde`: Hacker Disassembler Engine 64, Copyright (c) 2008-2009
  Vyacheslav Patkov. BSD 2-clause terms are reproduced in `LICENSE-HDE64.txt`.
  Decoder/header/table were taken from etaHEN commit
  `dafa13b562ddb137a4b4a97b9aaa287c0c57cc9c`; unrelated INI-parser code was removed.
- The controller links the GPLv3+ runtime from
  [ps5-payload-dev/sdk](https://github.com/ps5-payload-dev/sdk), release v0.41.
  SDK and compiler binaries are not part of the source archive.

The legacy PNG renderer remains in source-only host checks; it is not linked into the native controller. No Sony fonts are distributed. The settings
icon was drawn specifically for this project; it contains no third-party artwork.

## RNPS diagnostic ABI
The manual HomeUI export uses public device ABI values documented by zecoxao/decrypt_rnps, commit 5ad3e75156476464e6ff00b12db54c93566a99dd. No source or headers from that repository are bundled; the implementation is independently written.


## HomeUI style profile and external verification

`src/home_style.cpp` is an independently written, guarded in-memory bytecode
emitter. Its opcode/format definitions were verified against Facebook Hermes
v0.12.0 (MIT) and its generated instructions were executed with the official
Hermes v0.12.0 Linux CLI. React Native v0.70.6 processColor/normalizeColor (MIT)
was consulted for unsigned RGBA colour values. These external sources and
binaries are not bundled or linked into the payload.

P1sec/hermes-dec (AGPL-3.0) was used as an external disassembler to inspect the
user-provided input and the result. No code from it is included in this project.
The user-provided HomeUI, literal data and original function bytes are not
redistributed. Only profile hashes, numeric IDs/offsets and independently
written replacement instructions are present. The RNPS post-decrypt hook
approach also follows the GPL etaHEN reference already credited above.


The host layout validation also uses the external Meta Yoga 2.0.1 package
(MIT), which is not bundled or linked into the PS5 payload. Layout semantics
were checked against the official React Native layout-props documentation.
