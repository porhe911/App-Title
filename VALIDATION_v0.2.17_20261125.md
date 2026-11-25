# App Title v0.2.17 validation

- PS5 13.00 (`0x13000040`): owner confirmed corrected outside placement on 2026-11-25.
- Private diagnostic SHA-256: `6f9ee7f274d286a6d2edf38e9379c149c9a8938ca0036fbb2e268cbb80698284`. Hermes v89, 1795460-byte payload, 9727 functions, payload SHA-1 `8cae100a46561442979d1c4e073267515306d12a` matches 13.60.
- Accepted candidate: commit `cd4479f0fbc3d7f4991593cbcb20a8c4b92b96b6`, ELF SHA-256 `f3bd640ba0e1c08f56421d928a33594134ef1b37b95830d5edc9db8be97850c1`, 299400 bytes. CI: https://github.com/porhe911/App-Title/actions/runs/45680515824.
- Non-target generator outputs unchanged in candidate comparison; existing 4.03 and 9.05 profile sources unchanged. The private fixture is not distributed.
- Normal release rebuild updates version strings. Release CI must pass SDK build, ELF verification, host regressions and package validation; this is not a new console test of v0.2.17 or 13.60.
