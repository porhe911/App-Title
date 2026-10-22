# Build App Title 0.2.13

Requires Linux/POSIX, LLVM/Clang 18, LLD 18, make, Python 3 and PS5 Payload SDK **v0.41**.

Download the SDK from its official release:

https://github.com/ps5-payload-dev/sdk/releases/tag/v0.41

Extract the SDK outside this source directory, then run:

```bash
export PS5_PAYLOAD_SDK=/absolute/path/ps5-payload-sdk
make -j4
```

The only file needed on the console is `dist/App_Title_v0.2.13.elf`.
The shared object in `build/` is automatically embedded into the controller.
No Actions job or external repository mutation is needed to build.

All required application sources, rendering headers, disassembler and adapted
loader sources are included. The SDK/compiler binaries are not included.

Host checks:

```bash
make host-test
make media-test
make hooks-test
make shellui-test
make runtime-test
make style-test
make export-test
make home-style-test
python3 -m pip install Pillow
npm install --prefix build-ui --no-save jsdom@26.1.0 yoga-layout@2.0.1
python3 tests/test_host.py
python3 tools/verify_elf.py
```

The legacy rendering check (source-only, not the native runtime) uses `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf` on
the host. The console uses its own SST font; Sony font files are not bundled.
The DOM tests exercise the actual HTML and HTTP service with jsdom and real
XHR. They do not simulate PS5 WebKit, GPU caches or native controller events.

The build uses `prospero-clang++` and links the SDK's payload runtime. Do not
replace the controller with a Linux-compiled executable, and do not send the
internal shared object as a second standalone payload.

Only the shared component links to the SDK stubs for libSceRegMgr and libmonosgen-2.0. The controller resolves their exports in the existing ShellUI process. Registry and Application.Update callbacks are required. An additional ioctl callback styles the verified HomeUI after successful RNPS decryption; unsupported style prologues preserve the required pair.

The controller must not have libSceRegMgr or libmonosgen in DT_NEEDED. The SDK processes this list before main. Run tools/verify_elf.py after linking; its dependency allowlist is a startup regression check.

The injected native worker attaches to Mono only for metadata/JIT bootstrap and
detaches before entering its file/status loop, including on bootstrap failure.
Managed refreshes execute on the already-managed Application.Update thread.
Media registration closes each successfully initialized AppInstUtil session
before restoring its auth ID, then closes its owned UserService session.
Failed/already-initialized sessions are never terminated by App Title.
The Media test exercises cleanup ordering, missing exports and failure paths;
the ShellUI test checks balanced Mono attachment and original errno preservation.

Optional fixture and real Hermes VM checks (Sony input and Hermes tools are external):

```bash
make home-style-test shellui-test HOME_UI=/absolute/path/HomeUI_decrypted.bin
python3 tools/verify_style_vm.py --hermes /absolute/path/hermes-v0.12.0/hermes
python3 tools/verify_tile_vm.py --hermes /absolute/path/hermes-v0.12.0/hermes
python3 tools/verify_geometry_vm.py --hermes /absolute/path/hermes-v0.12.0/hermes
python3 tools/verify_focus_vm.py --hermes /absolute/path/hermes-v0.12.0/hermes --home-ui /absolute/path/HomeUI_decrypted.bin
python3 tools/verify_render_bytecode.py --hermes-dec /absolute/path/hermes-dec
python3 tools/verify_inside_baseline.py
python3 tools/verify_v027_layout.py --home-ui /absolute/path/HomeUI_decrypted.bin
node tests/test_outside_layout.js
```

For the exact 4.03 JavaScript RAM bundle fixture:

```bash
make ram-style-test RAM_UI=/absolute/path/4.03/HomeUI_decrypted.bin
./build-host/test_shellui /absolute/path/4.03/HomeUI_decrypted.bin
node tools/verify_ram_bundle.js /absolute/path/4.03/HomeUI_decrypted.bin
```

The RAM test produces twelve local outputs covering inside/outside, top/bottom
and white/custom/black text. The verification script parses every module and
executes the actual patched style, geometry and Tile modules with mocked
native components. It also checks the title-ID visibility gate, both telemetry
branches, unchanged launch modules and positive label bounds with Yoga.
The source fixture and generated system modules must stay outside releases.

The fixture test writes eight HBC files: outside modes 0–3 and inside modes 4–7,
covering bottom/top and white/custom colour. Styles, both Tile renderer branches,
and geometry/animation/row/game-title builders execute in the official Hermes v0.12.0 CLI.
Native components, React hooks and animation values are mocked. Builder tests
cover three input sizes, 96 cases, twice each. The unchanged parent renderer is
compared with its original function in 64 scenarios. The independent parser
checks seven functions, register/branch boundaries, Identifier properties and
inline caches. No external parser implementation or Sony input is packaged.

The inside baseline check compares entire HBC SHA256 digests against outputs
built from the released 0.2.3 source with the same user fixture. Yoga uses actual
captured styles and trees to check positive text bounds, bitmap separation and
selected-scale bounds in the enlarged row. It models three text measurements,
both clip settings and four outside modes; it does not implement RNPS native
clipping, font rasterization or all scene animations. Host tests do not replace
visual validation on the console.

The 0.2.8 layout check restores only the original game-title function in each
outside fixture, then compares its complete HBC digest with released 0.2.7.
This verifies that working ID rendering is unchanged. Game-title VM checks
verify TITLE_Y and top=214 outside, top=106 inside, unchanged horizontal and
minimized offsets, and the original 62-point line container.


## RAM table/transport regression

`make ram-style-test RAM_UI=/private/path/HomeUI_decrypted.bin` exercises
12 styles, 60 raw/padded views and 36 variants with relocated module source.
The main variant removes redundant function-expression parentheses in 285
unrelated modules, preserving their function bodies, and reproduces the
observed live dimensions: 1,385,934 payload bytes and 1,389,168 RNPS bytes.
Its startup and six target-module sources remain exact.

The patcher qualifies the known 1,795-byte startup and full 858-entry table.
It rejects unordered/overlapping,
out-of-bounds, unterminated or internally NUL-containing module ranges.
Source addresses come from the live index. Unrelated source is preserved;
a whole-file source hash/length is intentionally no longer required for RAM.
Hermes retains its existing complete-payload guard and generation logic.

Independent JavaScript/geometry/layout checks for the compact fixture:

```sh
node tools/verify_ram_bundle.js build-host/ram_variant_input.bin ram_variant_style_
```

The private controller/shared status ABI remains 112 bytes in 0.2.13.
Both pieces must be built together. Bounded startup metadata includes the
refused module ID and size; no automatic source dump is saved. Fully reboot
to clear an old resident component before loading a new version.

## Live component regression (0.2.13)

The RAM test also creates a second compact fixture: 110 debug filename
literals are shortened by five bytes each and ten unrelated function arguments
lose redundant parentheses. It has the observed 1,385,934-byte payload,
1,389,168-byte buffer and 5,271-byte Space module (527). Tile's synthetic length
is 1,943 bytes; the console has not yet reported its actual length or source.
This construction models the failure parameters and does not claim to capture
the console's actual source differences.

Outside mode checks exact source digests for 25, 102, 219 and 531. Module 527
requires its wrapper and one unique style-property context outside a quoted
string/comment, then changes only the order of two existing array elements.
Module 552 requires an exact canonical digest: only its build filename, five
debug line numbers and redundant outer function parentheses are normalized.
The live source is not normalized. Every staged write uses its actual slot
length, with Space capped at 8 KiB and Tile at 4 KiB. Inside mode qualifies and
edits only module 219 after the common startup/table validation.

Seventy-two additional envelope/raw cases cover compact and growing target
slots, debug line values and optional parentheses. Missing/ambiguous/non-code
Space contexts, excessive slot sizes and executable Tile changes must refuse
without any live mutation. The Space source is restored by undoing its one
reorder and compared byte for byte, covering launch and focus handlers too.

Run execution/layout checks for the three generated context fixtures:

```sh
node tools/verify_ram_bundle.js build-host/ram_context_input_0.bin ram_context_0_style_
node tools/verify_ram_bundle.js build-host/ram_context_input_1.bin ram_context_1_style_
node tools/verify_ram_bundle.js build-host/ram_context_input_2.bin ram_context_2_style_
node tools/verify_ram_registry.js /private/path/HomeUI_decrypted.bin
```

The registry check executes actual modules 554/555 and their transpiler helpers,
with Recoil constructors and native Registry API mocked; it verifies the exact
isHomeTitleIdDisplayEnabled key, boolean/integer/null values and errors.
