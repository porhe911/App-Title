# Validation: App Title 0.2.9

This is a test build adding the supplied firmware 4.03 HomeUI profile.
No console test of this version has been performed by the developer.

## Scope

- 4.03 input: RNPS v2 with an Indexed RAM bundle, 858 modules, 1,795 bytes of startup code and 1,386,504 bytes of plaintext.
- Existing 11.60 input: the previously verified Hermes v89 profile.
- Existing native registry, UI-thread reload, injector, settings service and Media registration behavior are retained.
- Default: enabled, white, below the artwork, outside mode. Existing saved preferences remain in effect.
- The separate report of game launches failing after several hours is not resolved by this change.

## Passed checks

- Exact RAM profile checks, twelve inside/outside/top/bottom/white/custom/black combinations.
- Module offsets, capacities, terminal NUL bytes, unrelated modules, startup code and the outer header/trailer remain unchanged.
- All 10,296 generated module sources parsed with Node.js.
- 96 executions of the actual Tile module cover native visibility/loading gates and both telemetry branches.
- 36 actual geometry/animation cases cover three sizes; 36 Yoga layouts verify positive bounds, artwork separation and title-row spacing.
- All 23 altered RAM fixtures and truncated-buffer cases are refused without writes.
- ASan and UBSan pass for RAM mutation and refusal cases. Leak detection is disabled because of the host environment.
- All eight complete 11.60 HomeUI patch outputs match the 0.2.8 baseline byte for byte.
- Post-decrypt routing tests pass with both the 4.03 and 11.60 fixtures.
- Core settings/persistence, relocation/hooks/rollback, restart guard, bounded logging and manual export checks pass.
- Media registration and credential restoration checks pass with mocked PS5 APIs.
- The actual HTTP and HTML interface passes palette/custom HEX, top/bottom/placement, D-pad, Cross/OPTIONS save, persistence, token and Host/Origin checks.
- PS5 Payload SDK v0.41 / Clang 18 build passes without warnings. ELF dependency allowlists and exact embedded ShellUI component checks pass.

## Limits

JavaScript execution uses Node.js with mocked native components; Yoga is a host layout model.
These checks do not implement the console's JavaScript engine, RNPS native clipping or font rasterization.
The complete 4.03 console run, including visual placement and game-launch testing, is still required.
Other HomeUI profiles and universal firmware compatibility are not claimed.
No Sony bundle, extracted module, font, firmware dump, SDK binary or external parser is included in this source tree or release package.
