bzero (sp, len)
char *sp;
int  len;
{
   while (len--) 
      *sp++ = '\0';
}
