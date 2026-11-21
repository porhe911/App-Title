# PS5 FW9.05 — R3 console acceptance (2026-11-21)

## Evidence and status

- Original HomeUI evidence: `HomeUI_decrypted.bin` from console, SHA-256 `0929015521f86885a238778a75dc6aef4d2b89add1596dcfc10b0977f2ff4cd4`. **Private; not committed or published.**
- Hermes v89 HomeUI profile: expected source hash and payload digest, 8,261 functions; six target functions #1390, #5610, #5408, #5404, #253 and #972.
- The original outside-ID attempt returned `style_state=3` (unsupported), `patches=0`.
- First generated FW9.05 ELF compiled but failed runtime display. Analysis found R8 overwritten in #253 and #5610.
- R3 fixes both overwrites by using R10 and R19; owner explicitly reported: **«вот теперь работает корректно, можно публиковать»** on 2026-11-21.
- R3 build GitHub Actions: <https://github.com/porhe911/App-Title/actions/runs/44899221042> — PS5 Payload SDK v0.41, clang/LLD 18 and ELF verification PASS.
- Console-tested executable: `App_Title_FW905_R3_TEST.elf`, 299,400 bytes, SHA-256 `e010cb4ca281a17874756267e7eb7fde97578de88b8ae77a10a793c505b60d6d`.
- Historical short-lived public name: `App_Title_FW905_R3.elf` (exact same bytes). Replaced by the general v0.2.16 publication. The v0.2.16 binary is rebuilt with a new internal version and is **not** the identical R3 binary.

## What the acceptance does NOT claim

It does not represent independent QA, every menu/position/color combination, long-session or game-launch stability, or FW9.00 compatibility. It is specific to stock 9.05 HomeUI matching the exact profile. Previously reported FW 4.03, 11.60 and 13.60 results retain their original independent provenance.

## Safety and provenance

- Patch is applied to an already decrypted in-memory HomeUI. It never alters files on Sony system storage.
- Target function headers and SHA-1s, whole original source hash and bytecode digest must match before any write.
- Distinct 9.05 dispatch is checked **before** 11.60/13.60 profiles. All other profiles retain their prior code.
- Original HomeUI dumps, private firmware code, user logs and any full firmware input do not enter version control.
- A forced restart is required between resident ELF versions.
