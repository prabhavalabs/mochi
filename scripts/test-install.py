#!/usr/bin/env python3
"""Install, relocate and consume Mochi without access to its source/build paths."""
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile

sdk = Path(__file__).resolve().parents[1] / "sdk/Mochi"
cmake = shlex.split(os.environ.get("CMAKE", "cmake"))

def run(*args):
    subprocess.run(cmake + list(map(str, args)), check=True)

with tempfile.TemporaryDirectory(prefix="mochi-install-") as temporary:
    root = Path(temporary)
    source, build = root / "source", root / "build"
    shutil.copytree(sdk, source)
    embedded_flags = [] if os.name == "nt" else ["-DCMAKE_CXX_FLAGS=-fno-exceptions -fno-rtti"]
    run("-S", source, "-B", build, "-DCMAKE_BUILD_TYPE=Release", *embedded_flags)
    run("--build", build, "--config", "Release")
    run("--install", build, "--config", "Release", "--prefix", root / "prefix")
    relocated = root / "relocated"
    (root / "prefix").rename(relocated)
    shutil.copytree(source / "tests/consumer", root / "consumer")
    shutil.rmtree(source)
    shutil.rmtree(build)
    # A fresh project can only find the relocated, installed package.
    run("-S", root / "consumer", "-B", root / "consumer-build",
        "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_PREFIX_PATH=" + str(relocated))
    run("--build", root / "consumer-build", "--config", "Release")
    candidates = [root / "consumer-build/mochi_consumer",
                  root / "consumer-build/Release/mochi_consumer.exe",
                  root / "consumer-build/mochi_consumer.exe"]
    executable = next(path for path in candidates if path.is_file())
    subprocess.run([str(executable)], check=True)
    print("PASS: relocated find_package consumer with original source/build removed")
