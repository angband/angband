TESTPROGS += player/birth \
             player/history \
             player/player

player/birth : player/birth.c ../angband.o
player/history : player/history.c ../angband.o
player/player : player/player.c ../angband.o
