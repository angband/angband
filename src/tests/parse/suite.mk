TESTPROGS += parse/a-info \
             parse/c-info \
             parse/e-info \
	     parse/f-info \
	     parse/flavor \
	     parse/h-info \
             parse/names \
             parse/parser \
             parse/k-info \
	     parse/owner \
	     parse/p-info \
	     parse/r-info \
	     parse/s-info \
	     parse/store \
	     parse/v-info \
	     parse/z-info

parse/a-info: parse/a-info.c ../angband.o
parse/c-info: parse/c-info.c ../angband.o
parse/e-info: parse/e-info.c ../angband.o
parse/f-info: parse/f-info.c ../angband.o
parse/flavor: parse/flavor.c ../angband.o
parse/h-info: parse/h-info.c ../angband.o
parse/names: parse/names.c ../angband.o
parse/parser: parse/parser.c ../angband.o
parse/k-info: parse/k-info.c ../angband.o
parse/owner: parse/owner.c ../angband.o
parse/p-info: parse/p-info.c ../angband.o
parse/r-info: parse/r-info.c ../angband.o
parse/s-info: parse/s-info.c ../angband.o
parse/store: parse/store.c ../angband.o
parse/v-info: parse/v-info.c ../angband.o
parse/z-info: parse/z-info.c ../angband.o
