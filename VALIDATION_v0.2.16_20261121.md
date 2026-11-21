# App Title v0.2.16 release acceptance — 2026-11-21

## Source provenance
- All previously supported 4.03 RAM, 11.60 Hermes and 13.60 Hermes code is unchanged by this release, except that `home_style_fw905.inc` is included for the 9.05 profile.
- FW9.05 R3 was user-confirmed functioning correctly on PS5 9.05 on 2026-11-21.
- The accepted R3 ELF checksum is recorded separately in `VALIDATION_FW905_20261121.md`; the renamed and internally version-bumped v0.2.16 ELF is **not** byte-identical to the owner-tested R3.
- The version is synchronized across runtime kVersion, startup notifications, Makefile target, ELF verifier, JSON HTTP test and package output.

## Release requirements
The GitHub Actions workflow must pass, on the release branch and merged main:
1. PS5 Payload SDK v0.41 / Clang 18 build.
2. `tools/verify_elf.py`: exact embedded ShellUI and DT_NEEDED guard.
3. Host test targets for core, media, hooks, ShellUI, runtime, style, export, Hermes HomeUI and RAM.
4. Packaging integrity and exact SHA-256 check against pinned v0.2.16 binary.
5. Publish ELF, source ZIP, SHA256SUMS for v0.2.16 and confirm their availability before retiring the temporary FW905-only release.

The GitHub Actions runs are the source for actual results; do not treat these requirements as already performed until the workflow succeeds.

## Runtime scope
PS5 9.05 has user confirmation for R3's visual behavior only. Version 0.2.16 changes internal version text and filename; console runtime confirmation of that newly named binary is still pending. No exhaustive all-firmware regression, firmware 9.00 support or long-session stability is claimed.
