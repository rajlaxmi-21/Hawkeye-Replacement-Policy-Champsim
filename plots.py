#!/usr/bin/env python3
"""Generate the two required plots for the Hawkeye assignment.

Plot 1: LLC miss rate vs. associativity (4/8/16-way), LRU vs Hawkeye,
        on 456.hmmer-191B -- reads logs from results3/.
Plot 2: LLC miss-rate reduction (%) over LRU, per benchmark, at the
        default LLC config -- reads logs from results4/.
"""

import re
import matplotlib.pyplot as plt

LLC_LINE_RE = re.compile(
    r"LLC TOTAL\s+ACCESS:\s+(\d+)\s+HIT:\s+(\d+)\s+MISS:\s+(\d+)"
)


def miss_rate(log_path):
    """Return LLC miss rate (%) parsed from a champsim log file."""
    with open(log_path) as f:
        text = f.read()
    match = LLC_LINE_RE.search(text)
    if not match:
        raise ValueError(f"Could not find LLC TOTAL line in {log_path}")
    access, _hit, miss = (int(x) for x in match.groups())
    return 100.0 * miss / access


def plot1():
    associativities = [4, 8, 16]
    lru_rates = [miss_rate(f"results3/llc{w}way_lru.log") for w in associativities]
    hawkeye_rates = [miss_rate(f"results3/llc{w}way_hawkeye.log") for w in associativities]

    fig, ax = plt.subplots(figsize=(6, 4.5))
    ax.plot(associativities, lru_rates, marker="o", label="LRU")
    ax.plot(associativities, hawkeye_rates, marker="o", label="Hawkeye")
    ax.set_xticks(associativities)
    ax.set_xlabel("LLC Associativity (ways)")
    ax.set_ylabel("LLC Miss Rate (%)")
    ax.set_title("LLC Miss Rate vs. Associativity (456.hmmer-191B)")
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig("plot1_missrate_vs_associativity.png", dpi=150)
    print("Wrote plot1_missrate_vs_associativity.png")
    for w, l, h in zip(associativities, lru_rates, hawkeye_rates):
        print(f"  {w}-way: LRU={l:.2f}%  Hawkeye={h:.2f}%")


def plot2():
    benchmarks = ["456.hmmer-191B", "429.mcf-22B", "473.astar-42B"]
    reductions = []
    for bench in benchmarks:
        lru = miss_rate(f"results4/plot2_{bench}_lru.log")
        hawkeye = miss_rate(f"results4/plot2_{bench}_hawkeye.log")
        reduction = (lru - hawkeye) / lru * 100.0
        reductions.append(reduction)

    fig, ax = plt.subplots(figsize=(6, 4.5))
    colors = ["tab:green" if r >= 0 else "tab:red" for r in reductions]
    ax.bar(benchmarks, reductions, color=colors)
    ax.axhline(0, color="black", linewidth=0.8)
    ax.set_ylabel("LLC Miss-Rate Reduction over LRU (%)")
    ax.set_title("Hawkeye Miss-Rate Reduction over LRU (default LLC: 2MB, 16-way)")
    ax.grid(True, axis="y", alpha=0.3)
    fig.tight_layout()
    fig.savefig("plot2_missrate_reduction.png", dpi=150)
    print("Wrote plot2_missrate_reduction.png")
    for bench, r in zip(benchmarks, reductions):
        print(f"  {bench}: {r:+.2f}%")


if __name__ == "__main__":
    plot1()
    plot2()
