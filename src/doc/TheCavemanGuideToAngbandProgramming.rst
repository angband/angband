****************************************
The Caveman Guide to Angband Programming
****************************************

Leon Marrick, December 2000

Said of Amelia Earhart, and good code too:
"...for she was straight and simple."

Introduction
============

Hacking with the Angband code is among the best ways
to learn C I know of. Ben Harrison's code is so lucid and so well commented
as to make reading it a pleasure. Starting out with very little C knowledge,
you can tinker with simple functions, advance to using pointers, see how
data structures operate, learn interface commands in various operating
systems, and eventually grapple with some fairly sophisticated algorithms
and code engines.

This document is designed to help you, the novice
programmer, produce Angband code that really does what you want it to do.
It discusses how to turn Angband into a debugger, talks about math from
a caveman's point of view, and finally offers some miscellaneous suggestions.

Angband is the Best Debugger
============================

There are few code problems
that Angband itself can't help solve. If things are getting confusing,
writing some temporary code to display messages and variables is often
your best move.

Use messages as debugging notices
---------------------------------

Let's start off with a truly
basic tip: Use messages. If you don't know when, how often, or even if
certain segments of your code are activating, the msg_print() and msg_format()
functions are your friends. Is an if-statement always returning FALSE in
practice? Is a loop running too much, and bogging things down? Where is
the code freezing or crashing? Find out with messages.

In some cases, you
must tweak the various options that affect how messages are displayed in
order to get useful output, or put messages inside loops to make sure you
pay attention to them. Another way to get messages to appear is to use
the prt() function with an on-screen start location.

Display your variables on screen
--------------------------------

Debuggers keep track of
lots of variables. Typically, however, you are interested in only a few,
but want to make absolutely certain you know what values they have at a
very precise point. Turn to the msg_format() function, or the format()
function nested inside a prt() statement.

For example, I still have
a copy of "generate.c" that prints out on screen the number of all kinds
of rooms as they are built. How many rooms of each type get created, on
average, on each level? How often does the build greater vault function
fail? After each room is built, what does a map of free and used dungeon
blocks look like? The prt() function can tell you, but no debugger can.

Debugging code that affects the screen
--------------------------------------

Much of the code you write
will affect the screen in some way, if only momentarily. First thing to
do if such code doesn't seem to work right is to remove any screen refreshes
after it dumps its output. You may also make the output more obvious temporarily,
or decide to display special characters showing the value of an internal
variable at that grid.

For example, many variants
of Angband have arc spells that rely on the distance() function. If you
freeze the area of effect of such a spell, you will see that this produces
some rather ugly results if the arc travels in a direction not divisible
by 45 degrees.

Caveman Mathematics
===================

Anyone who wants to code had better know math, but he
doesn't necessarily have to know much math. Since a complex equation confuses
me as fast as the next man, I'll share some of the simple, "caveman" math
that does most of the heavy lifting in my code.

Multiplication versus addition: The equation a * x + b
------------------------------------------------------

There are a lot of times
in Angband where a variable (say, melee skill) has an initial value and
is also affected by a variable (like player level). Key to balancing the
game is thinking carefully about how much weight to give the constant part
(usually a function of addition) and the variable part (often determined
by multiplication). How many monsters should appear on a level? How much
of a Ring of Speed's value should depend on its being a Ring of Speed and
how much on the actual speed bonus? How much of a spell's damage should
be guaranteed, and how much level-dependent? The Angband programmer usually
answers these questions by weighing addition versus multiplication until
the results work well for all values of the variable.

Division
--------

Division by zero is the
programmer's bugaboo. Variables fed into the equation "4 / x" should be
checked just beforehand, because this is where failure to control boundaries
or syntax errors can have immediate and fatal results.

In Angband, floating-point
math is a no-no. Many of the machines the game runs on crash nastily when
fed floating-point. Trouble is the division operator in fixed-point math
turns the numerator 3 divided by the denominator 2 into 1. As long as we
are careful about the limits of bytes and integers, we can inflate the
numerator before dividing it until we achieve the level of accuracy we
desire. If we want accuracy out to three places, then 100 * 3 / 2 will
yield 150.

The mod operator (%)
--------------------

I've never found anything
floating-point math could do in Angband that fixed-point couldn't with
a little ingenuity. Along with temporary inflation of values, tables, and
plain old-fashioned working around the problem, the mod operator is the
fixed-point mathematician's favorite tool. Remember that 5 / 2 = 2 in fixed-point
math, despite the remainder of 1. 5 % 2 yields that remainder: 1. All data
lost in division can be recovered.

There are other uses of
"%". If you code a loop, and want a portion of it to fire only one time
in four, you can use the line "if (i % 4 == 0)". If you want a value to
wrap around from 31 to 32 to 0, to avoid nasty results at 33, you can code
"if (y > 32) y = y % 32". This is very helpful to ensure legal bit operator
or table access.

Note: The mod operator causes no speed issues in most situations,
but is not fast enough for very low-level functions.

Calling the Random Number Generator (RNG) by name
-------------------------------------------------

Key to programming in this
game is the evocation of Angband's infamous RNG. You will be calling its
name frequently, and need to know the basic binding spells.

The function rand_int() yields a value between 0 and x-1. The function
randint() yields a value between 1 and x. Be ready to use either, with a
slight preference for rand_int() (randint() is actually rand_int() + 1),
depending on the comparison you have to make.

For example, if we want x
to have an x percentage chance of success, then "(x > rand_int(100))" is
correct, but "(x > rand_int(100))" or "(x > randint(100))" are not. With
any formula of this type, increases in x past the number being randomized
yield exactly the same result. 1000 is greater than 100, but so is 101.

You will also often see an
equation similar to "if (10 < randint(x))", which means that an x of
less than 10 is certain to yield FALSE, and an x of 10 or larger will yield
FALSE 10 times out of x. This function is useful for discriminating against
low 'x's, like the check for a hit in melee does against poor combat skill.

Unlike in the previous equation,
the result is never certain to be TRUE, no matter how high x gets. On the
other hand, if x can increase to a large multiple of 10, then we are very
likely to get a result of TRUE. This happens in the Angband trap code with
a high disarming skill; traps become almost harmless. If you want the function
to not approach certainty, but (say) approach a 50% chance of getting a
desired result, then use an additional test: "if (rand_int(2) == 0)" in
this case.

The damage of many spells
is determined by lines similar to "20 + randint(30)". This means that damage
done will be between 21 and 50, with each value having an equal chance
of happening. If you really meant damage to vary between 20 and 50, type
"20 + rand_int(31)" instead. There are many cases where this change might
profitably be made.

There are lots of other ways
to use rand_int(). Sometimes if complex or multiple calls to the RNG are
getting confusing, run the boundary conditions through the equations and
pay special attention to how ">", and "<", differ from ">=" and "<=".

Normal distributions the easy way with damroll()
------------------------------------------------

There are many situations
where you want controlled variation, biased towards values away from the
extremes. Normal distributions are ubiquitous in the real world; they're
good things to have in games too. The easiest way to get a close-to-normal
distribution is to use damroll().

This function takes two
values: number of dice (d), and dice sides (s). The result ranges from
d * 1 to d * s. A dagger (1d4) does between one and four points of damage.
The average damage is (d + d * s) / 2 - in the case of a dagger, this would
be 2.5. For mental calculations, some prefer to use the function: average
= d * (s+1) / 2.

The more dice there are,
and the fewer sides each die has, the more closely the results, over an
infinite number of runs, will cluster around the average. The result of
100d2 will very likely be quite close to 150. At the other extreme, using
one die is the same as using randint(): all legal values have the same
chance of occurring.

Warning: Using damroll is
like using rand_int() once for every die you roll. This is normally not
a problem, but be hesitant to use this function where speed is crucial.

General Suggestions
===================

Using Angband to help you debug
and getting comfortable with simple math helps a lot. Below are some ideas
for getting even better results.

Control boundary cases
----------------------

Bad code frays around the
edges, and breaks starting at extreme values. A good way to look for trouble
in your code is to run the following four numbers through your equations:

#. absolute minimum: the smallest number the code potentially allows to
   reach your equations
#. probable low end: the lowest number that a user
   may reasonably be expected to feed into your equations in a real game
#. probable high end: the highest number that a number may reasonably be
   expected to feed into your equations in a real game
#. absolute maximum: the largest number the code potentially allows to reach
   your equations

By checking numbers 1 and 4, you help stop your code from
crashing. By checking 2 and 3, you maintain game balance.

Use a spreadsheet
-----------------

Most valuable bit of advice
in this document: use a spreadsheet to work with non-trivial equations.
Doing so allows you to work interactively with math functions, looking
at all values for all variables simultaneously, in a way that you cannot
after you transfer them to code. Here is how you figure out the values
you want, and than play with equations until what you want is what you
get. All of a sudden, math starts doing what you want it to do...

Put more information in your code
---------------------------------

Code development is iterative
- it doesn't happen all at once. You can make life really hard on yourself
by adding no comments and scrunching your code into as small a space as
possible, or really easy by learning to write commentary early and enthusiastically.

If you're like me, even
your best friends could not call you a coding genius. If you know how to
write better than you know how to program, consider writing your way to
good code.

Learn when and where to worry about execution speed
---------------------------------------------------

Some parts of Angband require
more attention to speed than others. The less thinking the player has to
do to activate the code in question, and the less distracted he is while
it runs, the more he will pay attention to speed of execution. Also, the
more often code is called, whether by nesting of functions or in loops,
the more oportunity it has to eat up time.

The monster AI is a perfect
example. Monsters must very quickly decide whether to cast a spell, run,
or attack, because: 1) the code to make them plot and plan runs even when
the player is not spending time thinking, and 2) it is called very frequently
near major concentrations of monsters.

So how do you make the code fast where it matters? Standard Angband code
offers many a practical example of optimizing for speed. Some are so
sophisticated that they are almost indecipherable by the novice - others,
anyone can copy:

* Re-design loops and scans so that they have to cycle less to get the same
  result. See if you can make a search more intelligent, a test more likely
  to succeed, or a scan more focused. Be wary of functions or statements
  being called, failing, and having to be called again.
* Set up if-statements that winnow out those cases that do not need to activate
  the code. In the code that determines whether a monster will run, for example,
  most of the possible cases will obviously flee, or obviously stand and
  fight.
* Put tests that do the most work for the least effort first: A simple test
  goes before a complex test. A test that removes 40% of the candidates goes
  before an equally simple test that removes 20%.
* If you can't avoid calling rand_int altogether, at least get the maximum
  possible use out of the calls you must make. Keep rand_int out of loops,
  use a single call in independant tests (be careful), or divide up a random
  number into chunks, digits, or bits, and use the divisions. For example,
  each of the last four digits of rand_int(10000) may be used as a rand_int(10).
* Use permanent and temporary tables and global variables. Randomly search
  the entire monster list often enough, and you'll spend more time than you
  would if you filled a table with legal monsters, and used it when needed.
  There are many cases where the monster and object generation codes re-build
  the generation tables for precisely this reason.
* Sometimes, you need superfast math. Functions to use in these cases include
  the shift expressions "a << x" (multiply a by 2 * x) and "b >> y"
  (divide b by 2 * y), and other bit operators, but do not include "/", or
  "%".

And a last word
---------------

Angband still plays off floppy. A lot of coders have worked awfully hard to
make a game that is a shining example of just have much fun can fit on 1.4 meg
of space. Let this tradition be your tradition. Leave it to others to write
elephantine code that squeaks; let your code be mousy-small - and roar!

| Respectfully yours,
| Leon Marrick

Dedicated to:

* The coders who taught me everything I know,
* The experts who bailed me out of trouble, and
* The players who judged my errors gently.

Copyright 2000 by Leon Marrick

.. meta::
   :author: Leon Marrick
   :description:Hints for Angband developers
   :keywords: Angband, roguelike, development, programmer, developer, hint, tips, tricks, debugging
