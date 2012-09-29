

// this code is for your edification only and modifications to it or questions

// about it are not supported by Borland International



#include <stdio.h>

#include <alloc.h>

#include <string.h>

#include <stdlib.h>

#include "fsparse.h"



#define MAX_TABLE_SIZE 1000



char far_str[30];



char buffer[500];

char **stringTable;

unsigned int tableSize,maxString, startString;

extern char hdrExists;



void parse(FILE *istream, FILE *ostream, FILE *hdrstream)

{

	 char prevChar=0,thisChar=0;

     char inString=NOT_IN_STRING,inSingleQuote=FALSE,escapeNext=FALSE,

          iosState=FALSE;

     int charCount,parenLevel=0,oldStyleLevel=0,newStyleLevel=0;

     int i;

     

     if (hdrExists) maxString=GetHighFromHeader(hdrstream);

     else maxString=0;

     startString=maxString;

     stringTable=(char **)malloc(MAX_TABLE_SIZE*sizeof(char *));

     tableSize=0;

     

     while (1)

     {

        prevChar=thisChar;

        thisChar=getc(istream);

        if (feof(istream)) break;

		switch(inString)

        {

            case NOT_IN_STRING:

				SetCommentLevel(thisChar,prevChar,

						&oldStyleLevel,&newStyleLevel);

                // ignore comments

                if (oldStyleLevel>0 || newStyleLevel>0)

				{

					putc(thisChar,ostream);

					continue;

				}

                if (!inSingleQuote)

                {

					SetParenLevel(thisChar,&parenLevel);

                    SetIOSState(prevChar,thisChar,&iosState);

                }

				// entering string?

				if (thisChar=='"' && !inSingleQuote && !escapeNext)

				{

                    if (parenLevel>0 || iosState)

					{

						inString=IN_STRING;

						charCount=0;

					}

					else

					{

						inString=IGNORE_STRING;

						putc(thisChar,ostream);

					}

				}

				// or just doing the usual

				else

				{

					putc(thisChar,ostream);

					// Catch a single quote or '"' could cause problems

					// We use escapeNext to avoid getting confused by '\''

					if (thisChar=='\'' && !escapeNext)

					{

						if (inSingleQuote) inSingleQuote=FALSE;

						else inSingleQuote=TRUE;

					}

				}

				break;

            // the special case of LEAVING_STRING is necessary because we

            // may be leaving the string and then entering again (right

            // away or after whitespace) ie  "some quote" "continued")

            case LEAVING_STRING:

				SetCommentLevel(thisChar,prevChar,&oldStyleLevel,&newStyleLevel);

				SetParenLevel(thisChar,&parenLevel);



				// going back into string?

                if (thisChar=='"') inString=IN_STRING;

                // or really truly ending it for sure ...

                else if (NotWhiteSpace(thisChar))

                {

					// dump the string to our .FSH and .FS files (not

                    // forgetting to also output the current char)

                    inString=NOT_IN_STRING;

					buffer[charCount]=0;

					AddStringToTable(buffer,hdrstream,ostream);

					putc(thisChar,ostream);

				}

                break;

            case IN_STRING:

                // Might we be getting out of the string?

				if (thisChar=='"' && !escapeNext)

				{

					inString=LEAVING_STRING;

				}

                // or just staying with this string

                else

                {

                    buffer[charCount]=thisChar;

                    ++charCount;

                }

                break;

			case IGNORE_STRING:

				// this is the case when we have something in a string

				// but we cannot just replace it with a variable because

				// it is not in a function call; char a[]="blah" for

				// example

				putc(thisChar,ostream);

				if (thisChar=='"' && !escapeNext) inString=NOT_IN_STRING;

				break;

        } // end switch block 

        

        // set escapeNext so we won't respond to any

        // backslash-escaped characters  

        if (thisChar=='\\' && !escapeNext) escapeNext=TRUE;

        else escapeNext=FALSE;

     } /* end while */

     for (i=0; i<tableSize; ++i) free(stringTable[i]);

     free(stringTable);

} /* end parse function */



void SetCommentLevel(char thisChar, char prevChar, 

                    int *oldStyleLevel, int *newStyleLevel)

{

   if (prevChar=='/' && thisChar=='/' && *oldStyleLevel==0)

      *newStyleLevel=1;

   if (thisChar=='\n' && *newStyleLevel==1)

      *newStyleLevel=0;

   if (prevChar=='/' && thisChar=='*' && *newStyleLevel==0)

      ++(*oldStyleLevel);

   if (prevChar=='*' && thisChar=='/' && *newStyleLevel==0)

      --(*oldStyleLevel);

}



int NotWhiteSpace(char thisChar)

{

   if (thisChar=='\t' || thisChar=='\n' || thisChar=='\r' || thisChar==' ')

      return FALSE;

   else return TRUE;

}



void SetParenLevel(char thisChar, int *parenLevel)

{

   if (thisChar=='(') ++(*parenLevel);

   else if (thisChar==')') --(*parenLevel);

   if (*parenLevel>10) printf("Bad parenLevel: %d\t",*parenLevel);

}



void SetIOSState(char prevChar, char thisChar, char *iosState)

{

    if (prevChar==thisChar)

        if (prevChar=='<' || prevChar=='>') *iosState=TRUE;

    if (thisChar==';') *iosState=FALSE;

}





void AddStringToTable(char *s, FILE *hdrstream, FILE *ostream)

{

   int i;

   char buf[500];

   char stringName[50];

   int newString;

   

   strcpy(stringName,far_str);

   // maxString should be assigned elsewhere and refreshed on a

   // per-file basis.  as should tableSize.

   for (i=0; i<tableSize; ++i)

   {

       if (strcmp(s,stringTable[i])==0) break;         

   }

   if (i==tableSize)

   {

       newString = 1;

       printf("%s\n",s);

       if (tableSize>MAX_TABLE_SIZE)

       {

           for (i=0; i<tableSize; ++i)

               free(stringTable[i]);

           tableSize = 0;

       }

       stringTable[tableSize] = (char *)malloc(strlen(s)+1);

       strcpy(stringTable[tableSize], s);

       tableSize++;

   }

   else

       newString = 0;

   strcat(stringName,itoa(startString+i,buf,10));

   ++maxString;

   strcpy(buf,"char far ");

   strcat(buf,stringName);

   strcat(buf,"[]=\"");

   strcat(buf,s);

   strcat(buf,"\";\n");

   if (newString) fputs(buf,hdrstream);

   fputs(stringName,ostream);

   // give a comment as to what the string was

   // can't put comments inside comments, so we'll quickly parse for them

   for (i=1; i<strlen(s); ++i)

   {

      if (s[i-1]=='/' && s[i]=='/') s[i-1]=s[i]='C';

      if (s[i-1]=='/' && s[i]=='*') { s[i-1]='C'; s[i]='b'; }

      if (s[i-1]=='*' && s[i]=='/') { s[i-1]='C'; s[i]='e'; }

   }

   strcpy(buf," /* \"");

   strcat(buf,s);

   strcat(buf,"\" */ ");

   fputs(buf,ostream);

}



// if running FARSTR again, we want to make sure we don't start the numbering

// of those strings from 0 -- might have been a better idea to keep a

// single comment for this purpose in the .FSH file?  Currently we just

// pull the first number out of each line and make sure we start numbering

// higher than any of these.



unsigned int GetHighFromHeader(FILE *hdrstream)

{

     int highNum=0,thisNum;

     char *start, *end;

     fseek(hdrstream,0,SEEK_SET);

     fgets(buffer,500,hdrstream);

     while (!feof(hdrstream))

     {

        start=strpbrk(buffer,"0123456789");

        if (start!=NULL)

        {

            end=start;

            while (strchr("0123456789",*end)!=NULL) ++end;

            *end=0;

            thisNum=atoi(start);

            if (thisNum>=highNum) highNum=thisNum+1;

        }

        fgets(buffer,500,hdrstream);

    }

    fseek(hdrstream,0,SEEK_END);

    return(highNum);

}

