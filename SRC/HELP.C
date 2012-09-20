/* help.c: identify a symbol

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "monster.h"
#include "config.h"
#include "types.h"
#include "externs.h"


void ident_char()
{
  char command, query;
  register int i, n;

  if (get_com("Enter character to be identified :", &command))
    switch(command)
      {
	/* every printing ASCII character is listed here, in the order in which
	   they appear in the ASCII character set */
      case ' ': prt("  - An open pit.", 0, 0); break;
      case '!': prt("! - A potion.", 0, 0); break;
      case '"': prt("\" - An amulet, periapt, or necklace.", 0, 0); break;
      case '#': prt("# - A stone wall.", 0, 0); break;
      case '$': prt("$ - Treasure.", 0, 0); break;
      case '%':
	if (highlight_seams == TRUE)
	  prt("% - A magma or quartz vein.", 0, 0);
	else
	  prt("% - Not used.", 0, 0);
	break;
      case '&': prt("& - Demon (Oh dear!).", 0, 0); break;
      case '\'': prt("' - An open door.", 0, 0); break;
      case '(': prt("( - Soft armor.", 0, 0); break;
      case ')': prt(") - A shield.", 0, 0); break;
      case '*': prt("* - Gems.", 0, 0); break;
      case '+': prt("+ - A closed door.", 0, 0); break;
      case ',': prt(", - Food or mushroom patch.", 0, 0); break;
      case '-': prt("- - A wand", 0, 0); break;
      case '.': prt(". - Floor.", 0, 0); break;
      case '/': prt("/ - A pole weapon.", 0, 0); break;
	/* case '0': prt("0 - Not used.", 0, 0); break; */
      case '1': prt("1 - Entrance to General Store.", 0, 0); break;
      case '2': prt("2 - Entrance to Armory.", 0, 0); break;
      case '3': prt("3 - Entrance to Weaponsmith.", 0, 0); break;
      case '4': prt("4 - Entrance to Temple.", 0, 0); break;
      case '5': prt("5 - Entrance to Alchemy shop.", 0, 0); break;
      case '6': prt("6 - Entrance to Magic store.", 0, 0); break;
      case '7': prt("7 - Entrance to Black Market.", 0, 0); break;
      case '8': prt("8 - Entrance to your home.", 0, 0); break;
	/* case '9': prt("9 - Not used.", 0, 0);  break;*/
      case ':': prt(": - Rubble.", 0, 0); break;
      case ';': prt("; - A loose rock.", 0, 0); break;
      case '<': prt("< - An up staircase.", 0, 0); break;
      case '=': prt("= - A ring.", 0, 0); break;
      case '>': prt("> - A down staircase.", 0, 0); break;
      case '?': prt("? - A scroll.", 0, 0); break;
      case '@': prt(py.misc.name, 0, 0); break;
      case 'A': prt("A - Angel.", 0, 0); break;
      case 'B': prt("B - Birds", 0, 0); break; 
      case 'C': prt("C - Canine.", 0, 0); break;
      case 'D': prt("D - An Ancient Dragon (Beware).", 0, 0); break;
      case 'E': prt("E - Elemental.", 0, 0); break;
      case 'F': prt("F - Giant Fly.", 0, 0); break;
      case 'G': prt("G - Ghost.", 0, 0); break;
      case 'H': prt("H - Hybrid.", 0, 0); break;
      case 'I': prt("I - Minor Demon.", 0, 0); break;
      case 'J': prt("J - Jabberwock.", 0, 0); break;
      case 'K': prt("K - Killer Beetle.", 0, 0); break;
      case 'L': prt("L - Lich.", 0, 0); break;
      case 'M': prt("M - Mummy.", 0, 0); break;
	/* case 'N': prt("N - Not used.", 0, 0); break; */
      case 'O': prt("O - Ogre.", 0, 0); break;
      case 'P': prt("P - Giant humanoid.", 0, 0); break;
      case 'Q': prt("Q - Quylthulg (Pulsing Flesh Mound).", 0, 0); break;
      case 'R': prt("R - Reptiles and Amphibians.", 0, 0); break;
      case 'S': prt("S - Giant Scorpion/Spider.", 0, 0); break;
      case 'T': prt("T - Troll.", 0, 0); break;
      case 'U': prt("U - Umber Hulk.", 0, 0); break;
      case 'V': prt("V - Vampire.", 0, 0); break;
      case 'W': prt("W - Wight or Wraith.", 0, 0); break;
      case 'X': prt("X - Xorn/Xaren.", 0, 0); break;
      case 'Y': prt("Y - Yeti.", 0, 0); break;
      case 'Z': prt("Z - Zepher hound (Elemental hound).", 0, 0); break;
      case '[': prt("[ - Hard armor.", 0, 0); break;
      case '\\': prt("\\ - A hafted weapon.", 0, 0); break;
      case ']': prt("] - Misc. armor.", 0, 0); break;
      case '^': prt("^ - A trap.", 0, 0); break;
      case '_': prt("_ - A staff.", 0, 0); break;
	/* case '`': prt("` - Not used.", 0, 0); break; */
      case 'a': prt("a - Giant Ant/Ant Lion.", 0, 0); break;
      case 'b': prt("b - Giant Bat.", 0, 0); break;
      case 'c': prt("c - Giant Centipede.", 0, 0); break;
      case 'd': prt("d - a Dragon.", 0, 0); break;
      case 'e': prt("e - Floating Eye.", 0, 0); break;
      case 'f': prt("f - Felines", 0, 0); break;
      case 'g': prt("g - Golem.", 0, 0); break;
        /* case 'h': prt("h - Harpy.", 0, 0); break; */
      case 'h': prt("h - Humanoid (Dwarf, Elf, Halfling)",0,0); break; /* -CFT */
      case 'i': prt("i - Icky Thing.", 0, 0); break;
      case 'j': prt("j - Jelly.", 0, 0); break;
      case 'k': prt("k - Kobold.", 0, 0); break;
      case 'l': prt("l - Giant Louse.", 0, 0); break;
      case 'm': prt("m - Mold.", 0, 0); break;
      case 'n': prt("n - Naga.", 0, 0); break;
      case 'o': prt("o - Orc.", 0, 0); break;
      case 'p': prt("p - Person (Humanoid).", 0, 0); break;
      case 'q': prt("q - Quadroped.", 0, 0); break;
      case 'r': prt("r - Rodent.", 0, 0); break;
      case 's': prt("s - Skeleton.", 0, 0); break;
      case 't': prt("t - Giant Tick.", 0, 0); break;
        /* case 'u': prt("u - Unicorn.", 0, 0); break;  */
      case 'v': prt("v - Vortex.", 0, 0); break;
      case 'w': prt("w - Worm or Worm Mass.", 0, 0); break;
	/* case 'x': prt("x - Not used.", 0, 0); break; */
      case 'y': prt("y - Yeek.", 0, 0); break;
      case 'z': prt("z - Zombie.", 0, 0); break;
      case '{': prt("{ - Arrow, bolt, or bullet.", 0, 0); break;
      case '|': prt("| - A sword or dagger.", 0, 0); break;
      case '}': prt("} - Bow, crossbow, or sling.", 0, 0); break;
      case '~': prt("~ - Tool or miscellaneous item.", 0, 0); break;
      default:	prt("Not Used.", 0, 0); break;
      }

  /* Allow access to monster memory. -CJS- */
  n = 0;
  for (i = MAX_CREATURES-1; i >= 0; i--)
    if ((c_list[i].cchar == command) && bool_roff_recall (i))
      {
	if (n == 0)
	  {
	    put_buffer ("You recall those details? [y/n]", 0, 40);
	    query = inkey();
	    if (query != 'y' && query != 'Y')
	      break;
	    erase_line (0, 40);
	    save_screen ();
	  }
	n++;
	query = roff_recall (i);
	restore_screen ();
	if (query == ESCAPE)
	  break;
      }
}
