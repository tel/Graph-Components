redo-ifchange $1.c $1.h
redo-ifchange duset.so

gcc $1.c duset.so -L$HOME/local/lib -Wl,--rpath -Wl,$HOME/local/lib -lsqlite3 -o $3