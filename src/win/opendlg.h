/**
 * \file opendlg.h
 * \brief Windows Vista+ open file dialog showing character details
 *
 * Copyright (c) 2026 Klaas van Aarsen
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_OPENDLG_H
#define INCLUDED_OPENDLG_H

/**
 * Open a dialog for the user to select a save file.  The dialog shows the
 * character details of the highlighted save file.
 *
 * \param hwnd_owner is the handle to the owner window.
 * \param initial_dir is the directory to open the dialog in.
 * \param out_savefile is the buffer to store the selected path in, using the
 * ANSI code page.  Its contents are only meaningful if true is returned.
 * \param max_size is the size of out_savefile in bytes.
 * \return true if a save file was selected; false if the user cancelled or if
 * an error occurred.  Errors are reported to the user by this function.
 *
 * If FORCE_XP_COMPAT is defined, the classic dialog without character details
 * is shown instead, so that the Vista requirement is avoided.
 */
bool get_savefile(HWND hwnd_owner, char *initial_dir, char *out_savefile, size_t max_size);

#endif /* INCLUDED_OPENDLG_H */