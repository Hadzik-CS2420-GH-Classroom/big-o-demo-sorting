#!/usr/bin/env python3
"""Generate space-complexity charts from results_space.csv."""

import csv
import subprocess
import sys
import webbrowser
from pathlib import Path

try:
    import plotly.graph_objects as go
except ImportError:
    print("plotly not found -- installing...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "plotly"])
    import plotly.graph_objects as go

REPO_DIR = Path(__file__).resolve().parent
CSV_FILE = REPO_DIR / "results_space.csv"
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
    "Counting Sort":  "#7c3aed",
    "Bucket Sort":    "#c026d3",
    "Radix Sort":     "#0891b2",
}

LEGEND_GROUPS = [
    {
        "group": "quadratic",
        "title": "O(1) In-Place",
        "algos": ["Bubble Sort", "Insertion Sort", "Selection Sort"],
    },
    {
        "group": "efficient",
        "title": "O(n) / O(log n)",
        "algos": ["Merge Sort", "Quick Sort"],
    },
    {
        "group": "non-comparison",
        "title": "O(k) / O(n+k)",
        "algos": ["Counting Sort", "Bucket Sort", "Radix Sort"],
    },
]


def build_chart(algorithms):
    fig = go.Figure()

    for grp in LEGEND_GROUPS:
        for name in grp["algos"]:
            if name not in algorithms:
                continue
            data = algorithms[name]
            fig.add_trace(go.Scatter(
                x=data["n"],
                y=data["bytes"],
                mode="lines+markers",
                name=f"{name} -- {data['complexity']}",
                legendgroup=grp["group"],
                legendgrouptitle=dict(
                    text=grp["title"], font=dict(size=13, color="#555"),
                ),
                line=dict(color=COLORS.get(name, "#3b82f6"), width=3),
                marker=dict(size=10),
            ))

    fig.update_layout(
        title=dict(
            text="Sorting Algorithms: Auxiliary Space",
            font=dict(size=22),
        ),
        xaxis_title="n (input size)",
        yaxis_title="Auxiliary bytes (heap)",
        yaxis=dict(rangemode="tozero"),
        template="plotly_white",
        font=dict(size=14),
        legend=dict(
            font=dict(size=13),
            groupclick="togglegroup",
            tracegroupgap=12,
        ),
        margin=dict(t=60, b=60),
        height=600,
    )
    return fig


def generate_html(algorithms):
    fig = build_chart(algorithms)
    chart_div = fig.to_html(full_html=False, include_plotlyjs=True)

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
  .chart {{ margin-bottom: 2rem; }}
  .note {{ background: #f8f9fa; border-left: 4px solid #6c757d;
           border-radius: 6px; padding: 1rem 1.25rem; margin: 1.5rem 0;
           font-size: .9rem; color: #495057; }}
</style>
</head>
<body>
<h1>Big O Demo: Sorting -- Space Complexity</h1>
<p class="sub">Auxiliary heap memory used by each algorithm</p>
<div class="chart">{chart_div}</div>
<div class="note">
  <strong>What this measures:</strong> peak heap memory allocated <em>during</em> the sort,
  above the input array.  In-place sorts (bubble, insertion, selection) allocate zero
  extra heap memory.  Quick sort uses O(log n) <strong>stack</strong> space for recursion,
  which is not tracked here.  Counting sort's space depends on the value range (k),
  not the input size (n) -- that's why its line is flat.
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
        if not CSV_FILE.exists():
            sys.exit(f"ERROR: {CSV_FILE} not found -- run without --graph-only first")
        print("Skipping build/run -- using existing results_space.csv")
    rows = read_results()
    algorithms = group_by_algorithm(rows)
    generate_html(algorithms)
