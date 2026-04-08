#!/usr/bin/env python3
import math
import shutil
import subprocess
import sys
from pathlib import Path

FALLBACK_RUNS = 100


def reset_dir(dir_path: Path) -> None:
    if dir_path.exists():
        if dir_path.is_dir():
            shutil.rmtree(dir_path)
        else:
            print(f"Error: {dir_path} exists but is not a directory.", file=sys.stderr)
            raise SystemExit(2)
    dir_path.mkdir()


def parse_gtime(stderr_bytes: bytes) -> tuple[float, bytes]:
    """
    Parse gtime output:
      __CPU__ <user> <sys>
      __WALL__ <real>

    Returns:
      (wall_seconds, stderr_without_time_lines)
    """
    text = stderr_bytes.decode("utf-8", errors="replace")
    real = None
    kept_lines: list[str] = []

    for line in text.splitlines(True):
        s = line.strip()
        if s.startswith("__CPU__ "):
            pass
        elif s.startswith("__WALL__ "):
            real = float(s.split()[1])
        else:
            kept_lines.append(line)

    if real is None:
        raise ValueError("Failed to parse gtime output (missing WALL marker).")

    cleaned = "".join(kept_lines).encode("utf-8", errors="replace")
    return real, cleaned


def parse_morphoit_stderr(stderr_bytes: bytes) -> tuple[float, float, bytes]:
    """
    Parse morphoit.py stderr markers:
      __MI_TOTAL__ <seconds>
      __MI_RUN__ <seconds>

    Returns:
      (total_seconds, run_seconds, stderr_without_marker_lines)
    """
    text = stderr_bytes.decode("utf-8", errors="replace")
    total = None
    run = None
    kept_lines: list[str] = []

    for line in text.splitlines(True):
        s = line.strip()
        if s.startswith("__MI_TOTAL__ "):
            total = float(s.split()[1])
        elif s.startswith("__MI_RUN__ "):
            run = float(s.split()[1])
        else:
            kept_lines.append(line)

    if total is None or run is None:
        raise ValueError("Failed to parse morphoit timing output.")

    cleaned = "".join(kept_lines).encode("utf-8", errors="replace")
    return total, run, cleaned


def run_once_with_output(cmd_inner: list[str], out_path: Path) -> tuple[int, float, bytes]:
    """
    Run cmd_inner once with gtime, capturing stdout into out_path.

    Returns:
      (returncode, wall_seconds, cleaned_stderr)
    """
    cmd = ["gtime", "-f", "__CPU__ %U %S\n__WALL__ %e"] + cmd_inner
    with out_path.open("wb") as f_out:
        result = subprocess.run(cmd, stdout=f_out, stderr=subprocess.PIPE, check=False)

    runtime_s, cleaned = parse_gtime(result.stderr)
    return result.returncode, runtime_s, cleaned


def run_morphoit_once(src: Path, out_path: Path) -> tuple[int, float, float, bytes]:
    """
    Run morphoit.py once, capturing stdout into out_path.

    Returns:
      (returncode, total_time_seconds, run_time_seconds, cleaned_stderr)
    """
    cmd = ["python3", "morphoit.py", "-p", str(src)]
    with out_path.open("wb") as f_out:
        result = subprocess.run(cmd, stdout=f_out, stderr=subprocess.PIPE, check=False)

    total_s, run_s, cleaned = parse_morphoit_stderr(result.stderr)
    return result.returncode, total_s, run_s, cleaned


def time_batch_average(cmd_inner: list[str], n: int) -> float:
    """
    Time cmd_inner repeated n times as one timed batch (stdout discarded),
    return average runtime per run.
    """
    def sh_quote(s: str) -> str:
        return "'" + s.replace("'", "'\"'\"'") + "'"

    cmd_str = " ".join(sh_quote(x) for x in cmd_inner)
    loop = f'i=0; while [ "$i" -lt {n} ]; do {cmd_str} >/dev/null; i=$((i+1)); done'

    cmd = ["gtime", "-f", "__CPU__ %U %S\n__WALL__ %e", "sh", "-c", loop]
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, check=False)

    wall_total, _ = parse_gtime(result.stderr)
    return wall_total / n


def read_time_file(path: Path) -> tuple[float, float]:
    """
    Read two-line time file.
    """
    lines = path.read_text().strip().splitlines()
    if len(lines) < 2:
        raise ValueError("time file has fewer than 2 lines")
    return float(lines[0].strip()), float(lines[1].strip())


def fmt_ratio(numer: float, denom: float) -> str:
    if any(map(math.isnan, (numer, denom))) or denom == 0.0:
        return "n/a"
    return f"{(numer / denom) * 100:.2f}%"


def generate_baseline(test_dir: Path, baseline_dir: Path) -> int:
    """
    Rebuild baseline/ from scratch using morpho6.
    Each baseline .time file stores the baseline runtime twice:
      line 1 -> denominator for total time ratio
      line 2 -> denominator for runtime ratio
    """
    reset_dir(baseline_dir)

    morpho_files = sorted(test_dir.glob("*.morpho"))
    if not morpho_files:
        print("No .morpho files found in ./tests")
        return 0

    failures = 0

    for src in morpho_files:
        out_path = baseline_dir / f"{src.stem}.output"
        time_path = baseline_dir / f"{src.stem}.time"

        try:
            rc, runtime_s, cleaned_stderr = run_once_with_output(["morpho6", str(src)], out_path)
        except FileNotFoundError:
            print("Error: command not found: gtime or morpho6", file=sys.stderr)
            return 127
        except Exception as e:
            print(f"Error: failed running baseline for {src.name}: {e}", file=sys.stderr)
            rc, runtime_s, cleaned_stderr = 1, float("nan"), b""

        if rc == 0 and runtime_s == 0.0:
            try:
                runtime_s = time_batch_average(["morpho6", str(src)], FALLBACK_RUNS)
            except Exception as e:
                print(f"Error: batch timing failed for baseline {src.name}: {e}", file=sys.stderr)
                runtime_s = float("nan")

        with time_path.open("w") as f_time:
            if math.isnan(runtime_s):
                f_time.write("nan\nnan\n")
            else:
                f_time.write(f"{runtime_s:.9f}\n{runtime_s:.9f}\n")

        if rc != 0:
            failures += 1
            with out_path.open("ab") as f_out:
                f_out.write(b"\n\n--- STDERR ---\n")
                f_out.write(cleaned_stderr)
            print(f"[BASELINE FAIL {rc}] {src.name}", file=sys.stderr)
        else:
            print(f"[BASELINE OK] {src.name}")

    return 1 if failures else 0


def run_comparison(test_dir: Path, baseline_dir: Path, output_dir: Path) -> int:
    morpho_files = sorted(test_dir.glob("*.morpho"))
    if not morpho_files:
        print("No .morpho files found in ./tests")
        return 0

    failures = 0

    for src in morpho_files:
        b_out = baseline_dir / f"{src.stem}.output"
        b_time = baseline_dir / f"{src.stem}.time"
        o_out = output_dir / f"{src.stem}.output"
        o_time = output_dir / f"{src.stem}.time"

        status = "MATCH"
        error_reason = None

        if not b_out.is_file() or not b_time.is_file():
            status = "ERROR"
            error_reason = "missing baseline .output/.time"

        try:
            rc, total_mi, run_mi, cleaned_stderr = run_morphoit_once(src, o_out)
        except FileNotFoundError:
            print("Error: command not found: python3 and/or morphoit.py", file=sys.stderr)
            return 127
        except Exception as e:
            rc, total_mi, run_mi, cleaned_stderr = 1, float("nan"), float("nan"), b""
            status = "ERROR"
            error_reason = error_reason or f"morphoit run failed ({e})"

        with o_time.open("w") as f:
            if math.isnan(total_mi) or math.isnan(run_mi):
                f.write("nan\nnan\n")
            else:
                f.write(f"{total_mi:.9f}\n{run_mi:.9f}\n")

        if status != "ERROR":
            try:
                if b_out.read_bytes() != o_out.read_bytes():
                    status = "ERROR"
                    error_reason = "output mismatch"
            except Exception as e:
                status = "ERROR"
                error_reason = f"output compare failed ({e})"

        if rc != 0:
            with o_out.open("ab") as f_out:
                f_out.write(b"\n\n--- STDERR ---\n")
                f_out.write(cleaned_stderr)

        try:
            baseline_total_denom, baseline_runtime_denom = read_time_file(b_time)
        except Exception as e:
            baseline_total_denom, baseline_runtime_denom = float("nan"), float("nan")
            if status != "ERROR":
                status = "ERROR"
                error_reason = error_reason or f"could not parse baseline time ({e})"

        print(f"{src.name}: [{status}]")
        if status == "ERROR" and error_reason:
            print(f"Reason: {error_reason}")

        total_ratio = fmt_ratio(total_mi, baseline_total_denom)
        runtime_ratio = fmt_ratio(run_mi, baseline_runtime_denom)

        total_mi_s = "nan" if math.isnan(total_mi) else f"{total_mi:.9f}"
        run_mi_s = "nan" if math.isnan(run_mi) else f"{run_mi:.9f}"
        baseline_total_s = "nan" if math.isnan(baseline_total_denom) else f"{baseline_total_denom:.9f}"
        baseline_runtime_s = "nan" if math.isnan(baseline_runtime_denom) else f"{baseline_runtime_denom:.9f}"

        print(f"Total time ratio: {total_mi_s} / {baseline_total_s} = {total_ratio}")
        print(f"Runtime ratio:    {run_mi_s} / {baseline_runtime_s} = {runtime_ratio}")
        print()

        if status == "ERROR" or rc != 0:
            failures += 1

    return 1 if failures else 0


def main() -> int:
    test_dir = Path("tests")
    baseline_dir = Path("baseline")
    output_dir = Path("output")

    if not test_dir.is_dir():
        print("Error: expected ./tests directory to exist.", file=sys.stderr)
        return 2

    print("Regenerating baseline...")
    baseline_rc = generate_baseline(test_dir, baseline_dir)
    if baseline_rc == 127:
        return 127
    if baseline_rc != 0:
        print("Error: baseline generation failed.", file=sys.stderr)
        return 2
    print("Baseline regenerated.\n")

    reset_dir(output_dir)
    return run_comparison(test_dir, baseline_dir, output_dir)


if __name__ == "__main__":
    raise SystemExit(main())