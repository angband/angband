TESTPROGS += parse/parser parse/k-info parse/z-info

parse/parser: parse/parser.c ../angband.o
parse/k-info: parse/k-info.c ../angband.o
parse/z-info: parse/z-info.c ../angband.o
