#!/usr/bin/env python3
"""
Real host-hardware stress test for validating a monitoring system.

IMPORTANT:
- This creates REAL CPU, RAM, disk-I/O, and network load on the host.
- It does NOT electrically overload a USB port or deliberately violate
  voltage/current limits. Those require a controlled external electronic load
  and measurement hardware.
- Run only on a machine you are authorized to stress.
"""

import argparse
import os
import socket
import tempfile
import threading
import time
from concurrent.futures import ThreadPoolExecutor
from multiprocessing import Event, Pool, cpu_count

import numpy as np
import requests


STOP = Event()


def cpu_worker():
    """Sustained real CPU load using NumPy matrix operations."""
    rng = np.random.default_rng()
    a = rng.random((1200, 1200), dtype=np.float32)
    b = rng.random((1200, 1200), dtype=np.float32)
    while not STOP.is_set():
        np.matmul(a, b, out=a)


def start_cpu_stress(cores):
    cores = max(1, min(int(cores), cpu_count()))
    STOP.clear()
    pool = Pool(processes=cores)
    for _ in range(cores):
        pool.apply_async(cpu_worker)
    return pool, cores


def stop_cpu_stress(pool):
    if pool is not None:
        pool.terminate()
        pool.join()


def memory_worker(size_mb=256):
    """Allocate and repeatedly touch real RAM pages."""
    size_mb = max(64, int(size_mb))
    buf = np.empty(size_mb * 1024 * 1024 // 8, dtype=np.float64)
    rng = np.random.default_rng()
    while not STOP.is_set():
        buf[:] = rng.random(buf.size)
        # Force a real read pass over the allocation.
        _ = float(np.sum(buf))


def start_memory_stress(workers, size_mb):
    workers = max(1, int(workers))
    executor = ThreadPoolExecutor(max_workers=workers)
    futures = [executor.submit(memory_worker, size_mb) for _ in range(workers)]
    return executor, futures


def stop_memory_stress(executor):
    if executor:
        executor.shutdown(wait=False, cancel_futures=True)


def disk_io_stress(directory, chunk_mb=32):
    """Real sequential disk writes followed by reads."""
    path = os.path.join(directory, "stress_temp.bin")
    chunk = os.urandom(chunk_mb * 1024 * 1024)

    with open(path, "wb", buffering=0) as f:
        for _ in range(30):
            if STOP.is_set():
                break
            f.write(chunk)
            f.flush()
            os.fsync(f.fileno())

    if os.path.exists(path):
        with open(path, "rb", buffering=0) as f:
            while not STOP.is_set():
                data = f.read(chunk_mb * 1024 * 1024)
                if not data:
                    break

    try:
        os.remove(path)
    except FileNotFoundError:
        pass


def network_worker(url, duration, stop_event):
    """Consume real HTTP response data to create sustained network traffic."""
    end = time.monotonic() + duration
    total = 0

    while time.monotonic() < end and not stop_event.is_set():
        try:
            with requests.get(url, stream=True, timeout=(5, 15)) as r:
                r.raise_for_status()
                for chunk in r.iter_content(chunk_size=1024 * 1024):
                    if not chunk or stop_event.is_set() or time.monotonic() >= end:
                        break
                    total += len(chunk)
        except requests.RequestException:
            time.sleep(0.5)

    return total


def network_stress(duration, aggressive=False):
    """
    Real network traffic. The response body is consumed, unlike the original
    script, so traffic is actually generated rather than merely opening
    HTTP requests.
    """
    url = "https://speed.cloudflare.com/__down?bytes=100000000"
    workers = 8 if aggressive else 3
    executor = ThreadPoolExecutor(max_workers=workers)
    futures = [
        executor.submit(network_worker, url, duration, STOP)
        for _ in range(workers)
    ]
    return executor, futures


def run_case(case_num, duration, cpu_cores, memory_workers, memory_mb):
    print(f"\n{'=' * 80}")
    print(f"CASE {case_num}: {duration}s REAL HOST STRESS")
    print(f"{'=' * 80}")

    cpu_pool = None
    memory_executor = None
    network_executor = None

    try:
        # Case 1: baseline
        if case_num == 1:
            print("[Case 1] Baseline: no intentional stress")
            time.sleep(duration)

        # Case 2: disk I/O
        elif case_num == 2:
            print("[Case 2] REAL disk I/O stress")
            disk_io_stress(tempfile.gettempdir())

        # Case 3: CPU + RAM
        elif case_num == 3:
            print("[Case 3] REAL CPU + RAM stress")
            cpu_pool, used = start_cpu_stress(cpu_cores)
            memory_executor, _ = start_memory_stress(memory_workers, memory_mb)
            print(f"         CPU workers: {used}, RAM per worker: {memory_mb} MB")
            time.sleep(duration)

        # Case 4: CPU + RAM + disk + network
        elif case_num == 4:
            print("[Case 4] REAL CPU + RAM + disk + network stress")
            cpu_pool, used = start_cpu_stress(cpu_cores)
            memory_executor, _ = start_memory_stress(memory_workers, memory_mb)
            network_executor, _ = network_stress(duration, aggressive=True)
            print(f"         CPU workers: {used}, RAM per worker: {memory_mb} MB")
            disk_io_stress(tempfile.gettempdir())
            time.sleep(duration)

        # Case 5: same physical stress, but no intentional acceleration input
        elif case_num == 5:
            print("[Case 5] REAL system stress; acceleration is NOT manipulated")
            cpu_pool, used = start_cpu_stress(cpu_cores)
            memory_executor, _ = start_memory_stress(memory_workers, memory_mb)
            network_executor, _ = network_stress(duration, aggressive=True)
            print(f"         CPU workers: {used}, RAM per worker: {memory_mb} MB")
            time.sleep(duration)

        else:
            raise ValueError(f"Unknown case: {case_num}")

    finally:
        STOP.set()

        if network_executor:
            network_executor.shutdown(wait=False, cancel_futures=True)

        stop_memory_stress(memory_executor)
        stop_cpu_stress(cpu_pool)

        STOP.clear()

    print(f"CASE {case_num} FINISHED")


def main():
    parser = argparse.ArgumentParser(description="Real host stress validation")
    parser.add_argument("--case", type=int, choices=range(1, 6),
                        help="Run one case instead of all five")
    parser.add_argument("--duration", type=int, default=60,
                        help="Stress duration in seconds")
    parser.add_argument("--cpu-cores", type=int,
                        default=max(1, cpu_count() - 1),
                        help="CPU worker count; default leaves one core free")
    parser.add_argument("--memory-workers", type=int, default=1)
    parser.add_argument("--memory-mb", type=int, default=256,
                        help="RAM allocated/touched per memory worker")
    args = parser.parse_args()

    print("=== REAL HOST HARDWARE STRESS VALIDATION ===")
    print(f"Logical CPUs: {cpu_count()}")
    print(f"CPU workers: {args.cpu_cores}")
    print(f"RAM per worker: {args.memory_mb} MB")
    print(f"Duration: {args.duration}s")

    cases = [args.case] if args.case else [1, 2, 3, 4, 5]

    try:
        for case in cases:
            run_case(
                case,
                args.duration,
                args.cpu_cores,
                args.memory_workers,
                args.memory_mb,
            )
    except KeyboardInterrupt:
        print("\nInterrupted by user.")
    finally:
        STOP.set()

    print("\nStress testing complete. Allow the system to cool down normally.")


if __name__ == "__main__":
    main()
