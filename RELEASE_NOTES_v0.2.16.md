# App Title v0.2.16

A single standalone PS5 ELF with HomeUI profile support for **4.03, 9.05, 11.60 and 13.60**.

## Changes

- Added PS5 **9.05** Hermes v89 HomeUI support and corrected outside-icon Title ID placement.
- Fixed the 9.05 register conflicts in the style and Tile renderer (preserve R8; use spare R10/R19).
- Preserved existing exact HomeUI profiles, Media/FTP settings, colours, ID position and original image handling.
- Standardized the version and filename: `App_Title_v0.2.16.elf` with internal `0.2.16` and consistent startup notifications.
- Kept precise per-profile checks; no Sony firmware or decrypted HomeUI files are shipped.

**Console verification:** the 9.05 R3 candidate was confirmed working by the project owner on 2026-11-21. The normal v0.2.16 rebuild is mechanically based on that fix but has a different binary checksum due to version changes. Its build, ELF verification, tests and checksums are independently validated; it has not yet been rechecked on-console as v0.2.16.

## Install

Replace the existing payload/autoload entry with `App_Title_v0.2.16.elf`, **fully reboot**, jailbreak and launch the new ELF once. On supported firmware with the Media shortcut, choose **Outside icon → Below icon**, then Save. Never load two resident versions in one boot.

## Legacy and limitations

Original `v0.2.15` remains available for rollback. The temporary firmware-specific `v0.2.15-fw905` release is retired in favour of the regular versioning scheme.

Other firmware/HomeUI revisions are not claimed supported. Firmware 12+ Media shortcut registration remains unavailable. Long-duration stability and every screen/colour combination are not certified.

[English README](https://github.com/porhe911/App-Title/blob/main/README.md) · [Русский README](https://github.com/porhe911/App-Title/blob/main/README_RU.md) · [FW9.05 notes](https://github.com/porhe911/App-Title/blob/main/FW9.05.md)
