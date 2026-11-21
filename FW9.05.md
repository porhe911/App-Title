# App Title — PS5 firmware 9.05 (Hermes v89)

**Console verification (2026-11-21):** The owner reports that the exact **R3** standalone ELF shows Title IDs correctly on PS5 9.05, including the requested outside-icon placement. This verifies the reported working configuration, not independent testing of every layout combination, all games, or long-session stability.

- Release tag: `v0.2.15-fw905`
- File: `App_Title_FW905_R3.elf`
- SHA-256: `e010cb4ca281a17874756267e7eb7fde97578de88b8ae77a10a793c505b60d6d`
- Firmware reported by console: `0x09050005` (9.05)
- HomeUI: **Hermes v89**, **8,261 functions**, 1,591,008-byte verified bytecode.
- Input diagnostic SHA-256: `0929015521f86885a238778a75dc6aef4d2b89add1596dcfc10b0977f2ff4cd4` (**NOT** distributed).

The profile uses exact source, complete-bytecode, individual-function and header guards. Unrecognized or altered HomeUI bytecode is rejected without a partial write. Firmware dumps, decrypted bytecode and SDK binaries are not included in the repository or releases.

## Fix history

The first test ELF compiled correctly but failed to render. After live report, two register-liveness issues were corrected:

1. Hermes function #253: the style object `experienceTitle` remains in R8; the added height constant is moved to R10.
2. Hermes function #5610: `loadComponents` remains in R8; the added child array is allocated in R19.

The third (R3) ELF passed SDK v0.41 / Clang-LLD 18 compilation and `tools/verify_elf.py`; its exact SHA-256 is enforced by release CI. The owner then reported the R3 ELF working correctly on the console. These observations must not be extended to untested HomeUI revisions or long-duration stability.

## Installation on 9.05

1. Download `App_Title_FW905_R3.elf` from the [FW9.05 release](https://github.com/porhe911/App-Title/releases/tag/v0.2.15-fw905).
2. Disable autoload of earlier App-Title ELF versions and fully reboot PS5.
3. After jailbreak, launch the new ELF exactly once via a compatible payload loader.
4. Open **Media → App Title**, select **Outside icon / Below icon**, save and return home.
5. Optionally verify other positions, colours, launching a game and returning home; these extra combinations are not all individually owner-certified.

If anything fails, collect `/data/AppTitle/AppTitle.log` and `/data/AppTitle/AppTitle_shellui.log` and restore the previous ELF only **after a full reboot**. Do not run multiple resident versions in one boot session.

## Version scope

The release is intentionally named `0.2.15-fw905`, because the binary retains the internal `0.2.15` version string of the **exact user-tested R3 ELF**. This prevents silently changing accepted binary bytes just to relabel the version. Existing profiles (4.03, 11.60 and 13.60) are not changed by this patch. New Media shortcut registration on 12+ is still outside scope.
