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
        Extension(
            "sqlite_sqlean.crypto",
            sources=["sqlean/src/sqlite3-crypto.c"] + glob("sqlean/src/crypto/*.c"),
        ),
        Extension(
            "sqlite_sqlean.define",
            sources=["sqlean/src/sqlite3-define.c"] + glob("sqlean/src/define/*.c"),
        ),
        Extension(
            "sqlite_sqlean.fileio",
            sources=["sqlean/src/sqlite3-fileio.c"] + glob("sqlean/src/fileio/*.c"),
        ),
        Extension(
            "sqlite_sqlean.fuzzy",
            sources=["sqlean/src/sqlite3-fuzzy.c"] + glob("sqlean/src/fuzzy/*.c"),
        ),
        Extension("sqlite_sqlean.ipaddr", sources=["sqlean/src/sqlite3-ipaddr.c"]),
        Extension("sqlite_sqlean.json1", sources=["sqlean/src/sqlite3-json1.c"]),
        Extension("sqlite_sqlean.math", sources=["sqlean/src/sqlite3-math.c"], libraries=["m"]),
        Extension(
            "sqlite_sqlean.regexp",
            sources=[
                "sqlean/src/sqlite3-regexp.c",
                "sqlean/src/regexp/regexp.c",
            ]
            + glob("sqlean/src/regexp/pcre2/*.c"),
            define_macros=[
                ("PCRE2_CODE_UNIT_WIDTH", 8),
                ("LINK_SIZE", 2),
                ("HAVE_CONFIG_H", None),
                ("SUPPORT_UNICODE", None),
            ],
        ),
        Extension(
            "sqlite_sqlean.stats", sources=["sqlean/src/sqlite3-stats.c"], libraries=["m"]
        ),
        Extension("sqlite_sqlean.text", sources=["sqlean/src/sqlite3-text.c"]),
        Extension("sqlite_sqlean.unicode", sources=["sqlean/src/sqlite3-unicode.c"]),
        Extension("sqlite_sqlean.uuid", sources=["sqlean/src/sqlite3-uuid.c"]),
        Extension("sqlite_sqlean.vsv", sources=["sqlean/src/sqlite3-vsv.c"], libraries=["m"]),
    ],
    url="http://github.com/karlb/sqlite-sqlean",
    long_description=long_description,
    long_description_content_type="text/markdown",
    extras_require={"test": ["pytest"]},
    setup_requires=["wheel"],
)
