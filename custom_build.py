import platform
import zipfile
import os
import glob
import urllib.request
from subprocess import check_call
from pathlib import Path

SQLEAN_VERSION = "0.19.1"

# Detect platform
supported = {
    ("Linux", "x86_64"): "linux-x86",
    ("Darwin", "x86_64"): "macos-x86",
    ("Darwin", "arm64"): "macos-arm64",
    ("Windows", "x86_64"): "win-x64",
}
try:
    sqlean_filename = f"sqlean-{supported[(platform.system(), platform.machine())]}.zip"
except IndexError:
    raise Exception("unsupported platform")

# Remove old
for f in glob.glob("sqlite_sqlean/*.{so,dylib,dll}"):
    os.remove(f)
for f in glob.glob("dist/*"):
    os.remove(f)

# Fetch and unzip sqlean extension
Path("downloads").mkdir(exist_ok=True)
urllib.request.urlretrieve(
    f"https://github.com/nalgeon/sqlean/releases/download/{SQLEAN_VERSION}/{sqlean_filename}",
    f"downloads/{sqlean_filename}",
)
with zipfile.ZipFile("downloads/" + sqlean_filename, "r") as zip_ref:
    zip_ref.extractall("sqlite_sqlean")

# Build wheel
check_call("pip wheel . -w dist", shell=True)

# Wheel renaming to make wheels less specific, since sqlite extensions are compatible across python versions.
# Adapted from https://github.com/asg017/sqlite-regex/blob/main/.github/workflows/rename-wheels.py
wheel_dir = "dist"
for f in os.listdir(wheel_dir):
    filename = Path(wheel_dir, f)
    if not filename.suffix == ".whl":
        continue
    new_filename = (
        filename.name.replace("cp37-cp37", "py3-none")
        .replace("cp38-cp38", "py3-none")
        .replace("cp39-cp39", "py3-none")
        .replace("cp310-cp310", "py3-none")
        .replace("cp311-cp311", "py3-none")
        .replace(
            "linux_x86_64",
            "manylinux_2_17_x86_64.manylinux2014_x86_64.manylinux1_x86_64",
        )
    )
    if (platform.system(), platform.machine()) == ("Darwin", "arm64"):
        new_filename = new_filename.replace(
            "macosx_12_0_universal2", "macosx_11_0_arm64"
        )
    else:
        new_filename = new_filename.replace(
            "macosx_12_0_universal2", "macosx_10_6_x86_64"
        ).replace("macosx_12_0_x86_64", "macosx_10_6_x86_64")

    os.rename(filename, Path(wheel_dir, new_filename))
