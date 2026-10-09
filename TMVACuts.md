# TMVA_Cuts pipeline

This document explains the full pipeline in this repo: training TMVA classifiers,
applying them to select a mass spectrum, and fitting that spectrum for the
Upsilon (1S+2S+3S) yield. It also documents every option/flag involved and what
"deciles" and the various background-model numbers mean.

The three stages are three separate executables, built via `make`:

```
bin/tmva_train   # trains TMVA classifiers on signal/background samples
bin/tmva_apply   # scores every event in the dataset with the trained classifiers
bin/ups_fitter   # fits the (optionally cut-selected) mass spectrum for the yield
```

There's also `runTMVAFitScan.sh`, which chains `tmva_apply` + `ups_fitter` across
every varset and every classifier cut-point automatically.

---

## 1. Training (`tmva_train` / `TMVATrain.cxx`)

Uses ROOT's TMVA framework to train classifiers to separate signal (real
dielectron pairs) from background (combinatorial pairs), based on
kinematic/detector variables read from the input tree.

### How signal/background is defined for training

`TMVATrain.cxx` (`GetSBCriteria()`) supports two **mutually exclusive** ways
of labeling training signal/background, controlled by the globals `kMass` /
`kCharge` in `TMVACommon.cxx` (only one may be `true` at a time — both or
neither is a hard error):

- **`kMass`** (currently the default: `kMass = true`, `kCharge = false`) —
  signal = unlike-sign pairs inside the Upsilon mass window
  (`charge==0 && m in [m_sig_lo, m_sig_hi] = [8.8, 11.1]` GeV/c²);
  background = unlike-sign pairs *outside* that window
  (`charge==0 && m in [7.0,8.8) or (11.1,13.0]`).
- **`kCharge`** — signal = unlike-sign pairs (`charge==0`); background =
  like-sign pairs (`charge!=0`), regardless of mass.

This is the origin of the **`c`/`m` prefix** on varset names
(`c2v`/`m2v`, `c5v`/`m5v`, `c6v`/`m6v`, `c16v`/`m16v`, used throughout
`tmva_apply` and the weight-file directories under `dataset/weights/`):
**`m` = trained with the mass-window signal/background definition (`kMass`),
`c` = trained with the charge-sign signal/background definition
(`kCharge`)**. This has nothing to do with "Cuts vs MVA" methods — both
prefixes train the same six methods (see below); the prefix only records
which signal/background labeling was used during training.

**Caveat / what I could not verify:** `TMVATrain.cxx`'s current source always
trains on a fixed, hardcoded set of 3 input variables
(`vec_inputVariables` in `TMVACommon.cxx`: `max(|bemcZDist|)`,
`max(|bemcPhiDist|)`, `max(|DCA|)`) and has no varset argument at all
(`TMVATrainMain.cxx` takes no `argv`). This matches only the smallest variable
group described below (`2v`+`6v` combined = 3 variables) — it's the same
fixed model referred to in `TMVAApply.cxx` as the "fallback weights." How the
other, larger `dataset/weights/<varset>/` directories (5v/6v/16v, with more
input variables) were actually produced isn't reflected in this file as it
stands — presumably by rerunning training with `vec_inputVariables`/`kMass`/
`kCharge` edited by hand between runs. Don't take the paragraph below (variable
groups) as something `tmva_train` currently automates per-varset — it
describes what `tmva_apply`'s **reader** does with each varset name, which is
verified directly from `TMVAApply.cxx`.

### Variable groups (verified from `tmva_apply`'s reader, `TMVAApply.cxx`)

| flag  | variables added |
|-------|------------------|
| `k16v`| `dEdx1,2`, `eta1,2`, `phi1,2`, `E_TOW1,2`, `E_CLU1,2` (10 vars: dE/dx, tracking, calorimeter) |
| `k5v` | `min(E_TOW/E_CLU)`, `max(E_CLU/p)`, `min(E_CLU/p)` (3 vars: derived calorimeter/momentum ratios) |
| `k2v` | `max(|bemcZDist|)`, `max(|bemcPhiDist|)` (2 vars: calorimeter cluster-matching distances) |
| `k6v` | `max(|DCA|)` (1 var: distance of closest approach) |

The numeric suffix in a varset name is the cumulative variable count once the
relevant flags are enabled (`AdjustVariableFlags()` in `TMVAApply.cxx`):

| varset | k2v | k5v | k6v | k16v | variable count |
|--------|-----|-----|-----|------|----|
| `c2v` / `m2v`   | X | | | | 2 |
| `c5v` / `m5v`   | X | X | | | 5 |
| `c6v` / `m6v`   | X | X | X | | 6 |
| `c16v` / `m16v` | X | X | X | X | 16 |

### Classifier methods

Six TMVA methods are trained/booked (`vec_TMVAMethods` in `TMVACommon.cxx`):

- **Cuts** — TMVA's rectangular-cuts optimizer. Rather than one score, it
  produces a pass/fail decision *per requested signal efficiency*. This repo
  always evaluates it at signal efficiencies 0.0, 0.1, 0.2, ..., 1.0 (11
  points), stored as an 11-element vector branch `TMVACutsScore`.
- **BDTG** — Gradient-boosted decision tree.
- **HMatrix** — Fisher-like H-Matrix discriminant.
- **Fisher** — Fisher linear discriminant.
- **MLP** — Multi-layer perceptron (neural network).
- **SVM** — Support vector machine.

Training writes weight files to `dataset/weights/<varset>/TMVAOptimizeCuts_<Method>.weights.xml`
(and a fallback set directly under `dataset/weights/`). A method is only
usable later if its weight file exists.

---

## 2. Applying (`tmva_apply` / `TMVAApply.cxx`)

```
./bin/tmva_apply <varset>       # e.g. ./bin/tmva_apply c16v
```

For the given varset:

1. Loads (or builds, if missing) the base preprocessed tree from
   `data/cands+base.root`.
2. Picks the weight files: prefers `dataset/weights/<varset>/...`; if none of
   the 6 methods have weights trained for that specific varset, falls back to
   the base `dataset/weights/` weights (the fixed 3-variable model described
   above, and forces the reader's variable flags to match: `k2v=true,
   k6v=true, k5v=false, k16v=false`).
3. For every event, evaluates every *available* method (whichever has a
   weight file) and writes its score into a new branch:
   - `TMVACutsScore` (`std::vector<float>`, 11 entries — one per efficiency
     point 0.0...1.0)
   - `TMVABDTGScore`, `TMVAHMatrixScore`, `TMVAFisherScore`, `TMVAMLPScore`,
     `TMVASVMScore` (each a single `float`)
4. Writes the result, keeping only `m`, `charge`, and the score branches, to:

   ```
   dataset/weights/<varset>/tree_TMVA-processed.root   (TTree named "tree")
   ```

This file is the input to the fitting stage — it's just the mass spectrum
with classifier scores attached, no cut applied yet.

---

## 3. Fitting (`ups_fitter` / `UpsFitter.cxx`)

```
./bin/ups_fitter [inputFile] [cutExpr] [tag]
```

- **`inputFile`** — a `tree_TMVA-processed.root` from step 2. Leave empty
  (`""`) to fall back to the original `data/cands+base.root` (no TMVA
  selection applied at all — useful as an uncut baseline).
- **`cutExpr`** — a `TTreeFormula`-style cut string evaluated against the
  tree, e.g. `"TMVACutsScore[5]>0.5"` or `"TMVABDTGScore>0.2"`. Leave empty
  for no cut.
- **`tag`** — a name used to label all output files for this run (defaults
  to `"default"`).

Internally: opens `inputFile`, applies `cutExpr` via `TTree::CopyTree`
(needed instead of RooFit's own `Cut()` option because the cut can reference
branches, like `TMVACutsScore[i]`, that aren't part of the fitted dataset),
splits events into **US** (unlike-sign, `charge==0` — the real signal+background
sample) and **LS** (like-sign, `charge!=0` — a background-only control sample),
and performs a simultaneous mass fit to both. (Note: this US/LS split by
charge is independent of, and unrelated to, the `c`/`m` varset-naming
convention from training above — both apply to every varset either way.)

### The fit model

Both a US and LS spectrum are fit *simultaneously* (shared background-shape
philosophy: the LS spectrum constrains what the combinatorial background
under the US peak should look like). The signal model is the same in both
background variants:

- Three Crystal Ball shapes (`gauss1/2/3`), representing the Upsilon 1S, 2S,
  3S resonances. Their means are tied together (`sig2_mu = sig1_mu + 0.560`,
  `sig3_mu = sig1_mu + 0.890`, the known 2S−1S and 3S−1S mass splittings) —
  only the 1S mean floats freely.
- Parameters: `sig{1,2,3}_mu` (mean), `sig{1,2,3}_sigma` (width),
  `sig{1,2,3}_alpha`/`sig{1,2,3}_n` (Crystal-Ball tail shape parameters —
  these are numerically the least well-constrained, since they only affect
  the far tails of the peak).
- `sig1_frac`, `sig2_frac` — recursive fractions controlling the relative
  weight of the 3 signal peaks vs. each other within the "signal" portion.
- `bgUS_frac` — fraction of the US sample that's background (floats).
- `bgLS_frac` — fraction of the US sample attributable to LS-shaped
  background, i.e. how much the LS spectrum should be scaled to model the US
  background. Seeded from the raw `LS_entries/US_entries` count ratio, and
  **floats** in the fit (not fixed) so the fit itself determines the best
  value rather than assuming raw counts translate directly into fit weight.

**Two background shapes are fit and shown on every plot, independently:**

1. **Exponential background** (`bgLS`, `bgUS` — one decay constant each for
   LS/US). This is the original/default model.
2. **2nd-degree polynomial background** (`bgLS_c1,c2`, `bgUS_c1,c2` — shape
   `1 + c1·x + c2·x²`, via `RooPolynomial` with `lowestOrder=1`; the implicit
   constant term is dropped because it's redundant with the fit's own
   normalization).

Both are complete, independent simultaneous fits of the same US+LS dataset —
same signal model, different background functional form — so you can compare
which background shape actually describes the data better (via each one's
own χ²/ndof) without re-running anything.

### Reading the plot (`output/fig/<tag>_<date>_<timestamp>_fit.png`)

- Black points: US (unlike-sign) data. Red points: LS (like-sign) data.
- **Blue** solid curve: the exponential-background fit's total curve, with a
  thin gray hatched band hugging it — the fit's 1σ statistical uncertainty
  (`RooFit::VisualizeError`, linear propagation of the full fit covariance
  matrix). Its individual components are drawn as outlines only (not
  filled), to keep them visually distinct rather than stacking into a single
  wash: red-dashed = LS exponential, dark-gray-dashed = US exponential,
  magenta shades = the 3 individual Crystal-Ball signal peaks, teal = their
  sum, dark-orange = the combined exponential background (LS+US).
- **Green** dash-dot curve: the polynomial-background fit's total curve.
  Green dotted curve: the polynomial-background fit's LS-only background
  curve (its US-side background component isn't drawn separately, to avoid
  cluttering the plot further).
- Text annotations, color-matched to each model: `χ²/N_dof` for both US and
  LS samples, the fitted `Y(1S+2S+3S)` yield ± its propagated error, and
  `S/√(S+B)` (rough significance estimate).

### Output files

- `output/fig/<tag>_<date>_<timestamp>_fit.png` — the plot above.
- `output/results/<tag>_<date>_<timestamp>_fit.txt` — plain-text dump of
  US/LS entry counts and both models' χ²/ndof, signal fraction, yield, and
  yield error.
- `output/corr/`, `output/rootfiles/` — created but currently unused by the
  fit code.

---

## 4. Scanning cuts automatically (`runTMVAFitScan.sh`)

```
./runTMVAFitScan.sh [varset ...]     # defaults to all 8 varsets if none given
```

For each varset, this script:

1. Runs `tmva_apply <varset>` to (re)produce `tree_TMVA-processed.root`.
2. Discovers which TMVA score branches actually exist in that tree (some
   varsets may be missing weights for some methods) via
   `tools/list_score_branches.C`.
3. For **`TMVACutsScore`**: runs `ups_fitter` once per stored efficiency
   point, `TMVACutsScore[0]>0.5` through `TMVACutsScore[10]>0.5` — these are
   TMVA's own 11 requested-signal-efficiency working points (0%, 10%, ...,
   100%), not something computed here.
4. For every other (scalar) method branch — **BDTG, HMatrix, Fisher, MLP,
   SVM** — runs `ups_fitter` once per **decile** of that branch's score
   distribution in the tree, computed at runtime by
   `tools/score_deciles.C`.

### What "decile" means here

Since BDTG/Fisher/MLP/SVM/HMatrix scores don't share a common, predefined
scale the way Cuts' [0,1] efficiency index does (their numeric ranges depend
on the classifier and training), a fixed threshold like `>0.2` would select a
wildly different fraction of events for different methods or varsets. Instead,
`tools/score_deciles.C` looks at the *actual* score distribution for that
specific branch/tree, sorts all values, and reports the value at each 10%
percentile boundary (0th, 10th, 20th, ..., 100th percentile — 11 values
total, matching the same 11 cut-points Cuts naturally provides). Cutting at
the Nth decile therefore always selects roughly `(100-N)%` of events,
regardless of the classifier's raw score range — putting all 6 methods'
11-point scans on a comparable "how tight is this cut" footing.

Each resulting fit is tagged `<varset>_<Method>_cut<idx>` (e.g.
`c16v_BDTG_cut7`), so results/plots for every varset × method × cut-point
combination land as distinctly-named files in `output/fig`/`output/results`.

A fit that ends up with an empty (or near-empty) unlike-sign sample after a
very tight cut is skipped cleanly (`ups_fitter` exits non-zero, logged as a
warning) rather than crashing — this can legitimately happen at the tightest
cut-points where almost no background survives.

---

## Quick reference: common commands

```bash
# Train (writes weight files under dataset/weights/)
./bin/tmva_train

# Apply a trained varset to the full dataset (writes tree_TMVA-processed.root)
./bin/tmva_apply c16v

# Fit with a specific score cut
./bin/ups_fitter dataset/weights/c16v/tree_TMVA-processed.root "TMVABDTGScore>0.2" myrun

# Fit with no cut at all (baseline)
./bin/ups_fitter dataset/weights/c16v/tree_TMVA-processed.root "" baseline

# Full automated scan across all varsets/methods/deciles
./runTMVAFitScan.sh

# Scan just one varset
./runTMVAFitScan.sh c16v
```
