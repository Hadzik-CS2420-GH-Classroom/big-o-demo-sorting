#!/usr/bin/env python3
"""Generate space-complexity charts from results_space.csv."""

import csv
import subprocess
import sys
import webbrowser
from pathlib import Path

try:
    import plotly.graph_objects as go
    from plotly.subplots import make_subplots
except ImportError:
    print("plotly not found -- installing...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "plotly"])
    import plotly.graph_objects as go
    from plotly.subplots import make_subplots

REPO_DIR = Path(__file__).resolve().parent
CSV_FILE = REPO_DIR / "results_space.csv"
GROWTH_CSV = REPO_DIR / "results_space_growth.csv"
OUTPUT_HTML = REPO_DIR / "charts_space.html"


# -- Build & Run --------------------------------------------------------------

def find_executable():
    out_build = REPO_DIR / "out" / "build"
    if out_build.exists():
        for exe in out_build.rglob("big-o-demo-sorting-space.exe"):
            if "CompilerId" not in str(exe):
                return exe
    candidates = [
        REPO_DIR / "build" / "big-o-demo-sorting-space.exe",
        REPO_DIR / "build" / "Debug" / "big-o-demo-sorting-space.exe",
        REPO_DIR / "build" / "Release" / "big-o-demo-sorting-space.exe",
    ]
    for path in candidates:
        if path.exists():
            return path
    return None


def build_and_run():
    exe = find_executable()
    if exe is None:
        print("No executable found -- building with CMake...")
        build_dir = REPO_DIR / "build"
        subprocess.run(["cmake", "-B", str(build_dir), str(REPO_DIR)], check=True)
        subprocess.run(["cmake", "--build", str(build_dir)], check=True)
        exe = find_executable()
        if exe is None:
            sys.exit("ERROR: build succeeded but executable not found")
    print(f"Running {exe.name}...\n")
    subprocess.run([str(exe)], cwd=str(REPO_DIR), check=True)


# -- Parse CSV -----------------------------------------------------------------

def read_results():
    with open(CSV_FILE) as f:
        return list(csv.DictReader(f))


def group_by_algorithm(rows):
    algos = {}
    for row in rows:
        name = row["structure"]
        if name not in algos:
            algos[name] = {"n": [], "bytes": [], "complexity": ""}
        algos[name]["n"].append(int(row["n"]))
        algos[name]["bytes"].append(int(row["bytes"]))
        algos[name]["complexity"] = row["complexity"]
    return algos


# -- Chart Generation ----------------------------------------------------------

COLORS = {
    "Bubble Sort":    "#dc2626",
    "Insertion Sort": "#ea580c",
    "Selection Sort": "#d97706",
    "Merge Sort":     "#16a34a",
    "Quick Sort":     "#2563eb",
    "Heap Sort":      "#059669",
    "Counting Sort":  "#7c3aed",
    "Bucket Sort":    "#c026d3",
    "Radix Sort":     "#0891b2",
}

# Order for bar chart: grouped by category
BAR_ORDER = [
    "Bubble Sort", "Insertion Sort", "Selection Sort",
    "Merge Sort", "Quick Sort", "Heap Sort",
    "Counting Sort", "Bucket Sort", "Radix Sort",
]

# Algorithms that grow with n (for the growth chart)
# Categories for the growth chart: label, representative algorithm, member list
SPACE_CATEGORIES = [
    {
        "label": "O(1)",
        "rep": "Heap Sort",
        "members": "Bubble, Insertion, Selection,\nHeap, Quick Sort",
        "color": "#c62828",
    },
    {
        "label": "O(n)",
        "rep": "Merge Sort",
        "members": "Merge Sort, Radix Sort",
        "color": "#1b5e20",
    },
    {
        "label": "O(n+k)",
        "rep": "Bucket Sort",
        "members": "Bucket Sort",
        "color": "#c026d3",
    },
    {
        "label": "O(k)",
        "rep": "Counting Sort",
        "members": "Counting Sort\n(k = value range)",
        "color": "#4a148c",
    },
]


def fmt_bytes(b):
    """Human-readable byte string."""
    if b == 0:
        return "0"
    if b >= 1_000_000:
        return f"{b / 1_000_000:.1f} MB"
    if b >= 1_000:
        return f"{b / 1_000:.1f} KB"
    return f"{b} B"


def build_bar_chart(algorithms, target_n):
    """Bar chart: auxiliary bytes at a fixed n, one bar per algorithm."""
    names = []
    bytes_vals = []
    colors = []
    text_labels = []

    for name in BAR_ORDER:
        if name not in algorithms:
            continue
        data = algorithms[name]
        # Find the entry closest to target_n
        idx = min(range(len(data["n"])), key=lambda i: abs(data["n"][i] - target_n))
        b = data["bytes"][idx]
        names.append(name)
        bytes_vals.append(b)
        colors.append(COLORS.get(name, "#3b82f6"))
        text_labels.append("0 (in-place)" if b == 0 else fmt_bytes(b))

    fig = go.Figure(go.Bar(
        x=names,
        y=bytes_vals,
        marker_color=colors,
        text=text_labels,
        textposition="outside",
        textfont=dict(size=12),
    ))

    fig.update_layout(
        title=dict(
            text=f"Extra Memory Allocated During Sort (n={target_n:,})",
            font=dict(size=20),
            subtitle=dict(
                text="Beyond the input array itself (which is always n × 4 bytes)",
                font=dict(size=13, color="#666"),
            ),
        ),
        xaxis=dict(tickangle=-35),
        yaxis_title="Extra bytes allocated",
        yaxis=dict(rangemode="tozero"),
        template="plotly_white",
        font=dict(size=13),
        margin=dict(t=60, b=130),
        height=520,
        showlegend=False,
    )

    # Add category bracket annotations below the rotated tick labels
    brackets = [
        (0, 2, "O(1) In-Place", "#c62828"),
        (3, 5, "O(n log n) Efficient", "#1b5e20"),
        (6, 8, "O(n+k) Non-Comparison", "#4a148c"),
    ]
    for start, end, label, color in brackets:
        mid = (start + end) / 2
        fig.add_annotation(
            x=mid, y=-0.22, xref="x", yref="paper",
            text=f"<b>{label}</b>",
            showarrow=False,
            font=dict(size=12, color=color),
        )

    return fig


def read_growth_results():
    """Read the separate growth CSV (larger sizes, O(n) sorts only)."""
    algos = {}
    with open(GROWTH_CSV) as f:
        for row in csv.DictReader(f):
            name = row["structure"]
            if name not in algos:
                algos[name] = {"n": [], "bytes": [], "complexity": ""}
            algos[name]["n"].append(int(row["n"]))
            algos[name]["bytes"].append(int(row["bytes"]))
            algos[name]["complexity"] = row["complexity"]
    return algos


def build_growth_chart(growth_algos):
    """Grouped bar chart: one cluster per space-complexity class."""
    fig = go.Figure()

    # Get n values from any representative
    any_data = next(iter(growth_algos.values()))
    n_values = sorted(set(any_data["n"]))

    shades = ["#dbeafe", "#93c5fd", "#60a5fa", "#2563eb", "#1e3a8a"]
    size_shades = {n: shades[min(i, len(shades) - 1)]
                   for i, n in enumerate(n_values)}

    # X-axis labels are the Big O categories
    cat_labels = [cat["label"] for cat in SPACE_CATEGORIES]

    for n_val in n_values:
        bytes_vals = []
        text_labels = []
        for cat in SPACE_CATEGORIES:
            rep = cat["rep"]
            if rep in growth_algos:
                data = growth_algos[rep]
                idx = data["n"].index(n_val)
                b = data["bytes"][idx]
            else:
                b = 0
            bytes_vals.append(b)
            text_labels.append(fmt_bytes(b))

        fig.add_trace(go.Bar(
            x=cat_labels,
            y=bytes_vals,
            name=f"n={n_val:,}",
            marker_color=size_shades.get(n_val, "#3b82f6"),
            text=text_labels,
            textposition="outside",
            textfont=dict(size=10),
        ))

    # Add "which algorithms?" annotations below each cluster
    for i, cat in enumerate(SPACE_CATEGORIES):
        fig.add_annotation(
            x=i, y=-0.15, xref="x", yref="paper",
            text=f"<i>{cat['members']}</i>",
            showarrow=False,
            font=dict(size=10, color=cat["color"]),
            align="center",
        )

    fig.update_layout(
        title=dict(
            text="Extra Memory by Space Complexity Class",
            font=dict(size=20),
            subtitle=dict(
                text="One representative per class, tested at increasing input sizes",
                font=dict(size=13, color="#666"),
            ),
        ),
        barmode="group",
        yaxis_title="Extra bytes allocated",
        yaxis=dict(rangemode="tozero"),
        template="plotly_white",
        font=dict(size=13),
        legend=dict(
            title=dict(text="Input size"),
            font=dict(size=13),
        ),
        margin=dict(t=70, b=100),
        height=520,
    )
    return fig


def generate_html(algorithms):
    # Get the largest n for the bar chart
    any_algo = next(iter(algorithms.values()))
    target_n = max(any_algo["n"])

    bar_fig = build_bar_chart(algorithms, target_n)
    growth_data = read_growth_results()
    growth_fig = build_growth_chart(growth_data)

    bar_div = bar_fig.to_html(full_html=False, include_plotlyjs=True)
    growth_div = growth_fig.to_html(full_html=False, include_plotlyjs=False)

    input_bytes = target_n * 4  # sizeof(int) = 4

    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Big O Demo: Sorting -- Space Complexity</title>
<style>
  body {{ background: #ffffff; color: #1a1a1a; font-family: system-ui, sans-serif;
         max-width: 960px; margin: 0 auto; padding: 2rem; }}
  h1 {{ text-align: center; margin-bottom: 0.25rem; }}
  p.sub {{ text-align: center; color: #666; margin-top: 0; }}
  .chart {{ margin-bottom: 1.5rem; }}
  .panel {{ border-radius: 10px; padding: 1.25rem 1.5rem; margin-bottom: 1.25rem;
            font-size: .9rem; line-height: 1.6; }}
  .panel h3 {{ margin: 0 0 .6rem; font-size: 1rem; }}
  .panel ul {{ padding-left: 1.3rem; margin: .4rem 0; }}
  .panel li {{ margin-bottom: .3rem; }}
  .panel code {{ background: #e8e8e8; padding: .1em .35em; border-radius: 4px;
                 font-family: Consolas, monospace; font-size: .88em; }}
</style>
</head>
<body>
<h1>Big O Demo: Sorting -- Space Complexity</h1>
<p class="sub">Every sort receives the same input array ({target_n:,} ints = {input_bytes:,} bytes).
The chart below shows how much <em>extra</em> memory each algorithm allocates on top of that.</p>

<div class="chart">{bar_div}</div>

<!-- WHY ZERO -->
<div class="panel" style="background: #fce4ec; border-left: 5px solid #c62828;">
  <h3 style="color: #c62828;">Why are bubble, insertion, selection, heap, and quick sort at zero?</h3>
  <p>These are <strong>in-place</strong> algorithms &mdash; they sort by swapping and shifting
  elements inside the existing array. They only use a handful of local variables
  (<code>int temp</code>, <code>int i</code>, etc.) which live on the <strong>stack</strong>,
  not the heap. No temporary arrays, no copies &mdash; zero extra heap allocation.</p>
  <ul>
    <li><strong>Bubble sort:</strong> swaps adjacent elements &mdash; needs one <code>int</code> temp variable</li>
    <li><strong>Insertion sort:</strong> shifts elements right &mdash; needs one <code>int key</code> variable</li>
    <li><strong>Selection sort:</strong> swaps min into place &mdash; needs one <code>int min_idx</code></li>
    <li><strong>Heap sort:</strong> builds a max-heap in the existing array, then extracts max
    repeatedly &mdash; all swaps happen in-place, zero extra allocation</li>
    <li><strong>Quick sort:</strong> partitions in-place &mdash; zero heap allocations, but uses
    <strong>O(log n) stack frames</strong> for recursion (~{int(14 * 64):,} bytes at n={target_n:,}), which this tracker does not measure</li>
  </ul>
</div>

<!-- WHY ~40 KB -->
<div class="panel" style="background: #e8f5e9; border-left: 5px solid #1b5e20;">
  <h3 style="color: #1b5e20;">Why do merge sort, bucket sort, and radix sort use ~40 KB?</h3>
  <p>These algorithms need <strong>temporary arrays</strong> to do their work:</p>
  <ul>
    <li><strong>Merge sort &mdash; O(n):</strong> the merge step copies both halves into temporary
    vectors, then merges them back. At the top level, that's n ints =
    {target_n:,} &times; 4 bytes = <strong>{target_n * 4:,} bytes</strong>.</li>
    <li><strong>Radix sort &mdash; O(n):</strong> each digit pass copies all n elements into an
    output array. That's also {target_n:,} &times; 4 = <strong>{target_n * 4:,} bytes</strong>,
    plus a small 10-element count array per pass.</li>
    <li><strong>Bucket sort &mdash; O(n+k):</strong> distributes all n elements across 10 bucket
    vectors. The elements themselves total n &times; 4 bytes, plus each
    <code>std::vector</code> has ~24 bytes of bookkeeping overhead.</li>
  </ul>
  <p>The key pattern: <strong>double n, double the extra memory</strong>. That's O(n) space.</p>
</div>

<!-- WHY 4 MB -->
<div class="panel" style="background: #f3e5f5; border-left: 5px solid #4a148c;">
  <h3 style="color: #4a148c;">Why does counting sort use ~4 MB &mdash; 100&times; more than the others?</h3>
  <p>Counting sort allocates a <strong>count array</strong> sized to the <em>range of values</em>,
  not the number of elements. In this demo, values range from 1 to 1,000,000:</p>
  <ul>
    <li>Count array size: <strong>k = 1,000,000 ints = 4,000,000 bytes &asymp; 4 MB</strong></li>
    <li>This is the same whether n is 1,000 or 10,000 &mdash; k doesn't change</li>
    <li>If the values only ranged from 1 to 100, the count array would be just 400 bytes</li>
  </ul>
  <p>This is the <strong>space-time tradeoff</strong> of counting sort: it's blazing fast
  (O(n+k) time, no comparisons), but the count array can be enormous if the value
  range is large relative to n. At n=10,000 with k=1,000,000, the count array is
  <strong>100&times; larger than the input itself</strong>.</p>
</div>

<div class="chart">{growth_div}</div>

<div class="panel" style="background: #e3f2fd; border-left: 5px solid #1565c0;">
  <h3 style="color: #1565c0;">What to look for in Chart 2</h3>
  <p>Each cluster represents a <strong>space complexity class</strong>, tested at larger
  sizes (up to 500,000) so the growth patterns are obvious. The italic text below
  each cluster lists which algorithms belong to that class.</p>
  <ul>
    <li><strong>O(1):</strong> all bars are zero &mdash; no matter how large n gets,
    in-place sorts never allocate extra memory</li>
    <li><strong>O(n):</strong> bars grow proportionally &mdash; double n, double the
    extra memory (~4 bytes per element for merge sort and radix sort)</li>
    <li><strong>O(n+k):</strong> similar growth to O(n) plus per-bucket overhead from
    <code>std::vector</code> bookkeeping</li>
    <li><strong>O(k):</strong> all bars are the same ~4 MB regardless of n. Why?
    Counting sort allocates one counter for every <em>possible value</em>, not every
    element. In this demo, values range from 1 to 1,000,000 so k=1,000,000 &mdash;
    that's a million-entry array even when sorting just 10,000 elements. The
    input size n doesn't affect this cost at all; only the value range k does.</li>
  </ul>
</div>

</body>
</html>"""

    OUTPUT_HTML.write_text(html, encoding="utf-8")
    print(f"\nCharts written to {OUTPUT_HTML}")
    webbrowser.open(OUTPUT_HTML.as_uri())


# -- Main ----------------------------------------------------------------------

if __name__ == "__main__":
    if "--graph-only" not in sys.argv:
        build_and_run()
    else:
        if not CSV_FILE.exists() or not GROWTH_CSV.exists():
            sys.exit(f"ERROR: CSV files not found -- run without --graph-only first")
        print("Skipping build/run -- using existing CSV files")
    rows = read_results()
    algorithms = group_by_algorithm(rows)
    generate_html(algorithms)
