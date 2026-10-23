# Validation: App Title 0.2.11

Test candidate for the reported firmware 4.03 style refusal. No successful
console run of this version has been observed by the developer.

## Evidence and uncertainty

The supplied 0.2.10 startup log records firmware 4.03, successful Media
registration and service cleanup, working HTTP self-check, successful ShellUI
injection with zero unresolved imports, all three hooks installed, one registry
override and one successful home refresh. Its final state is unsupported style
(3) with zero patches. These facts isolate the reported startup outcome to the
style-selection path; they do not expose the live buffer dimensions.

The supplied HomeUI equals the earlier 4.03 fixture and passes the complete
original source and module checks. The supplied probe report identifies itself
as 0.2.8; it is not a measurement of the live 0.2.10 hook. The attached config
stores black/outside, while the startup log initially requested white/inside.
This is not treated as the cause of unsupported style, and saved preferences
are not overwritten.

The previous code rejects the identical source if its declared payload length
includes eight alignment bytes (unsupported), and ignores a raw payload view.
Both cases have been reproduced by compiling the unmodified 0.2.10 sources.
Whether either representation explains this console's refusal remains a
hypothesis. This build fixes these demonstrated input-handling cases and adds
small live-input metadata to make the remaining refusal identifiable.

## Change boundaries

- Accept the full known RAM source prefix with padding only through its next
  16 KiB allocation boundary, in an RNPS envelope or raw decrypted view.
- Retain the original complete-source digest, six module hashes, table entries,
  terminal NUL checks and staging before mutation. No relaxed source matching
  or writes in padding/transport data are introduced.
- Keep the original style generation and module replacements unchanged.
- Extend the private controller/shared status to 104 bytes with a new magic.
  The existing startup result reports format, buffer/payload sizes, module and
  startup counts, and refusal reason. No automatic dump or periodic text log.
- Preserve the injector, hook installer, settings service and 0.2.10 service/
  Mono resource lifecycle changes. No ShellCore, FTP, jailbreak or mount edits.

## Passed checks

- All twelve original-envelope 4.03 patch outputs equal the unmodified 0.2.10
  outputs byte for byte.
- Sixty additional raw/padded transport cases produce the identical source
  patches; every transport and trailing byte remains unchanged.
- The sixty corresponding corrupted-source cases fail the complete digest
  without mutation. The original 23 tamper and truncated-input cases pass.
- Native post-decrypt routing accepts the padded case and reports actual input
  metadata; altered module count produces the expected bounded refusal data.
- ASan and UBSan pass the RAM cases (host leak detector disabled).
- Core settings/status/persistence, native hook rollback/live-thread checks,
  reboot guard, capped/nonperiodic logs and manual-only export checks pass.
- Mocked Media service ownership/cleanup and Mono attach/detach, UI-thread
  refresh, errno, reentry/exception and GC-handle tests pass.
- Real HTTP/JSdom settings tests pass colour palettes/custom HEX, placement,
  top/bottom, preview, D-pad/Cross/OPTIONS, defaults, restart persistence and
  authentication/Host/Origin/malformed-request checks.
- Hermes SHA-1/style-generation/refusal core tests pass. The existing Hermes
  mutation/generation body is source-identical to 0.2.10. The full 11.60 fixture
  was not rerun in this session.
- PS5 Payload SDK v0.41 / Clang 18.1.3 build passes without warnings/errors;
  ELF dependency allowlists, diagnostic markers and exact embedding pass.

## Console check still required

Replace the previous ELF, fully reboot, run the jailbreak, load this version
once, wait 20–30 seconds, then select the white/below-image preset and Save in
Media settings. The previous resident component cannot be replaced in place.
If labels remain absent, the new short AppTitle.log is needed; the old exported
HomeUI alone does not describe the rejected live input.

Universal firmware compatibility and resolution of the separate delayed
game-launch/FTP report are not claimed. No Sony dump/source or SDK binaries
are included in the source or release package.
