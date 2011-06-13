redo-ifchange duset.so $1.c $1.h

gcc -Wall -O3 $1.c duset.so -I$HOME/local/include -L$HOME/local/lib -Wl,--rpath -Wl,$HOME/local/lib -lsqlite3 -o $3