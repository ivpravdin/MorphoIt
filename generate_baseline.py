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
            print(
                f"Error: {dir_path} exists but is not a directory. "
                "Name it something else you're messing with my tests.",
                file=sys.stderr,
            )
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

    for line in text.splitlines(True):  # keep newlines
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

def run_once_with_output(src: Path, out_path: Path) -> tuple[int, float, float, bytes]:
    """
    Run once, capturing stdout into out_path.
    Returns (returncode, cpu, wall, cleaned_stderr).
    """
    cmd = ["gtime", "-f", "__CPU__ %U %S\n__WALL__ %e", "morpho6", str(src)]
    with out_path.open("wb") as f_out:
        result = subprocess.run(cmd, stdout=f_out, stderr=subprocess.PIPE, check=False)

    cpu, wall, cleaned_stderr = parse_gtime(result.stderr)
    return result.returncode, cpu, wall, cleaned_stderr

def time_batch_average(src: Path, n: int) -> tuple[float, float]:
    """
    Time morpho6 src n times as one timed batch, discarding stdout,
    then return (avg_cpu_per_run, avg_wall_per_run).
    """
    # Put the loop INSIDE gtime so the measured time accumulates.
    # This avoids the "0.00 each run" resolution problem entirely.
    loop = f'i=0; while [ "$i" -lt {n} ]; do morpho6 "{src}" >/dev/null; i=$((i+1)); done'
    cmd = ["gtime", "-f", "__CPU__ %U %S\n__WALL__ %e", "sh", "-c", loop]
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, check=False)

    cpu_total, wall_total, _ = parse_gtime(result.stderr)
    return cpu_total / n, wall_total / n

def run_all(out_dir: Path) -> int:
    test_dir = Path("tests")
    if not test_dir.is_dir():
        print("Error: expected ./tests directory to exist.", file=sys.stderr)
        return 2

    morpho_files = sorted(test_dir.glob("*.morpho"))
    if not morpho_files:
        print("No .morpho files found in ./tests")
        return 0

    failures = 0

    for src in morpho_files:
        out_path = out_dir / f"{src.stem}.output"
        time_path = out_dir / f"{src.stem}.time"

        try:
            rc, cpu_s, wall_s, cleaned_stderr = run_once_with_output(src, out_path)
        except FileNotFoundError:
            print("Error: command not found: gtime or morpho6", file=sys.stderr)
            return 127
        except Exception as e:
            print(f"Error: failed running {src.name}: {e}", file=sys.stderr)
            rc, cpu_s, wall_s, cleaned_stderr = 1, float("nan"), float("nan"), b""

        # Fallback: if first measurement is all zeros, measure an N-run batch and average.
        if cpu_s == 0.0 and wall_s == 0.0 and rc == 0:
            try:
                cpu_s, wall_s = time_batch_average(src, FALLBACK_RUNS)
            except Exception as e:
                print(f"Error: batch timing failed for {src.name}: {e}", file=sys.stderr)
                cpu_s, wall_s = float("nan"), float("nan")

        with time_path.open("w") as f_time:
            if math.isnan(cpu_s) or math.isnan(wall_s):
                f_time.write("nan\nnan\n")
            else:
                f_time.write(f"{cpu_s:.9f}\n{wall_s:.9f}\n")

        if rc != 0:
            failures += 1
            with out_path.open("ab") as f_out:
                f_out.write(b"\n\n--- STDERR ---\n")
                f_out.write(cleaned_stderr)
            print(f"[FAIL {rc}] {src.name}", file=sys.stderr)
        else:
            print(f"[OK] {src.name}")

    return 1 if failures else 0

def main() -> int:
    out_dir = Path("baseline")
    reset_dir(out_dir)
    return run_all(out_dir)

if __name__ == "__main__":
    raise SystemExit(main())
