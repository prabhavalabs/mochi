#!/usr/bin/env python3
"""Build portable SDK consumers and run meaningful checks with sanitizers."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
sdk = root / "sdk/Mochi"
compiler = shlex.split(os.environ.get("CXX", "clang++"))
flags = ["-std=c++17", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-I" + str(sdk / "src")]
sources = sorted(str(p) for p in (sdk / "src").glob("*.cpp"))

with tempfile.TemporaryDirectory(prefix="mochi-sdk-") as temporary:
    output = Path(temporary)
    consumers = [sdk / "tests/sdk_test.cpp", root / "firmware/test/native.cpp",
                 sdk / "examples/offscreen.cpp", sdk / "examples/cast.cpp"]
    for consumer in consumers:
        binary = output / consumer.stem
        subprocess.run(compiler + flags + [str(consumer)] + sources + ["-o", str(binary)], check=True)
        args = [str(binary)]
        if consumer.name == "offscreen.cpp":
            args.append(str(output / "mochi.ppm"))
        if consumer.name == "cast.cpp":
            args.append(str(output / "cast.ppm"))
        subprocess.run(args, check=True)
    assert (output / "mochi.ppm").stat().st_size > 320 * 320 * 3
    assert (output / "cast.ppm").stat().st_size > 1280 * 320 * 3
