# App Title — FW 9.05 support (0.2.15-fw905)

**User-confirmed working:** PS5 firmware **9.05**, HomeUI Hermes v89, fixed game/application Title IDs shown **outside icon artwork**. Tested R3 ELF accepted by the project owner on 2026-11-21.

**Download:** `App_Title_FW905_R3.elf` (standalone PS5 payload, no separate ShellUI binary)

SHA-256: `e010cb4ca281a17874756267e7eb7fde97578de88b8ae77a10a793c505b60d6d`

Fixes:
- Exact verified 9.05 HomeUI adapter, including outside/inside placement, position and colour handling.
- R8 register bugs from the first attempted test ELF eliminated: live style object and loadComponents preserved by using R10 and R19.
- Existing 4.03, 11.60, 13.60 matching profiles preserved.
- Private HomeUI binaries/dumps are not packaged.

**Important:** turn off autoload of older App-Title payloads and **fully reboot** before running the new ELF. In Media → App Title choose Outside / Below, then save. Do not run multiple resident versions simultaneously.

Only the verified 9.05 HomeUI variant is claimed supported by this new profile; FW9.00, unknown HomeUI revisions, all UI permutations and long-session stability are not certified. The internal version string intentionally remains 0.2.15 for byte-for-byte identity with the **exact console-tested** executable. For other firmware or the historical release, see [v0.2.15](https://github.com/porhe911/App-Title/releases/tag/v0.2.15).

Build validation: PS5 Payload SDK v0.41, Clang/LLD 18, `tools/verify_elf.py` and exact ELF checksum are enforced in release CI.

[Detailed firmware notes](https://github.com/porhe911/App-Title/blob/main/FW9.05.md) · [Acceptance record](https://github.com/porhe911/App-Title/blob/main/VALIDATION_FW905_20261121.md)
