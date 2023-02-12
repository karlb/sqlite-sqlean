import importlib.util
from pathlib import Path
import sqlite3


def loadable_path(extension: str) -> str:
    spec = importlib.util.find_spec('sqlite_sqlean.' + extension)
    if spec is None or spec.origin is None:
        raise Exception(f"Unknown extension {extension!r}")
    p = Path(spec.origin)
    return str(p.parent / p.stem)


def load(conn: sqlite3.Connection, extension: str) -> None:
    conn.load_extension(loadable_path(extension))
