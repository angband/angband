

// this source code is for your edification only and modifications to it or

// questions about it are not supported by Borland International



#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <sys\stat.h>

#include "fsparse.h"



extern char far_str[];

extern unsigned _stklen=10000;

int parse(FILE *, FILE *, FILE *);

char hdrExists;



int main(int argc, char *argv[])

{

   struct stat dummy;

   FILE *istream, *ostream, *hdrstream;

   char prefix[128],iFile[128],oFile[128],hdrFile[128],buf[128];

   char *dot;

   if (argc<2)

   {

      printf("Filename: ");

      gets(iFile);

   }

   else strcpy(iFile,argv[1]);

   istream=fopen(iFile,"rb");

   if (istream==NULL)

   {

      printf("Input file %s not found\n",iFile);

      exit(1);

   }



   strcpy(prefix,iFile);

   

   // truncate filename at dot

   dot=strchr(prefix,'.');

   if (dot!=NULL) *dot=0;

   strupr(prefix);

   

   strcpy(hdrFile,prefix);

   strcat(hdrFile,".fsh");

   if (stat(hdrFile,&dummy)==-1) hdrExists=0; else hdrExists=1;

   hdrstream=fopen(hdrFile,"a+b");

   if (hdrstream==NULL)

   {

       printf("Cannot open or create %s\n",hdrFile);

       exit(2);

   }

   

   // Material below is for putting option in header

   // to group far strings all in a single segment

   if (!hdrExists)

   {

#if 0 /* SK: everything fits into one segment, loads faster */

      sprintf(buf,"#pragma option -zE%s_FARSEG\n",prefix);

      fputs(buf,hdrstream);

#else

      fputs("#pragma option -zESTRINGS_FARSEG\n",hdrstream);

#endif

      fseek(hdrstream,0,SEEK_END);

   }

   

   strcpy(oFile,prefix);

   strcat(oFile,".fs");

   ostream=fopen(oFile,"wb");

   if (ostream==NULL)

   {

       printf("Cannot create %s\n",oFile);

       exit(3);

   }  

   if (!hdrExists)

   {

      strcpy(buf,"#include \"");

      strcat(buf,hdrFile);

      strcat(buf,"\"\n");

      fputs(buf,ostream);

   }

   

   // set up the prefix for our string names, used in parse.c

   strcpy(far_str,prefix);

   strlwr(far_str);

   strcat(far_str,"FarString");

   

   // do the job

   printf("-- Strings found --\n");

   parse(istream,ostream,hdrstream);

   fclose(istream);

   fclose(ostream);

   fclose(hdrstream);

   return(0);

}



