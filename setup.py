from setuptools import setup  # type: ignore

# read the contents of README
from os import path

this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, "README.md"), encoding="utf-8") as f:
    long_description = f.read()

setup(
    name="sqlite-sqlean",
    version="0.1",
    description="Loadable sqlite extensions from sqlean",
    packages=["sqlite_sqlean"],
    package_data={"sqlite_sqlean": ["*.so", "*.dylib", "*.dll"]},
    url="http://github.com/karlb/sqlite-sqlean",
    long_description=long_description,
    long_description_content_type="text/markdown",
    extras_require={"test": ["pytest"]},
    setup_requires=["wheel"],
)
