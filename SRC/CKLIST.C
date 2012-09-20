#include "constant.h"
#include "config.h"
#include "types.h"
#include "monster.h"

#include <stdio.h>

extern creature_type c_list[MAX_CREATURES];

void main(){
  int i;

  printf("Unique Monsters:\n\n");
  for(i=0;i<MAX_CREATURES-1;i++)
    if (c_list[i].cdefense & UNIQUE)
      printf("%-50s ('%c'), Found at %d'\n", c_list[i].name,
      		c_list[i].cchar, c_list[i].level * 50);
}
