#!/usr/bin/env python3
import math
import shutil
import subprocess
import sys
from pathlib import Path
from time import perf_counter

FALLBACK_RUNS = 100


def clean_output(b: bytes) -> bytes:
    drop_prefixes = (
        b"[PE",
        b"[CODE",
        b"[COMPILING",
        b"[RUNNING",
    )
    return b"".join(
        line for line in b.splitlines(True)
        if not any(line.strip().startswith(p) for p in drop_prefixes)
    )


def reset_dir(dir_path: Path) -> None:
    if dir_path.exists():
        shutil.rmtree(dir_path)
    dir_path.mkdir()


def test_sources(test_dir: Path):
    return sorted(test_dir.rglob("*.morpho"))


def output_paths(root: Path, test_dir: Path, src: Path):
    rel = src.relative_to(test_dir)
    out = root / rel.with_suffix(".output")
    time_file = root / rel.with_suffix(".time")

    out.parent.mkdir(parents=True, exist_ok=True)
    time_file.parent.mkdir(parents=True, exist_ok=True)

    return out, time_file


def fmt_time(x: float) -> str:
    return "nan" if math.isnan(x) else f"{x:.9f}"


def fmt_ratio(numer: float, denom: float) -> str:
    if math.isnan(numer) or math.isnan(denom) or denom == 0.0:
        return "n/a"
    return f"{(numer / denom) * 100:.2f}%"


def read_time_file(path: Path) -> tuple[float, float]:
    lines = path.read_text().strip().splitlines()
    if len(lines) < 2:
        raise ValueError("time file has fewer than 2 lines")
    return float(lines[0]), float(lines[1])


def parse_morphoit_stderr(stderr_bytes: bytes):
    text = stderr_bytes.decode("utf-8", errors="replace")
    total = None
    run = None
    kept_lines = []

    for line in text.splitlines(True):
        stripped = line.strip()

        if stripped.startswith("__MI_TOTAL__"):
            total = float(stripped.split()[1])
        elif stripped.startswith("__MI_RUN__"):
            run = float(stripped.split()[1])
        else:
            kept_lines.append(line)

    if total is None or run is None:
        raise ValueError("Missing markers")

    cleaned_stderr = "".join(kept_lines).encode("utf-8", errors="replace")
    return total, run, cleaned_stderr


def run_morphoit_once(src: Path, out_path: Path):
    cmd = ["python3", "morphoit.py", "-p", str(src)]

    with out_path.open("wb") as f_out:
        result = subprocess.run(cmd, stdout=f_out, stderr=subprocess.PIPE)

    try:
        total, run, cleaned_stderr = parse_morphoit_stderr(result.stderr)
    except Exception:
        total, run = float("nan"), float("nan")
        cleaned_stderr = result.stderr

    return result.returncode, total, run, cleaned_stderr


def run_once_baseline(src: Path, out_path: Path):
    start = perf_counter()

    with out_path.open("wb") as f_out:
        result = subprocess.run(["morpho6", str(src)], stdout=f_out)

    elapsed = perf_counter() - start
    return result.returncode, elapsed


def time_batch_average(src: Path, n: int) -> float:
    start = perf_counter()

    for _ in range(n):
        subprocess.run(
            ["morpho6", str(src)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

    return (perf_counter() - start) / n


def generate_baseline(test_dir: Path, baseline_dir: Path):
    reset_dir(baseline_dir)
    failures = 0

    sources = test_sources(test_dir)
    if not sources:
        print("No .morpho files found in ./tests")
        return 0

    for src in sources:
        out, time_file = output_paths(baseline_dir, test_dir, src)

        rc, elapsed = run_once_baseline(src, out)

        if rc == 0 and elapsed == 0.0:
            elapsed = time_batch_average(src, FALLBACK_RUNS)

        with time_file.open("w") as f:
            f.write(f"{elapsed:.9f}\n")
            f.write(f"{elapsed:.9f}\n")

        if rc != 0:
            failures += 1

        rel = src.relative_to(test_dir)
        print(f"[BASELINE {'OK' if rc == 0 else 'FAIL'}] {rel}")

    return 1 if failures else 0


def run_comparison(test_dir: Path, baseline_dir: Path, output_dir: Path):
    reset_dir(output_dir)
    failures = 0

    for src in test_sources(test_dir):
        b_out, b_time = output_paths(baseline_dir, test_dir, src)
        o_out, _ = output_paths(output_dir, test_dir, src)

        rc, total, run, stderr = run_morphoit_once(src, o_out)

        status = "MATCH"
        reason = None

        if rc != 0:
            status = "ERROR (runtime)"
            reason = f"nonzero return code: {rc}"
        elif clean_output(b_out.read_bytes()) != clean_output(o_out.read_bytes()):
            status = "ERROR (output mismatch)"
            reason = "program output differs from morpho6 baseline"

        try:
            baseline_total, baseline_run = read_time_file(b_time)
        except Exception:
            baseline_total, baseline_run = float("nan"), float("nan")

        rel = src.relative_to(test_dir)
        print(f"{rel}: [{status}]")

        if reason:
            print(f"Reason: {reason}")

        print(
            f"Total time ratio: {fmt_time(total)} / {fmt_time(baseline_total)} = "
            f"{fmt_ratio(total, baseline_total)}"
        )
        print(
            f"Runtime ratio:    {fmt_time(run)} / {fmt_time(baseline_run)} = "
            f"{fmt_ratio(run, baseline_run)}"
        )

        if status != "MATCH":
            failures += 1
            if stderr:
                print("--- STDERR ---")
                print(stderr.decode("utf-8", errors="replace").strip())

        print()

    return 1 if failures else 0


def main():
    test_dir = Path("tests")
    baseline_dir = Path("baseline")
    output_dir = Path("output")

    if not test_dir.is_dir():
        print("Error: expected ./tests directory to exist.", file=sys.stderr)
        return 2

    print("Regenerating baseline...")
    baseline_rc = generate_baseline(test_dir, baseline_dir)

    if baseline_rc != 0:
        print("Error: baseline generation failed.", file=sys.stderr)
        return 2

    print("Baseline regenerated.\n")

    return run_comparison(test_dir, baseline_dir, output_dir)


if __name__ == "__main__":
    raise SystemExit(main())