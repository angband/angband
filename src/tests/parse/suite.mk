TESTPROGS += parse/a-info \
             parse/e-info \
             parse/names \
             parse/parser \
             parse/k-info \
	     parse/z-info

parse/a-info: parse/a-info.c ../angband.o
parse/e-info: parse/e-info.c ../angband.o
parse/names: parse/names.c ../angband.o
parse/parser: parse/parser.c ../angband.o
parse/k-info: parse/k-info.c ../angband.o
parse/z-info: parse/z-info.c ../angband.o
