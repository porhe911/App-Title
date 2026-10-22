# Validation: App Title 0.2.13

Candidate for the firmware 4.03 display refusal. The observed refusal has been
reproduced locally and the replacement code passes the checks below. Successful
display on the user's console has not yet been confirmed for this version.

## What the new logs establish

The actual 0.2.12 startup log identifies firmware 0x04030000. Media registration,
service cleanup, settings HTTP self-check, ShellUI injection (zero unresolved
imports), all three hook installations and the home refresh succeed. The native
Title ID query is intercepted once and overridden once.

The failure is later, in RAM style qualification:

```
style_state=4 patches=0
buffer=1389168 payload=1385934 modules=858 startup=1795
failure=4 failed_module=527 module_size=5271
```

The previous build required module 527 to have the reference size 5276 and an
exact whole-module digest. Its first three target checks (25, 102, 219) passed
before this refusal. Fixing the overall payload length in 0.2.12 therefore did
not address the next real difference: the live component itself is shorter.
The old fixed-length copy path was not reached; this log does not show that
0.2.12 wrote past a slot or caused a crash.

The actual five-byte source difference has not been captured. Build-path changes
are a supported test case, not a claimed explanation of those five bytes. The
log also does not yet expose the source or actual size of module 552.

## What changed

- Read/stage/copy module 527 (Space) and 552 (Tile) using their actual validated
  RAM-index lengths, with 8 KiB / 4 KiB staging caps. Keep the original index,
  terminators, header, allocation tail and unrelated modules untouched.
- Space no longer requires a whole-module digest. It requires its module
  wrapper and a unique complete strand style-property context. The match must
  lie outside quoted strings/comments; ambiguous bare slashes before it are
  conservatively refused. Only the order of two existing style-array elements
  is changed. Every other Space byte, including launch/focus handlers, stays
  exact even when its unrelated source differs from the export.
- Tile uses a canonical digest that normalizes only its build filename, five
  debug line numbers and redundant outer function parentheses. Imports,
  visibility, image sources, callbacks and other executable source must still
  match. This normalization is for comparison and is not written to live RAM.
- Outside mode retains exact source digests for 25, 102, 219 and 531. Inside
  mode qualifies and edits 219 only, after common startup and index validation;
  an incompatible outside-only component no longer blocks that option.
- All edits remain staged before any live mutation. White/below-image defaults,
  colours/custom HEX, top/bottom, placement and the native game-title gap remain.
- Correct status/notification wording: installed hooks are not proof of visible
  rendering. Private status remains 112 bytes. Logging remains bounded and
  nonperiodic, with no automatic source dump.

## Reproduction and passed checks

- The compact regression shortens 110 build filename literals by five bytes
  and removes redundant parentheses in ten unrelated module wrappers. It has
  exactly the reported buffer 1389168, payload 1385934 and module 527 size 5271.
  Its Tile size 1943 is synthetic, not a measurement from the console.
- Unmodified 0.2.12 compiled from the parent commit refuses this fixture with
  result=invalid, failure=4, failed_module=527, module_size=5271 and no mutation.
  The new patcher accepts it in all twelve colour/position/placement modes.
- All twelve original-envelope outputs match the 0.2.10 baseline byte for byte.
  Sixty raw/padded views and 36 previously relocated-source cases pass.
- Seventy-two additional envelope/raw cases pass for shortened target slots,
  longer build paths, changed debug lines and optional wrapper parentheses.
  Undoing the one Space array reorder restores every byte of its input source.
  Index/startup/header/tail/unrelated-source comparisons pass too.
- Eight context/source guard cases refuse without any mutation: missing or
  duplicate context, context only in a string or regexp, oversized Space,
  changed executable Tile source, wrong component suffix and a nonliteral
  debug line expression. Inside mode remains independent in those cases.
  Original tamper, malformed-table, missing-target and truncated-input checks
  pass. ASan/UBSan pass the RAM and context suites (host leak detection off).
- Four independent JavaScript execution/layout runs (reference plus three
  changed-target fixtures) each pass 10296 complete module parses, 96 Tile
  executions, 36 geometry cases and 36 Yoga layouts. These cover visibility,
  performance capture, colours, top/bottom, inside/outside and title spacing.
- Actual modules 554/555 and their async/transpiler helpers execute in the host
  registry check. The exact isHomeTitleIdDisplayEnabled key, true/false/integer/
  null results and error propagation pass. Native Registry and Recoil factory
  interfaces are mocked; this is not a native-console API test.
- Native post-decrypt routing accepts the shortened-target fixture and reports
  applied state. Mono lifecycle/GC-handle/refresh, core persistence, Media
  ownership/cleanup, native hook rollback, session guard, capped logs and
  manual-only export checks pass. Real HTTP/JSdom settings checks pass palette,
  custom HEX, placement, top/bottom, preview, D-pad/Cross/OPTIONS, defaults,
  restart persistence and request validation.
- Hermes generation/hash/refusal core checks pass; the entire home_style.cpp
  is source-identical to the 0.2.12 parent. The full 11.60 console fixture was
  not rerun in this session.
- PS5 Payload SDK v0.41 / Clang 18.1.3 build passes without warnings/errors.
  ELF dependency allowlists, diagnostic markers and exact embedding pass.

## Console check and remaining limits

Leave only this version in autoload, fully reboot, run the jailbreak and load
the ELF once. Wait 20–30 seconds. The default is white IDs below the bitmap;
existing preferences are preserved. An older resident component cannot be
replaced in place during the same boot.

The supplied logs establish the 0.2.12 refusal, not the complete live source.
Unknown Tile executable changes or missing style contexts still refuse safely;
universal firmware compatibility is not claimed. The separate delayed
game-launch/FTP report has not been resolved by a long console test. No
injector, hook-installer, game-launch, jailbreak or FTP behavior was added.
No Sony dump/source, user logs or SDK binaries are included in the package.
