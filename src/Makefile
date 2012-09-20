# This will get rewritten a little to make it GNU Make portable for the
# final release.  Until then, there are some rough edges...sorry.  -CWS

# SRCDIR is the directory containing the sources
# BINDIR is the directory where the angband binary while be put
# LIBDIR is where the other files (score, news, hours) will be put
# OBJDIR is where the object files will be put

SRCDIR = .
OBJDIR = .
LIBDIR = /User/games/lib/angband
BINDIR = /User/games/bin
PROGRAM = angband

#COPT = -g -Wall -Wshadow -Wcast-align -O1 -DLINT_ARGS
COPT = -O

# Note that any reasonably modern compiler does better when you *don't* use
# "register".  I've hacked it out here because I don't want to change every arg
# list in the game.  You might want to undo this if your compiler sucks.

CFLAGS = $(COPT) -fwritable-strings -DLIBDIR=\"$(LIBDIR)\" -Dregister=\"\"
CC = gcc

SOURCES = main.c misc1.c misc2.c store1.c files.c io.c create.c desc.c\
	generate.c sets.c dungeon.c creature.c death.c eat.c help.c magic.c\
	potions.c prayer.c save.c staffs.c wands.c scrolls.c spells.c\
	wizard.c store2.c signals.c moria1.c moria2.c monsters.c\
	treasure.c variable.c rnd.c recall.c unix.c undef.c\
	player.c tables.c describe_mon.c rods.c

OBJECTS = main.o misc1.o misc2.o store1.o files.o io.o create.o desc.o\
	generate.o sets.o dungeon.o creature.o death.o eat.o help.o magic.o\
	potions.o prayer.o save.o staffs.o wands.o scrolls.o spells.o\
	wizard.o store2.o signals.o moria1.o moria2.o monsters.o\
	treasure.o variable.o rnd.o recall.o unix.o undef.o\
	player.o tables.o describe_mon.o rods.o

LIBFILES = hours news origcmds.hlp owizcmds.hlp roglcmds.hlp rwizcmds.hlp \
	scores version.hlp welcome.hlp

default:	all

all:	angband

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$*.c -o $@

angband: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o angband -lcurses -ltermcap -lrpcsvc

install:	angband
	cp angband $(BINDIR)/$(PROGRAM)
	chown games.bin $(BINDIR)/$(PROGRAM)
	chmod 4511 $(BINDIR)/$(PROGRAM)

clean:;
	rm -f $(OBJDIR)/*.o $(BINDIR)/core $(OBJDIR)/core ./core angband

spotless: clean
	rm -f angband
	rm -f $(BINDIR)/angband

depend:
	/usr/local/bin/X11/makedepend $(INCLUDES) -p$(OBJDIR)/ $(SRCS)

query:
	echo "Monsters.c"
	egrep '^{"' monsters.c | wc -l
	echo "Describe_Mon.c"
	egrep '^{"' describe_mon.c | wc -l

create.o: constant.h types.h externs.h config.h
creature.o: constant.h types.h externs.h config.h monster.h
death.o: constant.h types.h externs.h config.h
desc.o: constant.h types.h externs.h config.h
dungeon.o: constant.h types.h externs.h config.h
eat.o: constant.h types.h externs.h config.h
files.o: constant.h types.h externs.h config.h
generate.o: constant.h types.h externs.h config.h
help.o: constant.h types.h externs.h config.h monster.h
io.o: constant.h types.h externs.h config.h
magic.o: constant.h types.h externs.h config.h
main.o: constant.h types.h externs.h config.h
misc1.o: constant.h types.h externs.h config.h monster.h
misc2.o: constant.h types.h externs.h config.h monster.h
monsters.o: constant.h types.h config.h monster.h
moria1.o: constant.h types.h externs.h config.h monster.h
moria2.o: constant.h types.h externs.h config.h monster.h
player.o: constant.h types.h config.h
potions.o: constant.h types.h externs.h config.h
prayer.o: constant.h types.h externs.h config.h monster.h
recall.o: constant.h config.h types.h externs.h monster.h
rnd.o: constant.h types.h
save.o: constant.h types.h externs.h config.h
scrolls.o: constant.h types.h externs.h config.h monster.h
sets.o: constant.h config.h types.h
signals.o: constant.h types.h externs.h config.h
spells.o: constant.h types.h externs.h config.h monster.h
staffs.o: constant.h types.h externs.h config.h monster.h
store1.o: constant.h types.h externs.h config.h
store2.o: constant.h types.h externs.h config.h
tables.o: constant.h types.h config.h
treasure.o: constant.h types.h config.h
unix.o: constant.h config.h types.h externs.h
variable.o: constant.h types.h config.h
rods.o: constant.h types.h externs.h config.h monster.h
wands.o: constant.h types.h externs.h config.h monster.h
wizard.o: constant.h types.h externs.h config.h
describe_mon.o: monster.h externs.h types.h

# DO NOT DELETE THIS LINE -- make depend depends on it.
