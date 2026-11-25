## Changes

- Added PS5 13.00 support using the shared 13.00/13.60 HomeUI profile.
- Fixed Title ID placement outside icons.
- Preserved inside placement and existing firmware profiles.

**Compatibility:** 4.03, 9.05, 11.60, 13.00 and 13.60 (matching HomeUI profiles).

The corrected 13.00 layout was confirmed on console by the project owner. The v0.2.17 rebuild is CI-tested; separate console testing of the rebuilt ELF and revised 13.60 layout has not yet been performed.

Replace the previous ELF, fully reboot, and launch once. On 12+ configure via FTP before launch: [FW13.60.md](https://github.com/porhe911/App-Title/blob/main/FW13.60.md).
