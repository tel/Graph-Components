redo-ifchange $1$2.c $1.so
redo-ifchange asserts.h

gcc $1$2.c $1.so -o $3