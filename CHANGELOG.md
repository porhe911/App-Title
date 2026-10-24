# Changelog

## 0.2.15 — 2026-10-24

- Integrate estate737's exact 13.600.007 Hermes profile from PR #3 with the 0.2.14 RAM renderer and diagnostics. Preserve contribution history and attribution.
- Keep raw/padded RAM handling, live module-table lookup, inside-mode independence, manual live export, bounded last-save results and Mono/AppInstUtil/UserService cleanup.
- Exercise both Hermes generators in 24 colour/position/placement cases. Let the external-fixture test select the 11.60 or 13.60 write allowlist; a 13.60 fixture is still needed to validate the complete new-profile rewrite independently.
- Retain the existing tested Mono detach stub and lifecycle checks; restore a working combined `shellui-test`.
- Include `FW13.60.md` both beside the packaged README and in source. No checksum manifest is included.
- The owner confirmed the combined 0.2.15 ELF working on 4.03 on 2026-10-24 and approved publication. The tested ELF is distributed unchanged. 13.60 verification remains attributed to estate737; Media registration on 12+ is still unavailable. No long-run fix for the separate delayed game/FTP report is claimed.

## 0.2.14 — outside renderer and live diagnostics test build

- Record user confirmation of inside/bottom/white IDs on 4.03 in 0.2.13. Its supplied startup result is successful inside mode; it does not diagnose the later outside-mode screenshots.
- Replace hardcoded Space/Tile variable matching and a whole-Tile canonical fingerprint with bounded React-call, style-property and module-import guards. Reuse the original image props, visibility condition and Text expression when moving the ID outside; retain callbacks and binding names.
- Keep all twelve reference outputs byte-identical to the old baseline. Add renamed-binding, arithmetic, extra-image-prop/callback and fake-context regressions.
- Replace the main log's last result after a settings save; refresh the bounded ShellUI result on a configuration/outcome change. Add current error metadata to the settings API and translate current failure phases.
- The existing manual export button can now request the actual validated RAM source before our edits. Allocation/file IO run on the native worker, capture is armed only by the explicit request, and resources are freed on completion/timeout. No automatic dump or periodic text log.
- Inside is console-confirmed; updated outside rendering and live export remain console-unconfirmed. Host execution/layout, memory, protocol, lifecycle and settings checks pass.

## 0.2.13 — live 4.03 component matching test build

- Reproduce the 0.2.12 refusal at module 527: the console reports 5,271 bytes, but the former exact-module gate required 5,276. Buffer/payload dimensions alone were insufficient to model the live input.
- Stage and copy Space/Tile using their actual validated slot lengths. Never read or write using the exported reference length for a differently sized slot.
- Guard the sole Space edit by its module wrapper and one unique code-level style-property context. Preserve all other Space bytes, including launch/focus handlers; no whole-Space digest is required.
- Verify Tile executable source with a canonical digest that excludes only its build filename, five debug line numbers and redundant outer function parentheses. Other executable changes still refuse the whole operation.
- Make inside mode independent of incompatible outside-only components. Preserve white/below-image defaults, colours, custom HEX, positions and native-title spacing.
- Correct startup/settings wording so installed hooks are not reported as confirmed rendering when validation fails. Logging remains bounded and nonperiodic.
- Add a compact fixture reproducing both reported outer dimensions and module 527 length, plus growing-path/debug-line cases, real JavaScript execution/layout and the actual async registry selector. These remain host models; console display still needs confirmation.

## 0.2.12 — live 4.03 RAM table test build

- Address the observed 4.03 refusal: live buffer 1,389,168 bytes, payload 1,385,934, 858 modules and 1,795-byte startup. The exported reference payload is 570 bytes longer.
- Resolve target module addresses from the live RAM index instead of fixed file offsets. Qualify the startup and all six complete target modules by exact digests; validate every table interval and source terminator before staging writes.
- Permit changes in unrelated module source/length without changing those modules. Preserve raw/padded transport support, settings, existing generated labels and resource cleanup.
- Include a refused module ID and its size in the bounded startup result. No automatic dump or periodic text logging.
- Add a semantically equivalent 570-byte-shorter regression fixture and independent JavaScript/layout checks. Keep the WASM verifier alive until its asynchronous checks complete.

The size-gate refusal is confirmed by the console log and reproduced locally. The synthetic compact fixture models the observed dimensions; it is not a capture of the console's live module contents. The actual target-module match and visual result still require a console run.

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

