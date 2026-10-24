# App Title

Standalone PS5 ELF payload that displays game and application title IDs on the home screen, with a persistent settings shortcut in **Media**.

**App Title 0.2.15:** the owner confirmed this ELF working on 4.03 on 2026-10-24. Includes live RAM rendering, resource cleanup, bounded logs, manual diagnostics and estate737's exact 13.60 profile from [PR #3](https://github.com/porhe911/App-Title/pull/3). The distributed ELF is the same file used for that console test; 13.60 verification remains attributed to the contributor.

[Download v0.2.15](https://github.com/porhe911/App-Title/releases/tag/v0.2.15) · [Русский README](README_RU.md) · [Build instructions](BUILDING.md)

## Preview

![App Title showing white title IDs below PS5 game and application icons](docs/images/app-title-preview.jpg)

Title IDs appear below the icons, outside the artwork, with white text by default.

## Features

- Title IDs such as `CUSA00001` and `PPSA03846`.
- Labels above or below the application image, with space reserved outside the artwork.
- Optional inside-image placement.
- Eight preset colours and custom six-digit HEX colours.
- White labels below the image by default.
- Settings saved under `/data/AppTitle/`.
- Permanent **Media → App Title** settings shortcut.
- Standalone ELF; etaHEN is not required.
- Autoload support after a reboot.

## Compatibility

Supported profiles:

| Firmware | HomeUI format | Validation |
| --- | --- | --- |
| 4.03 | `rnps-home_v2_ppr_releases_04.00`, Indexed RAM bundle | Owner confirmed 0.2.15 working after the requested outside/position/colour check, 2026-10-24 |
| 11.60 | `rnps-home 11.1.0+21887 / PS5 11.20 HOMEUI`, Hermes v89 | User-tested 0.2.8 baseline; original Hermes generation/patching logic retained |
| 13.60 | `rnps-home 13.0.0+22834`, Hermes v89 | Reported console-tested by estate737 for PR #3; this combined ELF awaits independent console verification |

Other firmware/HomeUI profiles have not been verified. Hermes retains its complete payload digest. RAM requires its exact startup, validated live table and per-edit source guards described in [BUILDING.md](BUILDING.md). Failed guards leave the entire buffer unchanged. New Media-shortcut registration on firmware 12+ is not implemented.

On 13.60, configure the payload using FTP before launch; see [FW13.60.md](FW13.60.md). Adding the display profile does not add Media registration on firmware 12+.

Requires a working jailbreak and a compatible privileged ELF loader. The payload does not provide a jailbreak.

## Installation

1. Download `App_Title_v0.2.15.elf` from [Releases](https://github.com/porhe911/App-Title/releases/tag/v0.2.15), or build it from this source. The [published 0.2.8 release](https://github.com/porhe911/App-Title/releases/tag/v0.2.8) does not include the 4.03/13.60 profiles.
2. Remove the previous App Title entry from autoload, or replace its file with the new ELF.
3. Fully reboot the console to clear any previous resident version.
4. Run your jailbreak, then load the ELF once or add it to your loader's autoload list.
5. Wait about 20–30 seconds, then open **Media → App Title** on 4.03/11.60. On 13.60 use the configuration file described above.

Choose the placement, position and colour, then select **Save** or press **OPTIONS**. The home scene refreshes to apply the settings. The settings interface is currently in Russian and supports D-pad / Cross navigation.

The Media shortcut remains installed after a reboot. Displaying IDs and opening its settings require the ELF to be running again; autoload handles this. Do not load multiple App Title versions in the same boot session. If ShellUI restarts, fully reboot before loading App Title again.

## Configuration and logs

Files are stored in `/data/AppTitle/`. The configuration is saved automatically by the settings page:

```ini
enabled=1
position=bottom
color=ffffff
placement=outside
```

`position` accepts `top` or `bottom`; `placement` accepts `outside` or `inside`. Old three-field configuration files remain supported.

`AppTitle.log` is limited to 8 KiB and replaces the last HomeUI result after a save; the bounded ShellUI result is refreshed on settings/outcome changes. There is no continuous text logging. For an outside-mode failure on 4.03, explicitly request diagnostic export and return home for five seconds. Enabled RAM sessions export actual pre-edit source to `/data/AppTitle/HomeUI_live.bin`; legacy export remains available for other sessions. Settings report the actual path. Do not publish system dumps in the repository.

## Source and build

Application code is **GPL-3.0-or-later**. All payload sources, vendored dependencies, the settings page and build scripts are included. See [BUILDING.md](BUILDING.md), [DEPENDENCIES.json](DEPENDENCIES.json) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Build using PS5 Payload SDK **v0.41**, Clang/LLD 18 and `make`. Only the final standalone ELF is installed on the console; its ShellUI component is embedded automatically. Sony HomeUI files, fonts, firmware dumps and SDK binaries are not included. Changes are made in memory; application icons and system files are not overwritten.

## Credits

Developed by **porhe911**. Thanks to the authors of [PS5-app_title](https://github.com/MasterPS0/PS5-app_title), [etaHEN](https://github.com/etaHEN/etaHEN), [Common FPS for PS5](https://github.com/porhe911/Common-FPS-for-PS5), [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk), [shsrv](https://github.com/ps5-payload-dev/shsrv) and [ShadowMountPlus](https://github.com/drakmor/ShadowMountPlus). Exact source revisions, adaptations and third-party licences are documented in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

The 13.60 HomeUI profile and emitter adaptations were contributed by [estate737](https://github.com/estate737/App-Title/tree/fw13.60-support), commit `613df61a39ecc8cf0f57df00ff63aad8646d2c6a`.

