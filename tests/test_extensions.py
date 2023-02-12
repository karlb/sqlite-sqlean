from pathlib import Path
try:
    # Some python builds don't have enable_load_extension enabled. In those
    # cases we need the pysqlite3-binary package to provide a sqlite3 with it
    # enabled to be able to run our tests.
    import pysqlite3 as sqlite3
except ImportError:
    import sqlite3
from subprocess import check_call

import pytest

import sqlite_sqlean

all_extensions = [
    "crypto",
    "define",
    "fileio",
    "fuzzy",
    "ipaddr",
    "json1",
    "math",
    "regexp",
    "stats",
    "text",
    "unicode",
    "uuid",
    "vsv"
]


@pytest.mark.parametrize("extension", all_extensions)
def test_extension(extension, monkeypatch):
    conn = sqlite3.connect(":memory:")
    conn.enable_load_extension(True)
    sqlite_sqlean.load(conn, extension)

    sqlean_path = Path(__file__).parent.parent / "sqlean"
    with open(sqlean_path / "test" / f"{extension}.sql") as f:
        test_sql_script = f.read()

    monkeypatch.chdir(sqlean_path)
    next_statement = ""
    for line in test_sql_script.splitlines():
        if line.startswith(".load") or 'load_extension' in line or line.startswith("--"):
            continue
        if line.startswith(".shell "):
            shell_cmd = line[len('.shell '):].replace(r'\\n', r'\n')
            check_call(shell_cmd, shell=True)
            continue

        next_statement += line
        if not sqlite3.complete_statement(next_statement):
            continue

        result = conn.execute(next_statement).fetchone()
        if next_statement.lower().startswith('select') and result and len(result) == 2:
            print(next_statement)
            assert result[1] == True, f"{extension} test {result[0]} failed."

        next_statement = ""
