####
# Please take a look at config.h for *critical* configuration steps needed.
# BINDIR is the directory where the angband binary while be put

BINDIR = /User/games/bin
PROGRAM = angband

####
# Optimization flags (or debugging).
####
# '-O' means optimize.  '-g' is for debugging.  Gcc lets you combine both.
# '-pipe' uses pipes rather than temporary files (which is faster).
# '-pg' enables profiling (for coders only).  '-W<something>' provides
# additional warnings.
####
# For HPUX, the compiler flags for 'cc' should be:
# COPT = -Ae -Dunix -g

#COPT = -g -O1 -Wall -Wcast-align -Wpointer-arith -Wwrite-strings -pipe
COPT = -g -O -pipe

####
# If you use gcc, you may find that adding -traditional-cpp will fix
# defining the LIBDIR in config.h.  '-Dconst=' may also help if your compiler
# complains about conflicting definitions

CFLAGS = $(COPT)
CC = cc

####
# some people might be able to (or even might have to) remove -lrpcsvc
# For Linux systems, this line should look like:
# LIBS = -lncurses -ltermcap -lbsd
 
LIBS = -lcurses -ltermcap -lrpcsvc

####
# If your system lacks the random() call, define EXTRAOBJS to include
# random.o.  Otherwise, you can leave it out.

#EXTRAOBJS=random.o
EXTRAOBJS=

# end of user configurable stuff
#######

SOURCES = main.c misc1.c misc2.c store1.c files.c io.c create.c desc.c\
	generate.c sets.c dungeon.c creature.c death.c eat.c help.c magic.c\
	potions.c prayer.c save.c staffs.c wands.c scrolls.c spells.c\
	wizard.c store2.c signals.c moria1.c moria2.c monsters.c\
	treasure.c variable.c recall.c unix.c undef.c\
	player.c tables.c describe_mon.c rods.c random.c util.c

OBJECTS = main.o misc1.o misc2.o store1.o files.o io.o create.o desc.o\
	generate.o sets.o dungeon.o creature.o death.o eat.o help.o magic.o\
	potions.o prayer.o save.o staffs.o wands.o scrolls.o spells.o\
	wizard.o store2.o signals.o moria1.o moria2.o monsters.o\
	treasure.o variable.o recall.o unix.o undef.o\
	player.o tables.o describe_mon.o rods.o util.o $(EXTRAOBJS)

LIBFILES = hours news origcmds.hlp owizcmds.hlp roglcmds.hlp rwizcmds.hlp \
	scores version.hlp welcome.hlp

default: all

all: $(PROGRAM)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(PROGRAM) $(LIBS)

install:	$(PROGRAM)
	cp $(PROGRAM) $(BINDIR)/$(PROGRAM)
	chown games.bin $(BINDIR)/$(PROGRAM)
	chmod 4511 $(BINDIR)/$(PROGRAM)

clean:;
	rm -f *.o ./core *~ .*~ temp1 temp2 gmon.*

realclean: clean
	rm -f $(PROGRAM)

spotless: clean
	rm -f $(PROGRAM)
	rm -f $(BINDIR)/$(PROGRAM)

depend:
	/usr/local/bin/X11/makedepend $(INCLUDES) -p$(OBJDIR)/ $(SRCS)

check:
	-/bin/rm -f temp1 temp2
	egrep '^{' monsters.c | awk -F\" '{print $$2}' > temp1
	egrep '^{' describe_mon.c | awk -F\" '{print $$2}' > temp2
	echo monster name problems
	-diff -c1 temp1 temp2

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
