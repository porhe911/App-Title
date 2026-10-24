# App Title 0.2.15 integration validation — 2026-10-23

This is a test candidate, not a console-confirmed stable release.

## Source scope

- Base: 0.2.14, commit `04b1907f8a10e3f1dca6bf31409b2475d823f6f9`.
- 13.60 contribution: estate737 PR #3, commit `613df61a39ecc8cf0f57df00ff63aad8646d2c6a`. The integration preserves it as a second commit parent and credits the contributor.
- `home_ram.cpp`, `ram_js.hpp`, `live_ram_export.hpp`, `shellui.cpp`, `media_launcher.cpp`, `core.cpp` and `hooks.cpp` are source-identical to 0.2.14. Existing raw/padded RAM handling, worker/service cleanup and bounded/manual diagnostics are preserved.
- The Hermes emitter/profile selection is retained from PR #3. The public host-verification helper can select either profile; the external-fixture test uses the corresponding independent write allowlist.
- `FW13.60.md` is included at both package levels. No checksum manifest, firmware bundle, user log, SDK or external compiler is packaged.

## Checks passed

| Check | Result |
| --- | --- |
| PS5 build, SDK v0.41 / Clang and LLD 18.1.3 | PASS; no warnings/errors; ELF verification and exact shared/UI embedding pass |
| Core, Media, hooks, ShellUI, runtime/logs, probe, export | PASS; includes matched Mono attach/detach and service session ownership/error paths |
| Real settings HTTP and DOM | PASS; palette/custom HEX, top/bottom, inside/outside, Cross/OPTIONS, persistence and request checks |
| Hermes generators without firmware files | PASS; 24 style cases across both profiles, black/white/custom RGBA, exact/short capacity handling |
| Complete 11.60 fixture | PASS; eight rewrites, allowlisted byte changes, footer integrity and 15 corrupt-input refusals |
| Legacy output comparison | PASS; eight complete 11.60 HBC outputs and twelve complete 4.03 RAM outputs are byte-identical to 0.2.14 |
| PR #3 emitter comparison | PASS; all 24 generated legacy/modern style snippets are byte-identical to the reviewed contribution |
| 4.03 native RAM suite | PASS; 12 styles, 60 raw/padded views, 36 relocated variants, 96 live-component envelope/raw cases, malformed/ambiguous source refusals |
| 4.03 JS execution/layout | PASS; reference and renamed-binding/extra-image-prop fixtures each pass 10,296 module parses, 96 Tile executions, 36 geometry and 36 Yoga cases |
| 4.03 registry selector | PASS; actual modules/async helpers, native Registry and Recoil factories mocked |
| Official Hermes v0.12.0 VM, 11.60 fixture | PASS; eight style modes, both Tile branches twice per mode, 96 geometry/animation/row/title cases |
| Yoga, 11.60 captured VM outputs | PASS; positive bounds, clipping/selected-scale and bitmap separation models |
| ASan and UBSan | PASS; complete ShellUI/RAM/manual-export lifecycle and profile-generator/11.60 fixture tests, isolated host working directory |

Private firmware inputs remain outside the source tree and package. Tests of
mocked APIs and host Yoga do not simulate PS5 WebKit, native fonts, GPU clipping,
GC scheduling or a long console session.

## Pending console and firmware checks

- The combined 0.2.15 ELF has not run on a console during this integration.
- On 4.03, inside/bottom/white was confirmed by the user for 0.2.13. The unchanged 0.2.14 outside renderer and live-export workflow still await console confirmation. This candidate does not claim a new outside-rendering fix.
- On 11.60, the user-confirmed release baseline remains 0.2.8; byte equality and VM tests support regression confidence, not a new console result.
- On 13.60, estate737 reports console success for PR #3. No complete 13.60 fixture was available here. New-profile generator checks and faithful integration do not independently verify the source string/literal tables or every copied function. Run `make home-style-test HOME_UI=/private/path/13.60/HomeUI_decrypted.bin` when that fixture is available, and visually test this combined ELF.
- Media registration on firmware 12+ is not implemented. Configure 13.60 over FTP before starting the ELF; the settings listener remains localhost-only.
- The separate delayed game-launch/FTP report has no demonstrated root cause or long-run fix in this candidate. No kernel, FakePkg, FTP or game-launch patches were added.

## First console check

Replace the previous autoload entry with only `App_Title_v0.2.15.elf`, fully
reboot, run jailbreak and load once. On 4.03/11.60 wait 20–30 seconds, open
Media → App Title, select white/below/outside, save and return home. Check both
position choices and a colour change. If 4.03 outside mode fails, after saving
and waiting ten seconds use the manual diagnostic button, return home for five
seconds and retain `HomeUI_live.bin` plus the short `AppTitle.log` privately.
On 13.60 follow `FW13.60.md` and change the file before loading the ELF.
