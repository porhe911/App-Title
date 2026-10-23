# Validation: App Title 0.2.14

The supplied 4.03 screenshots confirm the 0.2.13 inside/bottom/white baseline.
They also show an incorrect outside selection: the UI selects outside/bottom,
but IDs remain at the top of the bitmaps. The updated outside renderer and live
export in this candidate are host-tested, not yet console-confirmed.

## What is established and what is unknown

The actual 0.2.13 startup log reports firmware 0x04030000, successful hooks,
Media/HTTP setup and refresh, then style_state=2, patches=1, failure=0 with
requested=inside/bottom/ffffff and applied=inside/bottom/ffffff. It does not
include the later outside-mode result. The separate older launch snapshot does
not override that current startup evidence.

Therefore the outside screenshots cannot distinguish a subsequent source-guard
refusal from an applied-layout problem. No particular failed outside module is
claimed from this log. Source matching in 0.2.13 is demonstrably too restrictive
for otherwise equivalent bindings/props in local regressions; the exact console
outside source and outcome have not yet been captured.

## Final changes

- Match modules 527/552 by wrapper, require/style binding and unique React
  element/property structure. Stop requiring fixed minifier names and a
  whole-Tile canonical digest.
- Move the original conditional Text expression outside the Image/Animated mat.
  Preserve actual image sources, props, callbacks, gate, title expression and
  live React/native/style bindings. The only omitted props are literal debug
  source metadata on reconstructed nodes. Keep the positive 202-point frame,
  48-point bitmap margin, 336-point row and native title at 214.
- Keep exact startup/table and 25/102/219/531 guards, actual module capacities,
  NUL/bounds checks and staging before every live write. Inside still changes
  only 219. Failed guards do not partly modify the live source.
- Replace the main log's HomeUI row after an explicit settings save rather
  than retaining only the startup inside result. Refresh the small ShellUI
  result on a requested-config/outcome change. Text logs are not periodic.
- Add manual live RAM export: an explicit settings request arms a bounded
  worker allocation and managed home refresh. The pre-edit validated RAM is
  copied in the hook; allocation, file IO and release occur on the native
  worker. No automatic dump, hook-thread IO or extra Mono attachment is added.
  Success/timeout release memory; controller completion polling is nonblocking.
  API/UI report `/data/AppTitle/HomeUI_live.bin` for this route. Other sessions
  retain the prior file export.

## Passed checks

- All twelve reference patch outputs equal the earlier baseline byte for byte,
  including the now console-confirmed inside mode. No style generator or
  Hermes generation/dispatch/mutation body change was introduced.
- Original 60 raw/padded views and 36 relocated-source cases pass. Additional
  96 envelope/raw cases cover compact/growing target slots, debug metadata,
  renamed React/native/style namespaces, arithmetic before the target, and
  preserved extra image properties/callbacks.
- Space source is byte-identical after undoing just the array reorder. All
  source outside intended slots, startup/index/header/tail and terminators stay
  exact. Negative missing/duplicate/fake-string/regexp target cases, wrong
  style/import/gate, table corruption and capacity/truncation cases refuse
  without any live mutation.
- Six JavaScript verification runs (reference, four changed-target fixtures,
  and the worker-exported raw RAM) each pass 10296 complete parses, 96 actual
  Tile executions, 36 geometry cases and 36 Yoga layouts. Extra image callbacks
  and props are checked through actual module execution. These are host models.
- Actual 554/555 registry selector/helpers pass the key, true/false/integer/null
  and error cases with the native API mocked.
- Native routing/Mono lifecycle/GC handle/refresh tests pass. An explicit live
  export yields every byte of the original RAM before patches; idle polls do
  not rewrite it. Success, timeout, stale serial, duplicate and retry paths
  pass. Main-result replacement is checked across 1000 user-style updates;
  latest result stays unique/bounded and retains startup evidence.
- Core settings, Media cleanup, hook rollback, session guard, capped logs,
  manual-only export, real HTTP/JSdom palette/custom HEX/placement/top/bottom/
  preview/controller navigation and persistence checks pass.
- ASan/UBSan pass the changed-target and native/export suites (host leak
  detector disabled). SHA-1/Hermes generation/refusal core checks pass; the
  full 11.60 fixture was not rerun.
- PS5 Payload SDK v0.41 / Clang 18.1.3 build, ELF dependency allowlists and exact
  shared/UI embedding pass. No compiler warnings/errors remain.

## Next console check

Keep only the new ELF in autoload, fully reboot, run the jailbreak and load it
once. Select outside/bottom/white and Save. Wait 10 seconds and inspect home.
If it remains incorrect, explicitly prepare diagnostics and return home for
five seconds; send the resulting HomeUI_live.bin and current AppTitle.log.
These describe the actual failing input and last setting, unlike an old disk
export or startup-only inside result.

Unknown source structures can still refuse safely; universal firmware support
and resolution of the separate delayed game-launch/FTP issue are not claimed.
No Sony source/dump, user photos/logs or SDK binaries are redistributed.
