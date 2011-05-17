redo-ifchange $1.c $1.h
redo-ifchange duset.so

clang duset.so -lsqlite3 $1.c -o $3
