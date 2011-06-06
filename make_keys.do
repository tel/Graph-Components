redo-ifchange duset.so $1.c $1.h

clang duset.so $1.c -o $3 -lsqlite3