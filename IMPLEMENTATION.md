# TEAL TK84 — Implementation Notes

**Product:** TEAL TK84 Stereo VCA Compressor  
**Status:** Pre-release development  
**Last Updated:** 2026-07-11  
**Spec Reference:** TEAL-TK84-SPEC.md (Rev. A)

---

## 1. Goals & Design Philosophy

The TK84 must feel like a **precision analog bus compressor** with modern recallability and one unique twist: the post-compressor **Transient Processing stage**.

Core principles:
- Every primary control is **stepped** — no continuous pots. Automation snaps to detents.
- Perfect stereo imaging at all times (linked sidechain is non-negotiable).
- The Transient stage is **not** a lookahead enhancer. It is a post-compression differential processor that can restore or suppress attack energy that the compressor removed.
- Low CPU when idle (Clean mode + OS Off). Acceptable overhead only when using fast attack + oversampling or heavy transient shaping.
- Hardware-like behavior: double-click = default, scroll = one detent, no fine-drag on stepped params.

---

## 2. High-Level Architecture

```
                    ┌─────────────────────────────────────────────────────────────┐
                    │                        TK84Engine                           │
                    │                                                             │
Input L ──► [Input Trim] ──► [VCA L] ──► [Transient L] ──► [Output Gain] ──► Out L
Input R ──► [Input Trim] ──► [VCA R] ──► [Transient R] ──► [Output Gain] ──► Out R
                    │               ▲                       ▲
                    │               │                       │
                    │   ┌───────────┴───────────┐   ┌───────┴───────┐
                    │   │   Linked Sidechain    │   │  Transient    │
                    │   │  (Detector + Gain     │   │  Processor    │
                    │   │   Computer)           │   │  (Post-VCA)   │
                    │   └───────────────────────┘   └───────────────┘
                    │
                    │   Metering: GR LED (from CV) + VU (post Output Gain)
                    │   Bypass: 10 ms equal-power crossfade, latency compensated
                    └─────────────────────────────────────────────────────────────┘
```

**Key Decisions:**
- **Feed-forward** detection (pre-VCA) for fast, modern VCA character.
- **Max(L,R)** linked detection at 100% link. At lower link % the detectors diverge.
- **Transient stage is post-VCA, pre-Output Gain**. This is critical — the compressor sets density, the transient stage then shapes the front edge of that compressed signal.
- All timing (attack/release) computed in the **log domain** for natural ballistics.
- 64-bit float throughout. No dithering inside the plugin.

---

## 3. Core DSP Modules (Proposed Class Structure)

### 3.1 Parameters (Done)
`TEAL_TK84_Parameters.h` — already implemented with exact stepped values, enums, and helper functions.

### 3.2 Detector (`TK84Detector`)
- Hybrid RMS + Peak detector.
- RMS window ≈ 5 ms equivalent.
- Automatic weighting: more peak at fast attack settings, more RMS at slow settings.
- Sidechain HPF (Butterworth 12 dB/oct or Linkwitz-Riley).
- Stereo link amount blends between max(L,R) and independent L/R.
- Fast attack positions run detector at 4× internal rate with polyphase decimation of the control signal.

### 3.3 Gain Computer (`TK84GainComputer`)
- Soft knee width auto-scales with ratio (6 dB at 1.5:1 → 1 dB at LIMIT).
- All 8 ratios including true LIMIT (∞:1 with narrowed knee).
- Attack/Release coefficients recalculated per sample rate (time constants are sample-rate invariant).
- **AUTO release**: parallel fast (~120 ms) and slow (~1.2 s) integrators with crest-factor weighted mixing.
- Outputs control voltage in dB (negative = gain reduction).

### 3.4 VCA Gain Cell (`TK84VCA`)
- Exponential control law (6.1 mV/dB modeled).
- Optional "Vintage" mode: level-dependent 3rd-harmonic generation under gain reduction (≤ 0.05 % THD at 10 dB GR).
- Clean mode = mathematically linear (for mastering transparency).
- Per-channel, but driven by shared CV when linked.

### 3.5 Transient Processor (`TK84Transient`)
This is the signature feature.

- **Dual envelope follower differential** (fast vs slow).
- Fast follower attack time mapped from Intensity (0.1–2 ms).
- Slow follower attack time mapped from Intensity (20–80 ms).
- Onset detection is **slope + crest-factor** based, **level-independent**.
- Only the transient differential is gained by Transient Level (±12 dB).
- Sustain component always passes at unity.
- Soft saturating ceiling at +6 dBFS internal to protect against overshoot when boosting transients.
- Stereo behavior follows the Stereo Link setting.

### 3.6 Engine (`TK84Engine`)
Top-level class that owns:
- Two VCAs + one shared GainComputer + one Detector
- Two Transient Processors (or one linked)
- Input Trim, Output Gain
- Bypass crossfader
- Metering accumulators (GR, VU ballistics)
- Oversampling state machine (signal path 2×/4× linear phase when enabled)

---

## 4. Metering Implementation

### GR LED Ladder (12 segments)
- Source: final control voltage (post link).
- Attack: instantaneous (sample accurate).
- Release: 300 ms per 10 dB fallback.
- Peak hold: 1.5 s on top segment (user defeatable).
- Two ranges: ×1 (0.5–20 dB) and ×0.5 high-res (0.25–10 dB).

### Illuminated VU Meter
- True VU ballistics (ANSI C16.5: 300 ms integration).
- Placed **after** Output Gain so the needle shows the actual level being sent downstream.
- 0 VU reference switchable (−20 / −18 / −14 dBFS).
- Per-channel true-peak LED at −0.3 dBFS (2 s hold).

**Note:** VU meter should be relatively cheap to run — use a simple one-pole filter + RMS approximation that matches the spec ballistics.

---

## 5. JUCE Strategy

- **Framework:** JUCE 8 (as specified).
- **Parameter types:** Use `AudioParameterChoice` for all stepped/enum parameters. This gives the host correct stepped automation and discrete value lists.
- **GUI:** Fully vector, resizable 80–200 %. HiDPI/Retina.
- **LookAndFeel:** Custom TEAL house style (teal accents on dark charcoal).
- **Custom components needed:**
  - Stepped rotary knob (detent snapping + scroll wheel = 1 detent)
  - 12-segment LED ladder (with peak hold)
  - Analog-style VU meter with illuminated face (power state aware)
- **State:** Full `AudioProcessorValueTreeState` + custom XML for non-APVTS state (meter settings, etc.).
- **Presets:** Factory bank + user bank in a simple `.tk84p` format (or just use JUCE preset system + extra metadata).

---

## 6. Performance & Latency Targets

| Mode                    | Latency (samples) | Notes |
|-------------------------|-------------------|-------|
| Clean + OS Off          | 0                 | Default for bus work |
| OS 2×                   | 32                | Linear-phase decimator |
| OS 4×                   | 64                | For fast attack or heavy transient shaping |
| Bypass                  | 0 (compensated)   | Reported latency constant |

Target CPU: < 0.5 % of one modern core per instance in typical bus use (Clean mode).

---

## 7. Known Challenges & Solutions

| Challenge                              | Solution |
|----------------------------------------|----------|
| Stepped automation in hosts            | `AudioParameterChoice` + custom attachment that snaps |
| Fast attack aliasing in sidechain      | Automatic 4× internal oversampling + polyphase decimation of CV |
| Level-independent transient detection  | Slope + crest-factor detection, not threshold-based |
| Transient boost overshoot              | Internal soft ceiling at +6 dBFS before output gain |
| True VU ballistics + low CPU           | Dedicated lightweight VU class (not full RMS every sample) |
| AAX / Pro Tools workflow               | Proper AAX wrapper + delay compensation reporting |
| Perfect channel matching               | Shared CV for VCA + linked detector at 100 % link |

---

## 8. Testing & Calibration Plan

1. **Static curve verification** — feed 1 kHz sine, measure GR vs ideal curves for all ratios + knee.
2. **Timing verification** — step response tests for attack/release (63 % method).
3. **Transient stage isolation** — feed drum bus, verify that Intensity only affects onset portion while sustain stays unity.
4. **Stereo imaging** — hard-panned material at various link % settings.
5. **Bypass transparency** — level-matched A/B with 10 ms crossfade.
6. **Pro Tools session** — insert on real mix bus, compare recall and workflow feel.

---

*This document will evolve as implementation progresses.*