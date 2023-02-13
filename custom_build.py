import platform
import zipfile
import os
import glob
import urllib.request
import sys
from subprocess import check_call
from pathlib import Path

SQLEAN_VERSION = "0.19.1"

# (system, machine) -> (sqlean_system_id, wheel_system_id)
supported = {
    ("Linux", "x86_64"): (
        "linux-x86",
        "manylinux_2_17_x86_64.manylinux2014_x86_64.manylinux1_x86_64",
    ),
    ("Darwin", "x86_64"): ("macos-x86", "win_amd64"),
    ("Darwin", "arm64"): ("macos-arm64", "macosx_11_0_arm64"),
    ("Windows", "x86_64"): ("win-x64", "macosx_10_6_x86_64"),
}


def build(sqlean_system_id, target_dir):
    sqlean_filename = f"sqlean-{sqlean_system_id}.zip"

    # Remove old
    for f in (
        glob.glob("sqlite_sqlean/*.so")
        + glob.glob("sqlite_sqlean/*.dll")
        + glob.glob("sqlite_sqlean/*.dylib")
    ):
        os.remove(f)
    for f in glob.glob(f"{target_dir}/*"):
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
    check_call(f"rm -fr build && pip wheel . -w {target_dir}", shell=True)


def rename_wheel(wheel_system_id, input_dir, output_dir):
    """Wheel renaming to make wheels less specific, since sqlite extensions are compatible across python versions.

    Inspired by https://github.com/asg017/sqlite-regex/blob/main/.github/workflows/rename-wheels.py
    """

    input_wheels = os.listdir(input_dir)
    assert len(input_wheels) == 1
    filename = input_wheels[0]
    assert "py3-none-any" in filename
    new_filename = filename.replace("py3-none-any", f"py3-none-{wheel_system_id}")

    output_dir.mkdir(exist_ok=True)
    Path(input_dir, filename).replace(output_dir / new_filename)


if len(sys.argv) == 2 and sys.argv[1] == "all":
    build_platforms = list(supported)
else:
    build_platforms = [(platform.system(), platform.machine())]

for build_platform in build_platforms:
    try:
        sqlean_system_id, wheel_system_id = supported[build_platform]
    except IndexError:
        raise Exception("Unsupported platform: {build_platform}")
    build(sqlean_system_id, "specific_wheel")
    rename_wheel(wheel_system_id, "specific_wheel", Path("dist"))
