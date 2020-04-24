/**
 * \file list-ui-entry-renderers.h
 * \brief List of ways to render combinations of object or player properties.
 *
 * Additions here will have to have corresponding changes to
 * ui-entry-renderers.c.  Changes only impact how the second character screen
 * is drawn.  There's fuller descriptions of these renderers in
 * ui_entry_renderer.txt.
 */

/* symbol, default colors, default label colors, default symbols,
   default number of digits, sign indicator handling */
UI_ENTRY_RENDERER(COMPACT_RESIST_RENDERER_WITH_COMBINED_AUX, "wwwwwwGGGWWWWWWGGG", "swBrgwww", "?..+-*!=%", 0, NO_SIGN)
UI_ENTRY_RENDERER(COMPACT_FLAG_RENDERER_WITH_COMBINED_AUX, "wwwwGWWWWG", "swBw", "?..+!", 0, NO_SIGN)
UI_ENTRY_RENDERER(NUMERIC_AS_SIGN_RENDERER_WITH_COMBINED_AUX, "wwwwwGGWWWWWGG", "swBrww", "?..+-!=", 0, NO_SIGN)
UI_ENTRY_RENDERER(NUMERIC_RENDERER_WITH_COMBINED_AUX, "wwwBrbowwwBrbo", "swBrww", "?00****", 1, NO_SIGN)
UI_ENTRY_RENDERER(NUMERIC_RENDERER_WITH_BOOL_AUX, "wdsgGgrRwdsgGgrR", "wwwwwww", "? .s*=", 1, NO_SIGN)
