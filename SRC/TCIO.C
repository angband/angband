/* ibmpc/tcio.c: terminal I/O code for Turbo C

   Copyright (c) 1989-92 James E. Wilson, Eric Vaitl

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

/* This I/O module doesn't need PCcurses.  You may also need to make some
   changes to ms_misc.c.  */

#include <alloc.h>  /* malloc() */
#include <ctype.h>
#include <dos.h>
#include <stdlib.h>  /* getenv() */
#include <stdio.h> /* putch */
#include <process.h> /* spawnl() */
#include <conio.h> /* window(), gotoxy() */

#ifdef __TURBOC__
#include	<string.h>
#endif /* __TURBOC__ */

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#define MSG_LEN  73
#define LINES 25
#define COLS 80

static void *savescr; /* pointer to a saved screen */

void init_curses(void){
  if((savescr=malloc(LINES*COLS*2))==NULL){
    puts("Out of memory in init_curses()");
    exit(1);
  }
  clrscr();
  msdos_raw();
}


int get_string(char *in_str,int row,int column,int slen)
{
  register int start_col, end_col, i;
  char *p;
  int flag, aborted;

  aborted = FALSE;
  flag  = FALSE;
  gotoxy(column+1,row+1);
  for (i = slen; i > 0; i--)
    putch(' ');
  gotoxy(column+1,row+1);
  start_col = column;
  end_col = column + slen - 1;
  if (end_col > 79)
    {
      slen = 80 - column;
      end_col = 79;
  }
  p = in_str;
  do
    {
      i = inkey();
      switch(i)
        {
          case ESCAPE:
          aborted = TRUE;
          break;
        case CTRL('J'): case CTRL('M'):
          flag  = TRUE;
          break;
        case DELETE: case CTRL('H'):
          if (column > start_col)
            {
              column--;
              put_buffer(" ", row, column);
              move_cursor(row, column);
              *--p = '\0';
          }
          break;
        default:
          if (!isprint(i) || column > end_col)
            bell();
          else
            {
              gotoxy(column+1,row+1);
              putch((char) i);
              *p++ = i;
              column++;
          }
          break;
      }
  }
  while ((!flag) && (!aborted));
  if (aborted)
    return(FALSE);
  /* Remove trailing blanks     */
  while (p > in_str && p[-1] == ' ')
    p--;
  *p = '\0';
  return(TRUE);
}

void put_buffer(char *out_str,int row,int col){
  vtype tmp_str;
  if (col>79) col=79;
  strncpy(tmp_str,out_str,79-col);
  tmp_str[79-col]='\0';
  gotoxy(col+1,row+1);
  cputs(tmp_str);
}

void put_qio(void){
/* nothing to do */
}

void restore_term(void){
  _setcursortype(_NORMALCURSOR);
  fflush(stdout);
  clear_screen();
  msdos_noraw();
}

void shell_out(void){
  char *comspec;
#ifndef __TURBOC__
  char key;
  int val;
  char *str;
#endif /* __TURBOC__ */
  save_screen();
  clear_screen();
  puts("[Entering DOS shell, type exit to return to game.]");
  msdos_noraw();
  ignore_signals();
  if((comspec=getenv("COMSPEC")) ==NULL ||
    spawnl(P_WAIT,comspec,comspec,(char *)NULL)<0){
    puts("Sorry, there seems to be a problem with shell_out()");
    printf("comspec = %s\n",comspec);
    flush();
    puts("Hit a key to continue");
    while(!kbhit())
      ;
  }
  restore_signals();
  restore_screen();
}

void save_screen(void){
  gettext(1,1,COLS,LINES,savescr);
}

void restore_screen(void){
  puttext(1,1,COLS,LINES,savescr);
}

void clear_screen(void){
  window(1,1,COLS,LINES);
/*
I think later I might want to define seperate windows, so the above line
is definsive code.
*/
  clrscr();
}

void clear_from(int row){
  window(1,row+1,COLS,LINES);
  clrscr();
  window(1,1,COLS,LINES);
}

void flush(void){
  while(kbhit())
    getch();
}

void erase_line(int row, int col){
  if(row==MSG_LINE&&msg_flag)
    msg_print(NULL);
  gotoxy(col+1,row+1);
  clreol();
}

char inkey(void){
  int i;

  command_count=0;
  while(TRUE){
    _setcursortype(_NORMALCURSOR);
    i=msdos_getch();
    if(i==EOF){
      eof_flag++;
      msg_flag=FALSE;
      if(!character_generated||character_saved) exit_game();
      disturb(1,0);
      if(eof_flag>100){
        panic_save=1;
        strcpy(died_from,"(end of input: panic saved)");
        if(!save_char()){
          strcpy(died_from,"panic: unexpected eof");
          death=TRUE;
        }
        exit_game();
      }
    _setcursortype(_NOCURSOR);
    return ESCAPE;
  }
#if 0 /* this is bizarre.  Why would I want ^R to enter raw mode? -CFT */
  if(i!=CTRL('R'))
    return (char) i;
  msdos_raw();
    break;
  } /* while loop */
  return (CTRL('R'));
#else /* new code -CFT */
  _setcursortype(_NOCURSOR);
  return (char)i;
  } /* while loop */
#endif
}

void print(char ch, int row, int col){
  row -= panel_row_prt;
  col-=panel_col_prt;
  gotoxy(col+1,row+1);
#ifdef TC_COLOR  /*  Use direct video write for color.  --JMA  */
  putch((int)ch);
#else                /*  I really don't know why the non-color version uses
                          putchar instead of putch, but I'll leave it as I
                          found it....  --JMA  */
  putchar((int)ch);
#endif
}

void move_cursor_relative(int row, int col){
  row-=panel_row_prt;
  col-=panel_col_prt;
  gotoxy(col+1,row+1);
}

void count_msg_print(char *p){
  int i;
  i=command_count;
  msg_print(p);
  command_count=i;
}

void prt(char* str_buff,int row, int col){
  if (row==MSG_LINE&&msg_flag)
    msg_print(NULL);
  gotoxy(col+1,row+1);
  clreol();
  put_buffer(str_buff,row,col);
}

void move_cursor(int row,int col){
  gotoxy(col+1,row+1);
}


void msg_print(char* str_buff){
  register int old_len, new_len, t_len, spaces, split_len = 0;
  char in_char, *split = NULL, *t = str_buff;
  
  if(msg_flag)
    old_len=strlen(old_msg[last_msg]);
  else {
    old_len = 0; /* I use it to test for -CFT */
    gotoxy(1,MSG_LINE+1); /* msg_flag is false, so nothing important here */
    clreol(); /* clear it to get rid of any clutter that might be here... */
    }
/* Multiple messages on one msg line... from Purple X's Moria.  Thanks
   to brianm@soda.berkeley.edu (Brian Markenson) for this.
   Moria 5.5 may already have this, but my version of tcio.c doesn't,
   so PC Angband doesn't. -CFT */

    /* If the new message and the old message are short enough, we want
       display them together on the same line.  So we don't flush the old
       message in this case.  */

/* This should be even better... If 2 msgs don't fit on one line, it will
   split the second one (at a space), putting as much as it can (without
   splitting a word) on the end of the 1st line.  It also will loop, so
   that msg_print will be able to handle an extremely long mesg by breaking
   it up into several lines.  If the code cannot find a space at all, it
   will give up, and just break the mesg in the middle of a word.  This
   will cause 1 character to be lost.  But this should never happen, since
   it would require about 70 letters in a row, w/o a space...  -CFT */
   
  if (str_buff) {
    if (msg_flag)
      spaces = 2;
    else { /* if nothing there, don't leave extra spaces, and move to a */
      spaces = 0;  /* new, blank line to save this mesg. */
      if (++last_msg>=MAX_SAVE_MSG) last_msg=0; 
      strcpy(old_msg[last_msg], " "); /* 3 spaces on new mesg lines, this
      				helps to locate start of long, multi-line
      				rounds.  A single space here, plus the 2
				spaces added when merging lines... -CFT */
      old_len = 1;
      }
      
    new_len = strlen(str_buff);

    while (old_len+new_len+spaces >= MSG_LEN) { /* too long? try to make some fit */
      if (old_len+spaces < MSG_LEN) { /* more on this line? */
	t = str_buff;
	split = NULL;
	split_len = 0;
	for(t_len = 0; t_len+old_len+spaces < MSG_LEN; t_len++, t++)
	  if (*t == ' '){ /* this is a possible place to split */
	    split = t;	/* the message line -CFT */
	    split_len = t_len;
	    }

        if (!split && !old_len)  /* if we looked for MSG_LEN chars, and
        				couldn't find a space, we'd better
        				force it to split, to avoid an
        				infinite loop -CFT */
          split = str_buff+MSG_LEN -2; /* cut it here, 1 char will be lost */
          
	if (split) { /* okay, found a good place to split new line */
	  if (spaces) { /* we skip spaces if there was no old mesg */
	    strcat(old_msg[last_msg], "  ");
	    }
	  strncat(old_msg[last_msg], str_buff, split_len); /* remember 1st part */
	  old_msg[last_msg][old_len+spaces+split_len+1] = 0; /* end string */
	  put_buffer (&old_msg[last_msg][old_len],
		 MSG_LINE, old_len); /* show 1st part */
	  str_buff = split+1; /* then point to rest of message */
	  }
        } /* okay, now we've fit as much as we can, wait for keypress */
      
      t_len = strlen(old_msg[last_msg])+1;
      if (t_len > MSG_LEN) t_len = MSG_LEN;
      put_buffer("-more-", MSG_LINE, t_len);
      do{
        in_char=inkey();
        }while((in_char!=' ')&&(in_char!=ESCAPE)&&(in_char!='\n')&&
	   (in_char!='\r'));

      gotoxy(1,MSG_LINE+1); /* clear old mesg... */
      clreol();

      /* now adjust values for next pass through loop... */
      if(++last_msg>=MAX_SAVE_MSG) last_msg=0;
      old_msg[last_msg][0]=0; /* clear it, so we add properly */
      old_len = spaces = 0; /* no need for extra spaces, we're just
      				processing the rest of str_buff -CFT */
      new_len = strlen(str_buff)+1; /* calc new length */
      }

    /* okay, if old+new mesgs couldn't fit, then old is displayed, and
    	 only rest of new msg needed to be handled.  If both old+new did
    	 fit, we must process them here... -CFT */

    if (old_len) { /* then must fit, or would have entered while loop, so
    			just merge */
      put_buffer("  ", MSG_LINE, old_len); /* added just in case -CFT */
      put_buffer (str_buff, MSG_LINE, old_len + 2);
      strcat (old_msg[last_msg], "  ");
      strcat (old_msg[last_msg], str_buff);
      }      	
    else { /* only get here from while loop, with just the rest of str_buff
    		to output + remember */
      put_buffer(str_buff, MSG_LINE, 0);
      strncpy(old_msg[last_msg],str_buff,VTYPESIZ);
      old_msg[last_msg][VTYPESIZ-1]='\0';
      }       
    msg_flag = TRUE; /* remember that there's something on the msg line */
    command_count = 0;
    } /* if str_buff */
  else { /* msg_print(NULL).... */
    if (msg_flag) { /* was something on the command line */
      t_len = strlen(old_msg[last_msg])+1;
      if (t_len > MSG_LEN) t_len = MSG_LEN;
      put_buffer("-more-", MSG_LINE, t_len);
      do{
        in_char=inkey();
        }while((in_char!=' ')&&(in_char!=ESCAPE)&&(in_char!='\n')&&
	   (in_char!='\r'));
      gotoxy(1,MSG_LINE+1); /* clear old mesg... */
      clreol();
      msg_flag = FALSE; /* no message to worry about */
      }
    else { /* was nothing there, so no "-more-" needed */
      gotoxy(1,MSG_LINE+1); /* clear old mesg... */
      clreol();
      }
    } /* msg_print(NULL) */
}


int get_check(char*prompt){
  int res;
  prt(prompt,0,0);
  if(wherex()>MSG_LEN +1) gotoxy(74,1);
  cputs(" [y/n]");
  do{
    res=inkey();
  }while(res==' ');
  erase_line(0,0);
  if(res=='Y'||res=='y')
    return(TRUE);
  else
    return(FALSE);
}

/* just like get check, but only a 'Y' means yes.  Used in places where
   player doesn't want the up-and-left key mapped to 'y' to screw things
   up, like "enter wiz mode?" -CFT */
int get_Yn(char*prompt){
  int res;
  prt(prompt,0,0);
  if(wherex()>MSG_LEN +1) gotoxy(74,1);
  cputs(" [Y/n]");
  do{
    res=inkey();
  }while(res==' ');
  erase_line(0,0);
  if(res=='Y')
    return(TRUE);
  else
    return(FALSE);
}

int get_com(char *prompt,char *command){
  int res;
  if(prompt)
    prt(prompt,0,0);
  *command=inkey();
  if(*command==ESCAPE)
    res=FALSE;
  else
    res=TRUE;
  erase_line(MSG_LINE,0);
  return(res);
}
void bell(void){
  if (! sound_beep_flag)
    return;
  putchar('\007');
}

/* the rest is just modified -ev- */

/* definitions used by screen_map() */
/* index into border character array */
#define TL 0    /* top left */
#define TR 1
#define BL 2
#define BR 3
#define HE 4    /* horizontal edge */
#define VE 5

/* character set to use */
#   define CH(x)        (screen_border[1][x])

  /* Display highest priority object in the RATIO by RATIO area */
#define RATIO 3

void screen_map()
{
    register int  i, j;
  static int8u screen_border[2][6] = {
      {'+', '+', '+', '+', '-', '|'},     /* normal chars */
    {201, 187, 200, 188, 205, 186}      /* graphics chars */
};
  int8u map[MAX_WIDTH / RATIO + 1];
  int8u tmp;
  int priority[256];
  int row, orow, col, myrow, mycol = 0;
  char prntscrnbuf[80];

  for (i = 0; i < 256; i++)
    priority[i] = 0;
  priority['<'] = 5;
  priority['>'] = 5;
  priority['@'] = 10;
  priority[wallsym] = -5;
  priority[floorsym] = -10;
  priority['\''] = -3;
  priority[' '] = -15;

  save_screen();
  clear_screen();
  gotoxy(1,1);
  putch(CH(TL));
  for (i = 0; i < MAX_WIDTH / RATIO; i++)
    putch(CH(HE));
  putch(CH(TR));
  orow = -1;
  map[MAX_WIDTH / RATIO] = '\0';
  for (i = 0; i < MAX_HEIGHT; i++)
    {
        row = i / RATIO;
      if (row != orow)
        {
            if (orow >= 0)
            {
               sprintf(prntscrnbuf,"%c%s%c",CH(VE), map, CH(VE));
               gotoxy(1,orow+2);
               cputs(prntscrnbuf);
          }
          for (j = 0; j < MAX_WIDTH / RATIO; j++)
            map[j] = ' ';
          orow = row;
      }
      for (j = 0; j < MAX_WIDTH; j++)
        {
            col = j / RATIO;
          tmp = loc_symbol(i, j);
#ifdef TC_COLOR
          if (!no_color_flag) textcolor(LIGHTGRAY);
#endif
          if (priority[map[col]] < priority[tmp])
            map[col] = tmp;
          if (map[col] == '@')
            {
                mycol = col + 1; /* account for border */
              myrow = row + 1;
          }
      }
  }
  if (orow >= 0)
    {
      sprintf(prntscrnbuf,"%c%s%c",CH(VE), map, CH(VE));
      gotoxy(1,orow+2);
      cputs(prntscrnbuf);
  }
  gotoxy(1,orow+3);
  putch(CH(BL));
  for (i = 0; i < MAX_WIDTH / RATIO; i++)
    putch(CH(HE));
  putch(CH(BR));
  gotoxy(24,24);
  cputs("Hit any key to continue");
  if (mycol > 0)
    gotoxy(mycol+1, myrow+1);
  inkey();
  restore_screen();
}

void pause_exit(int prt_line, int delay)
{
    char dummy;

#ifdef __TURBOC__
  /* Otherwise, TURBO C complains that delay is never used.  */
  dummy = (char) delay;
#endif
  prt("[Press any key to continue, or Q to exit.]", prt_line, 10);
  dummy = inkey();
  if (dummy == 'Q')
    {
      erase_line(prt_line, 0);
      exit_game();
  }
  erase_line(prt_line, 0);
}

void pause_line(int prt_line)
{
  prt("[Press any key to continue.]", prt_line, 23);
  (void) inkey();
  erase_line(prt_line, 0);
}

