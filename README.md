# Python package for the sqlean SQLite extensions

This python package includes a loadable extension module for sqlite provided by [sqlean](https://github.com/nalgeon/sqlean). This allows other python packages to use this extension without requiring dependencies outside of the python ecosystem. For more details about the specific extensions, see https://github.com/nalgeon/sqlean/tree/main/docs.

I'm currently experimenting with two different approaches to packaging sqlean:
* Using the binaries from the original sqlean release
* Building the binaries as part of the package build process.

Please see https://github.com/karlb/sqlite-sqlean/issues/1 for details.

## Installation

### Latest Release

Install the `sqlite-sqlean` package from pypi.

### Current Development Version

Install via pip

```sh
pip install git+https://github.com/karlb/sqlite-sqlean
```

or add this to you requirements.txt:

```
git+https://github.com/karlb/sqlite-sqlean
```

## Usage
Example usage for the `crypto` extension.

```python
import sqlite3
import sqlite_sqlean

conn = sqlite3.connect(":memory:")
conn.enable_load_extension(True)
conn.load_extension(sqlite_spellfix.extension_path())
sqlite_sqlean.load(conn, 'crypto')
# now use as described in https://github.com/nalgeon/sqlean/blob/main/docs/crypto.md
```
