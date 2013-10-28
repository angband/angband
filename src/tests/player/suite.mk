TESTPROGS += player/birth player/player

player/birth : player/birth.c ../angband.o
player/player : player/player.c ../angband.o
