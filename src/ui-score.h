/**
 * \file ui-score.h
 * \brief Highscore display for Angband
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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

#ifndef INCLUDED_UI_SCORE_H
#define INCLUDED_UI_SCORE_H

void show_scores(void);
void predict_score(bool allow_scrolling);

#endif /* INCLUDED_UI_SCORE_H */
