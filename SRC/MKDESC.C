/* mkdesc.c : make an external description file from describe_mon.c,
	monsters.c, and this source file...  This automates the process
	a bit.  Note that this program should be remade whenever the monster
	list or the descriptions change. 	-CFT */
#include "constant.h"
#include "config.h"
#include "types.h"
#include "monster.h"

#include <stdio.h>
#include <string.h>

extern creature_type c_list[MAX_CREATURES];
extern describe_mon_type desc_list[MAX_CREATURES];


int main(int ac, char *av[]){
  int mnum, i, j;
  long oldseek, newseek;
  FILE *descfile;
  char *not_found = "ERROR - NO MATCHING DESCRIPTION FOUND!!!!\n";
  char buf[1024];
  
  if (ac < 2){
    printf("Usage: %s outfile\n", av[0]);
    return 1;
  }
  if ( (descfile = fopen(av[1], "wb")) == NULL){
    printf("Error opening outfile '%s'.\n", av[1]);
    return 1;
  }
  
  oldseek = newseek = 0L; /* write out dummy values to pad file */
  for(i=0;i<MAX_CREATURES;i++) fwrite(&oldseek, sizeof(long), 1, descfile);
  newseek = ftell(descfile); /* ptr to 1st string */
  
  mnum = 0;
  while (mnum < MAX_CREATURES){
    for(i=0; i<MAX_CREATURES;i++){
      if (!strcmp(desc_list[i].name,
	   (mnum != (MAX_CREATURES-1) ? c_list[mnum].name : "Player ghost"))){
	oldseek = newseek; /* remember current place */
        strcpy(buf, desc_list[i].desc);
        fprintf(descfile, "%s\n", buf);
	newseek = ftell(descfile); /* remember new place */
	fseek(descfile, (long)(mnum * sizeof(long)), SEEK_SET);
	fwrite(&oldseek, sizeof(long), 1, descfile); /* overwrite dummy
	  						seek ptr */
	fseek(descfile, newseek, SEEK_SET); /* and return to where we were */
        break; /* stop looking after 1st match */
      }
    }
    if (i==MAX_CREATURES){
    	/* unfound, put some "BOOGA BOOGA" message... */
	oldseek = newseek; /* remember current place */
        fprintf(descfile, "%s\n", not_found);
	newseek = ftell(descfile); /* remember new place */
	fseek(descfile, (long)(mnum * sizeof(long)), SEEK_SET);
	fwrite(&oldseek, sizeof(long), 1, descfile); /* overwrite dummy
	  						seek ptr */
	fseek(descfile, newseek, SEEK_SET); /* and return to where we were */
        break; /* stop looking after 1st match */
    }
    fputc('.', stdout);  fflush(stdout);
    mnum++;
  } /* while mnum < MAX_CREATURES */
  fclose(descfile);
  return 0;
} 
