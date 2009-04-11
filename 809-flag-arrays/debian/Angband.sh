#!/bin/sh
#                               -*- Mode: Sh -*- 
# Angband.sh --- 
# Author           : Manoj Srivastava ( srivasta@glaurung.green-gryphon.com ) 
# Created On       : Wed Jul 26 05:04:24 2000
# Created On Node  : glaurung.green-gryphon.com
# Last Modified By : Manoj Srivastava
# Last Modified On : Mon Feb 16 23:29:49 2004
# Last Machine Used: glaurung.internal.golden-gryphon.com
# Update Count     : 2
# Status           : Unknown, Use with caution!
# HISTORY          : 
# Description      : 
# arch-tag: ccef83cb-3b14-4f98-a6a3-7705448b1212
#
# 

# Describe attempt
echo "Launching angband..."
sleep 2

# Main window
ANGBAND_X11_FONT_0=10x20
export ANGBAND_X11_FONT_0

ANGBAND_X11_AT_X_0=5
export ANGBAND_X11_AT_X_0

ANGBAND_X11_AT_Y_0=510
export ANGBAND_X11_AT_Y_0


# Message window
ANGBAND_X11_FONT_1=8x13
export ANGBAND_X11_FONT_1

ANGBAND_X11_AT_X_1=5
export ANGBAND_X11_AT_X_1

ANGBAND_X11_AT_Y_1=22
export ANGBAND_X11_AT_Y_1

ANGBAND_X11_ROWS_1=35
export ANGBAND_X11_ROWS_1


# Inventory window
ANGBAND_X11_FONT_2=8x13
export ANGBAND_X11_FONT_2

ANGBAND_X11_AT_X_2=635
export ANGBAND_X11_AT_X_2

ANGBAND_X11_AT_Y_2=182
export ANGBAND_X11_AT_Y_2

ANGBAND_X11_ROWS_3=23
export ANGBAND_X11_ROWS_3


# Equipment window
ANGBAND_X11_FONT_3=8x13
export ANGBAND_X11_FONT_3

ANGBAND_X11_AT_X_3=635
export ANGBAND_X11_AT_X_3

ANGBAND_X11_AT_Y_3=22
export ANGBAND_X11_AT_Y_3

ANGBAND_X11_ROWS_3=12
export ANGBAND_X11_ROWS_3


# Monster recall window
ANGBAND_X11_FONT_4=6x13
export ANGBAND_X11_FONT_4

ANGBAND_X11_AT_X_4=817
export ANGBAND_X11_AT_X_4

ANGBAND_X11_AT_Y_4=847
export ANGBAND_X11_AT_Y_4

ANGBAND_X11_COLS_4=76
export ANGBAND_X11_COLS_4

ANGBAND_X11_ROWS_4=11
export ANGBAND_X11_ROWS_4


# Object recall window
ANGBAND_X11_FONT_5=6x13
export ANGBAND_X11_FONT_5

ANGBAND_X11_AT_X_5=817
export ANGBAND_X11_AT_X_5

ANGBAND_X11_AT_Y_5=520
export ANGBAND_X11_AT_Y_5

ANGBAND_X11_COLS_5=76
export ANGBAND_X11_COLS_5

ANGBAND_X11_ROWS_5=24
export ANGBAND_X11_ROWS_5


# Gamma correction
ANGBAND_X11_GAMMA=142
export ANGBAND_X11_GAMMA


# Launch Angband
/usr/games/angband -mx11 -- -n6 &



