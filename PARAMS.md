# Parameter IDs (do NOT change)

This file lists the stable parameter IDs used by the compressor plugin. These IDs are part of the public automation/state contract with hosts and users — do not rename, remove, or repurpose them once released.

- `threshold` — Threshold (dB) — range: -60.0 .. 0.0
- `ratio` — Ratio — range: 1.0 .. 20.0
- `attack` — Attack (ms) — range: 0.1 .. 100.0
- `release` — Release (ms) — range: 10.0 .. 1000.0
- `knee` — Knee (dB) — range: 0.0 .. 12.0
- `makeup` — Makeup Gain (dB) — range: -12.0 .. 12.0
- `mix` — Wet/Dry Mix (%) — range: 0.0 .. 100.0
- `sidechain` — Sidechain Enable (bool)
- `sc_hpf` — Sidechain HPF frequency (Hz) — range: 20.0 .. 2000.0
- `lookahead` — Lookahead Enable (bool)
- `lookahead_ms` — Lookahead time (ms) — range: 0.0 .. 100.0
- `oversample` — Oversampling Enable (bool)
- `oversample_factor` — Oversampling factor (choice: `1x`, `2x`, `4x`)

Guidelines

- Lock IDs before the first release and document any new parameters here.
- If you must change an ID, create a migration strategy that maps old IDs to new ones when loading saved states.
- Add new parameters only at the end of the list to minimize compatibility issues.
