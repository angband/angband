/* File: x-infofnt.c */

#include "x-infofnt.h"

#include "z-util.h"
#include "z-virt.h"


/*** The current Infofnt ***/

infofnt *Infofnt = (infofnt*)(NULL);




/*
 * Nuke an old 'infofnt'.
 */

errr Infofnt_nuke ()
{
  infofnt *ifnt = Infofnt;

  /* Deal with 'name' */
  if (ifnt->name)
  {
    /* Free the name */
    string_free (ifnt->name);
  }

  /* Nuke info if needed */
  if (ifnt->nuke)
  {
    /* Free the font */
    XFreeFont (Metadpy->dpy, ifnt->info);
  }

  /* Success */
  return (0);
}



/*
 * Prepare a new 'infofnt'
 */

static errr Infofnt_prepare (XFontStruct *info)
{
  infofnt *ifnt = Infofnt;

  XCharStruct *cs;

  /* Assign the struct */
  ifnt->info = info;

  /* Jump into the max bouonds thing */
  cs = &(info->max_bounds);

  /* Extract default sizing info */
  ifnt->asc = cs->ascent;
  ifnt->hgt = (cs->ascent + cs->descent);
  ifnt->wid = cs->width;

  /* Success */
  return (0);
}





/*
 * Initialize a new 'infofnt'.
 */

errr Infofnt_init_real (XFontStruct *info)
{
  /* Wipe the thing */
  WIPE(Infofnt, infofnt);

  /* No nuking */
  Infofnt->nuke = 0;

  /* Attempt to prepare it */
  return (Infofnt_prepare (info));
}




/*
 * Init an infofnt by its Name
 *
 * Inputs:
 *	name: The name of the requested Font
 */

errr Infofnt_init_data (char *name)
{
  XFontStruct *info;


  /*** Load the info Fresh, using the name ***/

  /* If the name is not given, report an error */
  if (!name) return (-1);

  /* Attempt to load the font */
  info = XLoadQueryFont (Metadpy->dpy, name);

  /* The load failed, try to recover */
  if (!info) return (-1);


  /*** Init the font ***/

  /* Wipe the thing */
  WIPE(Infofnt, infofnt);

  /* Attempt to prepare it */
  if (Infofnt_prepare (info))
  {
    /* Free the font */
    XFreeFont (Metadpy->dpy, info);

    /* Fail */
    return (-1);
  }

  /* Save a copy of the font name */
  Infofnt->name = string_make (name);

  /* Mark it as nukable */
  Infofnt->nuke = 1;

  /* Success */
  return (0);
}




