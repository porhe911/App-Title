# Changelog

## 0.2.11 — 4.03 runtime-buffer test build

- Accept the exact known 4.03 RAM source when its declared length includes transport padding, bounded to the next 16 KiB allocation boundary. Keep the full source digest and every module guard; never write the extra tail.
- Recognize the same exact source as a raw decrypted RAM payload, in addition to an RNPS envelope.
- Add bounded input metadata to the existing startup status/log: format, buffer/payload sizes, RAM module/startup counts and refusal reason. No continuous text log or automatic dump is added.
- Preserve all twelve 4.03 style outputs, existing colour/placement settings and the resource cleanup from 0.2.10.

The old code's rejection of identical padded/raw source has been reproduced locally. The supplied 4.03 log proves successful native activation and a style refusal, but does not identify its input dimensions. Console confirmation of this candidate is still required. The delayed game/FTP report remains unverified.

## 0.2.10 — resource-lifecycle test build

- Detach the owned native ShellUI worker from Mono after metadata/JIT bootstrap, including every failure path. Managed home refreshes remain on the existing UI thread.
- Close each successfully initialized AppInstUtil session before restoring the installer identity, then close the owned UserService session. Do not terminate sessions whose initialization failed or reported already initialized.
- Resolve matching cleanup exports before initialization; missing cleanup support does not open a new session. Stop startup if explicit service cleanup or identity restoration fails.
- Preserve the original callback's errno across style processing and managed home refreshes.
- Retain 4.03 and 11.60 layout profiles, saved settings, Media shortcut and bounded logs.

This addresses resource-lifecycle issues identified during investigation. It is not a confirmed fix for the reported delayed game-launch or FTP failures; no console soak test is claimed.

## 0.2.9 — 4.03 test build

- Add the exact 4.03 HomeUI JavaScript Indexed RAM bundle profile alongside the existing 11.60 Hermes profile.
- Preserve top/bottom, inside/outside, preset/custom colour settings and white/bottom/outside defaults.
- Keep RAM module offsets, table lengths, startup code, launch handlers and the outer RNPS header/trailer unchanged.
- Recognize RAM bundles in the manually requested diagnostic export.
- Add local fixture, JavaScript execution and layout checks. No console validation of 0.2.9 is claimed.
- Preserve the eight 11.60 HomeUI patch outputs byte for byte.

This build does not claim to fix the separately reported launch failure after several hours.

## 0.2.8

Initial public open-source release.

- Standalone ELF with title IDs for games and applications.
- Above/below image placement with reserved space outside the artwork.
- Optional inside-image placement.
- Eight preset text colours and custom HEX colours.
- Persistent Media settings shortcut with D-pad, Cross and OPTIONS controls.
- Saved settings and autoload support.
- White IDs below the image by default.
- Native game-title and platform row moved below the ID row to prevent overlap.
- Bounded startup logs without continuous text logging.
- Verified HomeUI profile on firmware 11.60.

The outside-ID layout was confirmed working in 0.2.7. Version 0.2.8 keeps that layout and adjusts the native game-title row.

