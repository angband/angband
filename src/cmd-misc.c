#include "angband.h"
#include "cave.h"
#include "cmds.h"
#include "game-cmd.h"
#include "monster/monster.h"
#include "wizard.h"


/*
 * Toggle wizard mode
 */
void do_cmd_wizard(void)
{
	/* Verify first time */
	if (!(p_ptr->noscore & NOSCORE_WIZARD))
	{
		/* Mention effects */
		msg("You are about to enter 'wizard' mode for the very first time!");
		msg("This is a form of cheating, and your game will not be scored!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_WIZARD;
	}

	/* Toggle mode */
	if (p_ptr->wizard)
	{
		p_ptr->wizard = FALSE;
		msg("Wizard mode off.");
	}
	else
	{
		p_ptr->wizard = TRUE;
		msg("Wizard mode on.");
	}

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw "title" */
	p_ptr->redraw |= (PR_TITLE);
}




#ifdef ALLOW_DEBUG

/*
 * Verify use of "debug" mode
 */
void do_cmd_try_debug(void)
{
	/* Ask first time */
	if (!(p_ptr->noscore & NOSCORE_DEBUG))
	{
		/* Mention effects */
		msg("You are about to use the dangerous, unsupported, debug commands!");
		msg("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the debug commands? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_DEBUG;
	}

	/* Okay */
	do_cmd_debug();
}

#endif /* ALLOW_DEBUG */



#ifdef ALLOW_BORG

/*
 * Verify use of "borg" mode
 */
void do_cmd_try_borg(void)
{
	/* Ask first time */
	if (!(p_ptr->noscore & NOSCORE_BORG))
	{
		/* Mention effects */
		msg("You are about to use the dangerous, unsupported, borg commands!");
		msg("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the borg commands? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_BORG;
	}

	/* Okay */
	do_cmd_borg();
}

#endif /* ALLOW_BORG */


/*
 * Quit the game.
 */
void do_cmd_quit(cmd_code code, cmd_arg args[])
{
	/* Stop playing */
	p_ptr->playing = FALSE;

	/* Leaving */
	p_ptr->leaving = TRUE;
}


/*
 * Display the options and redraw afterward.
 */
void do_cmd_xxx_options(void)
{
	do_cmd_options();
	do_cmd_redraw();
}


/*
 * Display the main-screen monster list.
 */
void do_cmd_monlist(void)
{
	/* Save the screen and display the list */
	screen_save();
	display_monlist();

	/* Wait */
	anykey();

	/* Return */
	screen_load();
}


/*
 * Display the main-screen item list.
 */
void do_cmd_itemlist(void)
{
	/* Save the screen and display the list */
	screen_save();
	display_itemlist();

	/* Wait */
	anykey();

	/* Return */
	screen_load();
}


/*
 * Invoked when the command isn't recognised.
 */
void do_cmd_unknown(void)
{
	prt("Type '?' for help.", 0, 0);
}



