# TEAL TK84 Stereo VCA Compressor — Development Roadmap

**Current Status (2026-07-11):**  
Repo contains only `TEAL-TK84-SPEC.md`. Implementation has begun in the local workspace.

**Documents Created:**
- `TEAL_TK84_Parameters.h` — Complete stepped parameter system (all detents, enums, helpers)
- `IMPLEMENTATION.md` — Architecture, class breakdown, DSP decisions, challenges

---

## Phase 0: Planning & Foundation (Current)

- [x] Read and internalize full engineering spec
- [x] Create precise parameter definitions matching Rev. A exactly
- [x] Write `IMPLEMENTATION.md` (architecture + module breakdown)
- [x] Write this `ROADMAP.md`
- [ ] Commit initial files to GitHub repo (`TEAL-TK84-Stereo-VCA-Compressor`)
- [ ] Decide final framework (JUCE 8 as spec'd, or minimal custom + JUCE wrapper later)

**Next Action after Phase 0:** Begin core DSP implementation.

---

## Phase 1: Core DSP Engine (Highest Priority)

Goal: A fully functional, testable DSP engine that can process audio correctly with all features from the spec (no GUI yet).

### Deliverables
- `TK84Detector.h/.cpp`
  - Hybrid RMS + Peak
  - SC HPF (5 frequencies)
  - Stereo link blending (100% / 50% / 0%)
  - Automatic 4× internal rate for fast attack positions

- `TK84GainComputer.h/.cpp`
  - All 8 ratios including true LIMIT mode
  - Auto-scaling soft knee
  - Full attack/release timing engine (sample-rate invariant)
  - Program-dependent AUTO release (dual integrators)

- `TK84VCA.h/.cpp`
  - Exponential control law
  - Optional vintage nonlinearity model (odd harmonics under GR)
  - Clean mode for transparent operation

- `TK84Transient.h/.cpp`
  - Dual envelope differential (fast vs slow, Intensity-mapped)
  - Level-independent onset detection (slope + crest)
  - Transient Level application with soft ceiling protection
  - Linked or independent behavior following Stereo Link setting

- `TK84Engine.h/.cpp`
  - Full signal path wiring
  - Input Trim + Output Gain
  - Bypass with 10 ms equal-power crossfade + latency compensation
  - GR and VU metering accumulators
  - Oversampling control (Off / 2× / 4× linear phase hooks)
  - State management for all parameters

- Unit tests / simple console test harness (optional but recommended)

**Estimated Effort:** 2–4 focused sessions

**Success Criteria:** Can load a drum bus or full mix, dial in settings from the spec examples (Glue recipe, Drum bus punch, Limit mode), and hear correct behavior + metering.

---

## Phase 2: JUCE 8 Plugin Shell + Basic GUI

Goal: Turn the DSP engine into a loadable plugin (VST3 first, then AU/AAX/CLAP).

### Deliverables
- JUCE 8 project (Projucer or CMake)
- `PluginProcessor.h/.cpp` wrapping `TK84Engine`
- `AudioProcessorValueTreeState` layout using `AudioParameterChoice` for all stepped parameters
- Basic GUI with:
  - Stepped rotary controls (custom LookAndFeel with detent snapping)
  - 12-segment GR LED ladder component
  - Illuminated VU meter component (with switchable reference)
  - Bypass switch + status LED
- Parameter attachment that enforces stepped behavior
- Resizable window (80–200 %)
- Preset management skeleton (factory + user)

**Target:** First audible plugin that can be inserted in a DAW (especially Pro Tools via AAX later).

---

## Phase 3: Advanced Features & Integration

- Full oversampling implementation (linear-phase decimators)
- Complete metering (peak hold, ballistics tuning)
- A/B compare system
- Full preset system with `.tk84p` format + factory bank (bus glue, drum bus, parallel, limit, transient restore)
- AAX Native packaging + Pro Tools delay compensation testing
- CLAP + AU support
- CPU optimization pass

---

## Phase 4: Polish, Calibration & Release Prep

- GUI visual polish (teal face illumination, proper analog needle graphics, HiDPI assets)
- Calibration verification against spec curves
- Listening tests on real material (bus, drums, vocals, full mixes)
- Factory preset tuning
- Documentation (user manual, quick start)
- Version 1.0 tagging + release build scripts

---

## Current Blockers / Open Questions

1. **Framework confirmation** — Proceed with full JUCE 8 now, or keep DSP engine completely standalone first?
2. **Oversampling library** — Use JUCE’s built-in or custom linear-phase?
3. **Metering graphics** — How detailed should the custom VU/LED components be in v1?
4. **Testing environment** — Do you want a simple standalone test app, or jump straight to plugin?

---

## How to Use This Roadmap

After each coding session, update the checkboxes and move items between phases as reality dictates. The DSP engine in Phase 1 is the most critical — once it sounds and behaves correctly, the rest is “just” wrapping and polishing.

---

**Let’s keep momentum.** Phase 0 is essentially complete with the documents now written.

**Next suggested step:** Move into **Phase 1** and start coding the `TK84Detector` + `TK84GainComputer` (or whichever module you want to tackle first).

Ready when you are. Just say the word (or pick a specific module).