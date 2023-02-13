
import importlib.util
from pathlib import Path
import sqlite3

BASE_PATH = Path(__file__).parent

def loadable_path(extension: str) -> str:
    return str(BASE_PATH / extension)


def load(conn: sqlite3.Connection, extension: str) -> None:
    conn.load_extension(loadable_path(extension))
