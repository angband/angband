/**
 * \file snd-win.h
 * \brief Sound handling for Windows
 *
 * Copyright (c) 2016 Graeme Russ
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */

#ifndef INCLUDED_SND_WIN_H
#define INCLUDED_SND_WIN_H

errr init_sound_win(struct sound_hooks *hooks, int argc, char **argv);

#endif /* !INCLUDED_SND_WIN_H */
