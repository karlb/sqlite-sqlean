// Copyright (c) 2023 Anton Zhiyanov, MIT License
// https://github.com/nalgeon/sqlean

#ifndef FILEIO_EXTENSION_H
#define FILEIO_EXTENSION_H

#include "../sqlite3ext.h"

int fileio_ls_init(sqlite3* db);
int fileio_scalar_init(sqlite3* db);
int fileio_scan_init(sqlite3* db);

#endif /* FILEIO_EXTENSION_H */