# Build App Title 0.2.15

Requires Linux/POSIX, LLVM/Clang 18, LLD 18, make, Python 3 and PS5 Payload SDK **v0.41**.

Download the SDK from its official release:

https://github.com/ps5-payload-dev/sdk/releases/tag/v0.41

Extract the SDK outside this source directory, then run:

```bash
export PS5_PAYLOAD_SDK=/absolute/path/ps5-payload-sdk
make -j4
```

The only file needed on the console is `dist/App_Title_v0.2.15.elf`.
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

The private controller/shared status ABI remains 112 bytes in 0.2.15.
Both pieces must be built together. Bounded startup metadata includes the
refused module ID and size; no automatic source dump is saved. Fully reboot
to clear an old resident component before loading a new version.

## Live component regression (0.2.15)

Outside mode keeps exact digests for modules 25, 102, 219 and 531. Modules
527 and 552 use bounded source edits in `src/ram_js.hpp`: the module wrapper,
require binding, style imports and actual React element/property structure
must agree. Unique Space styling is reordered; the existing conditional Text
is moved from the image mat into a positive-height View. Original image props,
callbacks and expressions are copied rather than rebuilt with fixed names.
Only literal debug source metadata is omitted from the reconstructed nodes.
All six changes are staged before any live RAM byte is written. Inside mode
still qualifies/edits only 219 after startup/table checks.

The extended suite covers 96 envelope/raw cases with shorter or longer slots,
renamed React/native/style bindings, unrelated arithmetic, and extra image
props/callbacks. It refuses duplicate/missing targets, fake calls inside strings
or regexps, wrong imports/styles/gates and excessive capacities without writing.
The reference outputs in all twelve modes remain byte-identical to the prior
baseline. Space launch/focus bytes are checked by undoing only the style reorder.

```sh
node tools/verify_ram_bundle.js build-host/ram_context_input_0.bin ram_context_0_style_
node tools/verify_ram_bundle.js build-host/ram_context_input_1.bin ram_context_1_style_
node tools/verify_ram_bundle.js build-host/ram_context_input_2.bin ram_context_2_style_
node tools/verify_ram_bundle.js build-host/ram_context_input_3.bin ram_context_3_style_
node tools/verify_ram_registry.js /private/path/HomeUI_decrypted.bin
```

The registry test executes actual modules 554/555 and their async helpers with
native Registry/Recoil factories mocked. Neither these checks nor Yoga geometry
are native-console display confirmation.

## Manual live RAM export and result logs

On enabled RAM sessions, the existing manual settings action sends a 24-byte
PID/serial-scoped request to the native component. The native file worker
allocates at most 4 MiB and queues a managed home refresh. After table/startup
validation, before any source edit, the hook copies RAM into the armed buffer.
It does no allocation, file IO or Mono work. The native worker writes
`/data/AppTitle/HomeUI_live.bin` and frees the buffer on success or timeout.
The controller polls completion without blocking its session loop. Outside
this explicit action the observer does not allocate/copy/write a dump.

Legacy/non-RAM export retains `HomeUI_decrypted.bin`. The API reports the actual
export path. Raw RAM exports are supported by both JS verification tools. With
a local 4.03 fixture, `make shellui-test HOME_UI=/private/path/HomeUI_decrypted.bin`
also checks that the full exported RAM equals the unmodified input, tests the
request/timeout path and leaves a private `build-host/ram_live_input.bin` for:

```sh
node tools/verify_ram_bundle.js build-host/ram_live_input.bin ram_context_0_style_
```

`AppTitle.log` replaces its final HomeUI row at startup and after a user save;
`AppTitle_shellui.log` replaces its bounded result when requested settings or
validation outcome changes. Neither text file is periodically rewritten.

## Combined Hermes profiles (0.2.15)

The 13.600.007 profile is integrated from estate737 PR #3, commit
`613df61a39ecc8cf0f57df00ff63aad8646d2c6a`. The runtime retains the
4.03 raw/RNPS RAM dispatch before checking either exact Hermes profile.
The 0.2.14 native worker, resource cleanup and RAM patcher are retained.

`make home-style-test` runs both 11.60/13.60 generators in 24 style cases
without firmware files, including exact/short capacities and unsigned RGBA.
With `HOME_UI=/private/path/HomeUI_decrypted.bin`, the test accepts an external
11.60 or 13.60 RNPS fixture, selects an independent per-profile write allowlist,
checks eight complete rewrites, the footer digest and 15 corrupt-input refusals.
13.60 outputs use `build-host/home_style_1360_<mode>.hbc`; 11.60 retains the
existing filenames consumed by the legacy VM/layout tools.

The existing `verify_*_vm.py` and legacy layout/parser tools are specific to
the 11.60 fixture. Do not report them as 13.60 VM tests. The integration run
has 4.03/11.60 fixtures; a complete 13.60 fixture and a run of this combined
ELF on 13.60 are pending. See `FW13.60.md` for the FTP-only setup on 12+.
