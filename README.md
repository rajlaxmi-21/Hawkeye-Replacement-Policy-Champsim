# Hawkeye Cache Replacement Policy on ChampSim

This repository is [ChampSim](https://github.com/ChampSim/ChampSim), a trace-based
microarchitecture simulator, extended with an implementation of the **Hawkeye**
cache replacement policy (Jain & Lin, ISCA 2016) for the last-level cache (LLC),
plus the configs, unit tests, and scripts used to evaluate it against LRU.

Hawkeye replays each cache set's access history through **OPTgen**, an efficient
approximation of Belady's OPT algorithm, to label past accesses as OPT-hits or
OPT-misses. Those labels train a PC-indexed **predictor** that classifies each
load instruction as cache-friendly or cache-averse. On a fill, a line's
classification sets its initial **RRIP** re-reference value, so lines from
cache-averse PCs are evicted first.

## Repository layout

```
replacement/hawkeye/     Hawkeye policy implementation
  hawkeye.h / hawkeye.cc    Glue module wired into ChampSim's replacement API
  optgen.h / optgen.cc      OPTgen: per-set OPT-hit/miss classification
  predictor.h / predictor.cc  PC-indexed saturating-counter predictor
  rrip.h / rrip.cc          RRPV insertion/victim-selection helpers

hawkeye_tests/            Standalone unit tests for optgen/predictor/rrip
  optgen_test.cc, predictor_test.cc, rrip_test.cc
                           Each has a TEST_VECTOR_START/END block meant to be
                           filled in with concrete inputs (by hand or a grading
                           script) before compiling and running it directly.

llc{4,8,16}way_{lru,hawkeye}.json
                           Minimal configs used to sweep LLC associativity and
                           compare the "lru" and "hawkeye" replacement policies.

results_plot_1/           champsim stdout logs: miss rate vs. associativity
results_plot_2/           champsim stdout logs: per-benchmark miss-rate reduction
plots.py                  Parses the logs above and renders the two PNG plots
plot1_missrate_vs_associativity.png
plot2_missrate_reduction.png

traces/                   .champsimtrace.xz traces used for evaluation
test.txt                  Shell snippets used to (re)generate results_plot_1/2
```

Everything else (`src/`, `inc/`, `branch/`, `btb/`, `prefetcher/`, `config/`,
`Makefile`, `config.sh`, `vcpkg/`, ...) is upstream ChampSim; see
[Using ChampSim](#using-champsim) below.

## Building and running Hawkeye vs. LRU

```sh
# one-time setup
git submodule update --init
vcpkg/bootstrap-vcpkg.sh
vcpkg/vcpkg install

# configure and build with the Hawkeye LLC replacement policy
./config.sh llc16way_hawkeye.json
make -j"$(sysctl -n hw.ncpu)"   # or nproc on Linux

# run a trace
bin/champsim --warmup-instructions 20000000 --simulation-instructions 50000000 \
    traces/456.hmmer-191B.champsimtrace.xz
```

Swap in `llc16way_lru.json` (or the 4-/8-way variants) and rebuild to compare
against baseline LRU. `LLC TOTAL ACCESS/HIT/MISS` in the simulator output gives
the miss rate for that run.

### Reproducing the plots

`test.txt` contains the exact build/run loops used to populate
`results_plot_1/` (associativity sweep on `456.hmmer-191B`) and
`results_plot_2/` (miss-rate reduction on `456.hmmer-191B`, `429.mcf-22B`, and
`473.astar-42B` at the default 16-way LLC). Adjust the `results3`/`results4`
output directory names in `plots.py` to match wherever you save the logs, then:

```sh
python3 plots.py
```

### Results snapshot

| LLC ways | LRU miss rate | Hawkeye miss rate |
|---|---|---|
| 4  | 29.6% | 28.1% |
| 8  | 27.0% | 23.8% |
| 16 | 25.8% | 20.7% |

(`456.hmmer-191B`, from `results_plot_1/`; see `plot1_missrate_vs_associativity.png`.)

Miss-rate reduction over LRU at 16-way varies by benchmark (see
`plot2_missrate_reduction.png` / `results_plot_2/`) — Hawkeye's benefit depends
on how much reuse-distance structure OPTgen can extract from each trace.

## Unit tests

`hawkeye_tests/*.cc` each `#include` one Hawkeye header directly and exercise it
with a small hand-written driver. Fill in the `TEST_VECTOR_START`/`_END` block
in a file, then compile and run it standalone, e.g.:

```sh
g++ -std=c++20 -I replacement/hawkeye hawkeye_tests/optgen_test.cc \
    replacement/hawkeye/optgen.cc -o /tmp/optgen_test && /tmp/optgen_test
```

## Using ChampSim

<a id="using-champsim"></a>
ChampSim is the result of academic research. If you use this software in your
work, please cite it:

    Gober, N., Chacon, G., Wang, L., Gratz, P. V., Jimenez, D. A., Teran, E., Pugsley, S., & Kim, J. (2022). The Championship Simulator: Architectural Simulation for Education and Competition. https://doi.org/10.48550/arXiv.2210.14324

ChampSim uses [vcpkg](https://vcpkg.io) (included as a submodule) for
dependencies, and takes a JSON configuration script — see `champsim_config.json`
for a fully-specified example; any option not specified falls back to a
default.

```sh
./config.sh <configuration file>
make
bin/champsim --warmup-instructions 200000000 --simulation-instructions 500000000 \
    ~/path/to/traces/600.perlbench_s-210B.champsimtrace.xz
```

Traces used for the 3rd Data Prefetching Championship (DPC-3) are available
[here](https://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/), and CRC-2
traces [here](http://bit.ly/2t2nkUj). If you rely on ChampSim regularly, mirror
your own trace set in case these links break.

To add another branch predictor, prefetcher, or replacement policy, copy an
existing module's directory as a template (e.g. `replacement/lru/` →
`replacement/mypolicy/`), implement it, reference it by name in your config
JSON, then `./config.sh` + `make` again. Tracing utilities for your own
programs live in `tracer/`.
