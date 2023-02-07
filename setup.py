from setuptools import setup, Extension  # type: ignore
from glob import glob

# read the contents of README
from os import path
this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, "README.md"), encoding="utf-8") as f:
    long_description = f.read()


setup(
    name="sqlite-sqlean",
    version="0.1",
    description="Loadable sqlite extensions from sqlean",
    py_modules=["sqlite_sqlean"],
    ext_modules=[
        Extension("sqlean.crypto", sources=["sqlean/src/sqlite3-crypto.c"] + glob("sqlean/src/crypto/*.c")),
        Extension("sqlean.define", sources=["sqlean/src/sqlite3-define.c"] + glob("sqlean/src/define/*.c")),
        Extension("sqlean.fileio", sources=["sqlean/src/sqlite3-fileio.c"] + glob("sqlean/src/fileio/*.c")),
        Extension("sqlean.fuzzy", sources=["sqlean/src/sqlite3-fuzzy.c"] + glob("sqlean/src/fuzzy/*.c")),
        Extension("sqlean.ipaddr", sources=["sqlean/src/sqlite3-ipaddr.c"]),
        # Extension("json1", sources=["sqlean/src/sqlite3-json1.c"]),
        Extension("sqlean.math", sources=["sqlean/src/sqlite3-math.c"], libaries=['m']),

        Extension("sqlean.stats", sources=["sqlean/src/sqlite3-stats.c"], libaries=['m']),
        Extension("sqlean.text", sources=["sqlean/src/sqlite3-text.c"]),
        Extension("sqlean.unicode", sources=["sqlean/src/sqlite3-unicode.c"]),
        Extension("sqlean.uuid", sources=["sqlean/src/sqlite3-uuid.c"]),
        Extension("sqlean.vsv", sources=["sqlean/src/sqlite3-vsv.c"], libaries=['m']),
    ],
    #url="http://github.com/karlb/sqlite-spellfix",
    long_description=long_description,
    long_description_content_type="text/markdown",
    extras_require={"test": ["pytest"]},
    setup_requires=['wheel'],
)
