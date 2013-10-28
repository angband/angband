#ifndef INCLUDED_DEBUG_H
#define INCLUDED_DEBUG_H

/**
 * Send the formatted string to whatever debug console or output is set up.
 *
 * The output will be treated logically as a single line, so do not include
 * newline characters in the format string.
 *
 * It is recommended you call debug() with "DHERE" at the beginning of your
 * format string, like so:
 * 	debug(DHERE "important info");
 *
 * This gives you file and line number information.
 */
void debug(const char *fmt, ...);

#endif /* INCLUDED_DEBUG_H */
