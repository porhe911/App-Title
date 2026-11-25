# PS5 9.05 HomeUI support

The general **App Title v0.2.17** ELF supports exact-match PS5 firmware 9.05 HomeUI (Hermes v89) alongside the existing 4.03, 11.60, 13.00 and 13.60 profiles.

## Evidence

- Console reported firmware code: `0x09050005` (9.05).
- Hermes v89, 8,261 functions, payload 1,591,008 bytes.
- Exact private diagnostic SHA-256: `0929015521f86885a238778a75dc6aef4d2b89add1596dcfc10b0977f2ff4cd4` (not distributed).
- Original outside-ID diagnostic: `style_state=3`, `patches=0` (unsupported profile).
- R3 fixes Hermes register-liveness issues: in function #253 keep `experienceTitle` in R8 using R10 for the height; in #5610 keep `loadComponents` in R8 using R19 for the children array.
- **The owner confirmed R3 works correctly on PS5 9.05 on 2026-11-21.** The historic tested R3 ELF hash is `e010cb4ca281a17874756267e7eb7fde97578de88b8ae77a10a793c505b60d6d`; the newer v0.2.16 binary differs by the corrected internal version and requires separate verification.

The private decrypted HomeUI and console logs are **not** distributed. Unknown HomeUI profiles are rejected without modification.

## Installation

Download `App_Title_v0.2.17.elf` from [v0.2.17](https://github.com/porhe911/App-Title/releases/tag/v0.2.17). Disable old autoload, fully reboot PS5, jailbreak, run only one App-Title ELF and select **Outside / Below** in **Media → App Title**.

On firmware 12+ automatic Media registration is not provided; configure via FTP, as described in [FW13.60.md](FW13.60.md).

The owner verified R3's reported 9.05 rendering outcome; this does not certify every colour/position, every game launch or long-running stability. In particular do not treat 9.00 as equivalent to 9.05.
