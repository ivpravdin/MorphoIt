#!/usr/bin/env python3
import shutil
import subprocess
from pathlib import Path
import sys
import math

FALLBACK_RUNS = 100

def reset_dir(dir_path: Path) -> None:
    if dir_path.exists():
        if dir_path.is_dir():
            shutil.rmtree(dir_path)
        else:
            print(f"Error: {dir_path} exists but is not a directory.", file=sys.stderr)
            raise SystemExit(2)
    dir_path.mkdir()

def parse_gtime(stderr_bytes: bytes) -> tuple[float, float, bytes]:
    """
    Parse gtime output in custom format:
      __CPU__ <user> <sys>
      __WALL__ <real>
    Returns (cpu_seconds=user+sys, wall_seconds=real, stderr_without_time_lines).
    """
    text = stderr_bytes.decode("utf-8", errors="replace")
    user = sys_t = real = None
    kept_lines: list[str] = []

    for line in text.splitlines(True):
        s = line.strip()
        if s.startswith("__CPU__ "):
            parts = s.split()
            user = float(parts[1])
            sys_t = float(parts[2])
        elif s.startswith("__WALL__ "):
            real = float(s.split()[1])
        else:
            kept_lines.append(line)

    if user is None or sys_t is None or real is None:
        raise ValueError("Failed to parse gtime output (missing CPU/WALL markers).")

    cpu = user + sys_t
    cleaned = "".join(kept_lines).encode("utf-8", errors="replace")
    return cpu, real, cleaned

def run_once_with_output(cmd_inner: list[str], out_path: Path) -> tuple[int, float, float, bytes]:
    """
    Run cmd_inner once, capturing stdout into out_path, timing with gtime.
    Returns (returncode, cpu, wall, cleaned_stderr).
    """
    cmd = ["gtime", "-f", "__CPU__ %U %S\n__WALL__ %e"] + cmd_inner
    with out_path.open("wb") as f_out:
        result = subprocess.run(cmd, stdout=f_out, stderr=subprocess.PIPE, check=False)
    cpu, wall, cleaned = parse_gtime(result.stderr)
    return result.returncode, cpu, wall, cleaned

def time_batch_average(cmd_inner: list[str], n: int) -> tuple[float, float]:
    """
    Time cmd_inner repeated n times as one timed batch (stdout discarded),
    return (avg_cpu_per_run, avg_wall_per_run).
    """
    def sh_quote(s: str) -> str:
        return "'" + s.replace("'", "'\"'\"'") + "'"

    cmd_str = " ".join(sh_quote(x) for x in cmd_inner)
    loop = f'i=0; while [ "$i" -lt {n} ]; do {cmd_str} >/dev/null; i=$((i+1)); done'

    cmd = ["gtime", "-f", "__CPU__ %U %S\n__WALL__ %e", "sh", "-c", loop]
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, check=False)

    cpu_total, wall_total, _ = parse_gtime(result.stderr)
    return cpu_total / n, wall_total / n

def read_time_file(path: Path) -> tuple[float, float]:
    """
    Read two-line time file: line1 CPU, line2 wall.
    """
    lines = path.read_text().strip().splitlines()
    if len(lines) < 2:
        raise ValueError("time file has fewer than 2 lines")
    return float(lines[0].strip()), float(lines[1].strip())

def fmt_ratio(numer: float, denom: float) -> str:
    if any(map(math.isnan, (numer, denom))) or denom == 0.0:
        return "n/a"
    return f"{(numer / denom) * 100:.2f}%"

def baseline_complete(test_dir: Path, baseline_dir: Path) -> bool:
    """
    Baseline is 'complete' if for every tests/*.morpho there is a matching
    baseline/<stem>.output and baseline/<stem>.time.
    """
    for src in test_dir.glob("*.morpho"):
        if not (baseline_dir / f"{src.stem}.output").is_file():
            return False
        if not (baseline_dir / f"{src.stem}.time").is_file():
            return False
    return True

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

        # Run MorphoIt once + time
        morphoit_cmd = ["python3", "morphoit.py", str(src)]
        try:
            rc, cpu_mi, wall_mi, cleaned_stderr = run_once_with_output(morphoit_cmd, o_out)
        except FileNotFoundError:
            print("Error: command not found: gtime and/or python3 and/or morphoit.py", file=sys.stderr)
            return 127
        except Exception as e:
            rc, cpu_mi, wall_mi, cleaned_stderr = 1, float("nan"), float("nan"), b""
            status = "ERROR"
            error_reason = error_reason or f"morphoit run failed ({e})"

        # Fallback: if first measurement is all zeros, measure an N-run batch and average.
        if rc == 0 and cpu_mi == 0.0 and wall_mi == 0.0 and status != "ERROR":
            try:
                cpu_mi, wall_mi = time_batch_average(morphoit_cmd, FALLBACK_RUNS)
            except Exception as e:
                cpu_mi, wall_mi = float("nan"), float("nan")
                status = "ERROR"
                error_reason = error_reason or f"batch timing failed ({e})"

        # Write MorphoIt time file
        with o_time.open("w") as f:
            if math.isnan(cpu_mi) or math.isnan(wall_mi):
                f.write("nan\nnan\n")
            else:
                f.write(f"{cpu_mi:.9f}\n{wall_mi:.9f}\n")

        # Compare outputs if we can
        if status != "ERROR":
            try:
                if b_out.read_bytes() != o_out.read_bytes():
                    status = "ERROR"
                    error_reason = "output mismatch"
            except Exception as e:
                status = "ERROR"
                error_reason = f"output compare failed ({e})"

        # Append stderr to output on failure
        if rc != 0:
            with o_out.open("ab") as f_out:
                f_out.write(b"\n\n--- STDERR ---\n")
                f_out.write(cleaned_stderr)

        # Read baseline times for reporting
        try:
            cpu_m6, wall_m6 = read_time_file(b_time)
        except Exception as e:
            cpu_m6, wall_m6 = float("nan"), float("nan")
            if status != "ERROR":
                status = "ERROR"
                error_reason = error_reason or f"could not parse baseline time ({e})"

        # Print per-test summary
        print(f"{src.name}: [{status}]")
        if status == "ERROR" and error_reason:
            print(f"Reason: {error_reason}")

        cpu_ratio = fmt_ratio(cpu_mi, cpu_m6)
        wall_ratio = fmt_ratio(wall_mi, wall_m6)

        cpu_mi_s = "nan" if math.isnan(cpu_mi) else f"{cpu_mi:.9f}"
        cpu_m6_s = "nan" if math.isnan(cpu_m6) else f"{cpu_m6:.9f}"
        wall_mi_s = "nan" if math.isnan(wall_mi) else f"{wall_mi:.9f}"
        wall_m6_s = "nan" if math.isnan(wall_m6) else f"{wall_m6:.9f}"

        print(f"CPU:  {cpu_mi_s} / {cpu_m6_s} = {cpu_ratio}")
        print(f"Wall: {wall_mi_s} / {wall_m6_s} = {wall_ratio}")
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

    # Auto-regenerate baseline if needed
    if (not baseline_dir.is_dir()) or (not baseline_complete(test_dir, baseline_dir)):
        print("Baseline incomplete or mismatched — regenerating baseline...")
        try:
            subprocess.run(["python3", "generate_baseline.py"], check=True)
        except FileNotFoundError:
            print("Error: python3 or generate_baseline.py not found.", file=sys.stderr)
            return 127
        except subprocess.CalledProcessError:
            print("Error: baseline regeneration failed.", file=sys.stderr)
            return 2

        if not baseline_dir.is_dir() or not baseline_complete(test_dir, baseline_dir):
            print("Error: baseline still incomplete after regeneration.", file=sys.stderr)
            return 2

        print("Baseline regenerated.\n")

    # Fresh output dir every run
    reset_dir(output_dir)

    return run_comparison(test_dir, baseline_dir, output_dir)

if __name__ == "__main__":
    raise SystemExit(main())
