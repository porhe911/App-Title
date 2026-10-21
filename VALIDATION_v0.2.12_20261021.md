# Validation: App Title 0.2.12

Test candidate for firmware 4.03. Successful display on the console has not
yet been confirmed for this version.

## Observed failure

The supplied 0.2.11 startup log reports firmware 0x04030000, successful Media
setup/cleanup, HTTP self-check, ShellUI injection with zero unresolved imports,
three installed hooks, one title override and one successful home refresh.
It then reports style_state=3, patches=0, input_format=2, buffer=1389168,
payload=1385934, modules=858, startup=1795, failure=1.

The declared live payload is 570 bytes shorter than the exported reference
(1386504 bytes). The 0.2.11 size gate therefore refuses it before any digest
check. Compiling the unmodified 0.2.11 patcher against a compact synthetic
fixture with the observed dimensions reproduces the same refusal and metadata.
This establishes the size gate as a cause of rejection; the actual contents
of the console's live six target modules have not been captured or verified.

The supplied manual dump is the same original reference. Its older probe
report measures the file, not the live hook input. The startup log requested
black/bottom/outside, whereas the subsequently saved config is white/top/inside.
Preferences are preserved; colour does not explain the unsupported state.

## Change and checks

- Resolve source addresses from the live 858-entry RAM table. Require the
  exact 1795-byte startup and the exact sizes/digests of all six target modules.
  Validate every nonempty range for order, overlap, bounds and NUL termination;
  retain the 32 MiB input cap and bounded trailing alignment.
- Unrelated module source and lengths may differ and remain untouched. A RAM
  whole-payload hash/length is intentionally no longer required. Replacement
  generation is unchanged, staged before any mutation and confined to the
  verified target slots. Hermes retains its previous complete-payload guard.
- All twelve original patch outputs match the 0.2.10 baseline byte for byte.
  Sixty raw/padded views and 36 relocated-source cases pass; all unrelated
  source, startup, index, header and tail bytes remain unchanged.
- The principal synthetic variant removes redundant parentheses in 285
  unrelated modules, preserving function bodies and producing exactly 1385934
  payload / 1389168 buffer bytes. This is a semantically equivalent regression
  fixture, not the actual live payload. Smaller and larger variants also pass.
- For each of the original and compact fixtures, independent checks pass
  10296 complete JavaScript parses, 96 Tile executions, 36 geometry cases and
  36 Yoga layouts covering colour, top/bottom, inside/outside and visibility.
  The asynchronous verifier now remains alive until the layout checks finish.
- Original 23 tamper cases, truncated inputs, edits in each target module,
  missing target and six malformed non-target table cases refuse without
  mutation. ASan/UBSan pass these RAM tests; host leak detection was disabled.
- Native post-decrypt routing accepts the compact fixture and reports the
  expected target ID/size on corruption. Private status is 112 bytes with a
  new magic; existing bounded startup logs add failed_module and module_size.
  No automatic dump or periodic text logging is added.
- Core persistence, Media ownership/cleanup, native hook rollback, session
  guard, capped logs, manual export and Mono lifecycle/refresh tests pass.
  Real HTTP/JSdom tests pass palette/custom HEX, top/bottom, placement,
  preview, D-pad/Cross/OPTIONS, defaults, persistence and request validation.
- PS5 Payload SDK v0.41 / Clang 18.1.3 build passes without warnings/errors.
  ELF dependency allowlists, diagnostic markers and exact embedding pass.
  The Hermes mutation/generation body remains source-identical to 0.2.11;
  the full 11.60 fixture was not rerun in this session.

## Remaining console check

Replace the old ELF, fully reboot, run the jailbreak and load this ELF once.
Wait 20–30 seconds, then use the Media settings white/below-image preset for
a visible baseline if needed. An older resident component cannot be replaced
in place. If it still refuses, the short AppTitle.log will identify a failed
target module where applicable; no automatic dump is needed for this step.

Universal firmware compatibility and resolution of the separate delayed
game-launch/FTP report are not claimed. The injector, hook installer and
game-launch handlers are unchanged. No Sony dump/source, user logs or SDK
binaries are included in the source or package.
