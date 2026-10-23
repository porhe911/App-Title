# Validation: App Title 0.2.10

Test build for resource lifecycle during resident operation, retaining the 4.03
profile from 0.2.9 and the 11.60 profile from 0.2.8. No console test or successful
long-duration game/FTP run has been performed by the developer.

## Motivation and scope

The supplied 11.60 system logs show three game launches rejected by LNC with
`0x80940033 (m_is_pkg value is false)` before a game process starts. Two of the
same games launch in the next boot session. The FTP process is listed, but the
logs do not identify its connection failure. This does not prove a root cause.

Code review found an owned native worker left attached to Mono throughout its
polling/sleep loop, and successful AppInstUtil/UserService initialization with
no matching termination. This version bounds those lifetimes. The background
worker performs no managed API work after bootstrap; refreshes still run on
the existing managed UI thread. Metadata pointers refer to the existing root
domain. No runtime/domain shutdown is requested.

Only sessions successfully initialized by this call are terminated. The
installer closes while the temporary installer identity is still active;
UserService closes after restoring the original identity. A missing cleanup
export prevents that service's initialization. Explicit cleanup failure stops
startup rather than continuing resident operation.

The ioctl result and errno are preserved across style work. The Update
callback preserves the original Update's errno across a queued refresh.
There are no automatic jail, package-registration, mount, FTP or ShellCore
repairs. The layout patchers and game launch handlers are unchanged.

## Checks

- Mono bootstrap success, all seven stage failures and three invalid signature
  cases pass. Every successful attach has exactly one detach; the worker is
  detached before refresh frames. Reentry, exceptions, GC handles and original
  errno are checked while retaining one refresh per actual settings change.
- Actual post-decrypt routing passes with both supplied 4.03 and 11.60 files.
- Mocked Media APIs pass cold load, existing launcher, install/query failures,
  16 repeat registrations alternating 4.03/11.60, initialization ownership,
  missing cleanup exports, unavailable installer, termination failures and
  identity-restoration failure. AppInstUtil closes before identity restoration;
  UserService closes after it, including early-return cleanup paths.
- The 4.03 RAM profile passes twelve style combinations, 23 tamper cases,
  truncated inputs and unchanged module capacity/NUL/header/table/startup/
  launch-module/trailer checks.
- All eight complete 11.60 generated patch outputs equal the 0.2.8 baseline
  byte for byte. Both layout patchers, the hook installer, the injector and
  settings-service implementation equal 0.2.9 source byte for byte.
- Core persistence/relocation, hook live-thread/rollback checks, reboot guard,
  capped/nonperiodic logs and authenticated/manual-only export checks pass.
- Real host HTTP/JSdom tests pass palette/custom HEX, inside/outside, top/bottom,
  preview, D-pad, Cross/OPTIONS, reset to white/bottom/outside, restart
  persistence and token/Host/Origin/malformed-request checks.
- PS5 Payload SDK v0.41 / Clang 18 build passes without warnings or errors.
  ELF dependency allowlists, Mono attach/detach imports, dynamic cleanup export
  names and exact embedded shared-component checks pass.

## Limits

Host mocks verify ownership and call ordering, not Sony service internals or
the console's GC implementation. Preserving the exact style patcher outputs
does not replace a console render test. A full reboot is required before
loading this version because the old injected component remains resident.
This version is a mitigation candidate, not a confirmed resolution of the
delayed game-launch and FTP report. Universal firmware support is not claimed.
