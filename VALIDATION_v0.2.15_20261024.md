# App Title 0.2.15 release acceptance — 2026-10-24

The owner confirmed the supplied 0.2.15 ELF working on 4.03 and approved
accepting the pull requests and publishing it. This acceptance follows the
requested outside/position/colour console check. It is a user-reported result,
not a new independent measurement or a long-session game/FTP test.

The ELF is unchanged from integration commit
`6e712df8ac1a3a8bfa09fb2a597b88bcb2422dcd`. Release preparation changes
documentation and packaging only. The original integration report is retained
in `VALIDATION_v0.2.15_20261023.md`.

## Compatibility status

| Profile | Console evidence |
| --- | --- |
| 4.03 RAM | Combined 0.2.15 confirmed working by the owner on 2026-10-24 |
| 11.60 Hermes | Previously confirmed 0.2.8 baseline; all eight complete generated HBC results remain byte-identical |
| 13.600.007 Hermes | Original PR #3 console success reported by estate737; separate combined-ELF confirmation remains pending |

Media registration on 12+ remains unavailable. The separate delayed game/FTP
report has no demonstrated long-run resolution. No universal firmware support
is claimed.

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

## Release files

- `App_Title_v0.2.15.elf`: the exact console-tested standalone payload.
- `App_Title_v0.2.15.zip`: the payload, source, licenses, build instructions,
  historical validation reports and 13.60 configuration.
- No checksum manifest or private firmware/tooling input is included.
