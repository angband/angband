# Contributing to Angband

This document is a guide to contributing to Angband.  It is largely a compilation of previous advice from various authors, updated as needed.

## Coding Guidelines

This section describes what Angband code and its documentation should look like.  You may also want to read the old [Angband security guide](/src/doc/security.txt), although the default build configuration no longer uses setgid.

### Rules

* K&R brace style, with tabs of four spaces
* Avoid lines over 80 characters long (not strict if there are multiple indents, but ideally they should be refactored)
* If a function takes no parameters, it should be declared as function(void), not just as function().
* Use const where you shouldn't be modifying a variable.
* Avoid global variables like the plague, we already have too many.
* Use enums where possible instead of defines, and never use magic numbers.
* Don't use floating point.
* Code should compile as C89 with C99 int types, and not rely on undefined behaviour.
* Don't use the C built-in string functions, use the my_ versions instead (strcpy -> my_strcpy, sprintf -> strnfmt()).  They are safer.

### Our indent style is:
* Opening braces should be on a separate line at the start of a function, but should otherwise follow the statement which requires them ('if', 'do', 'for' et al.)
* Closing braces for should be on separate lines, except where followed by 'while' or 'else'
* Spaces around the mathematical, comparison, and assignment operators ('+', '-', '/', '=', '!=', '==', '>', ...).  No spaces around  increment/decrement operators ('++', '--').
* Spaces between C identifiers like 'if', 'while' and 'for' and the opening brackets ('if (foo)', 'while (bar)', ...),
* `do { } while ();` loops should have a newline after "do {", and the "} while ();" bit should be on the same line.
* No spaces between function names and brackets and between brackets and function arguments (function(1, 2) instead of function ( 1, 2 )).
* If you have an if statement whose conditionally executed code is only one statement, do not write both on the same line, except in the case of "break" or "continue" in loops.
* `return` does not use brackets, `sizeof` does.
* Use two indents when a functional call/conditional extends over multiple lines, not spaces.

#### Example:
```C
    if (fridge) {
        int i = 10;

        if (i > 100) {
            i += randint0(4);
            bar(1, 2);
        } else {
            foo(buf, sizeof(buf), FLAG_UNUSED, FLAG_TIMED,
                    FLAG_DEAD);
        }
      
        do {
            /* Only print even numbers */
            if (i % 2) continue;

            /* Be clever */
            printf("Aha!");
        } while (i--);

        return 5;
    }
```

Write code for humans first and execution second. Where code is unclear, comment, but e.g. the following is unneccessarily verbose and hurts readability:
```C
    /* Delete the object */
    object_delete(idx);
```

### Code modules

* You should write code as modules wherever possible, with functions and global variables inside a module with the same prefix, like "macro_".
* If you need to initialise stuff in your module, include "init" and "free" functions and call them appropriately rather than putting module-specific stuff all over the place.
* One day the game might not quit when a game ends, and might allow loading other games.  Keep this in mind.

### Documentation

Be careful when documenting functions to use the following design:
```C
    /**
     * Provides an example of a documentation style.
     *
     * The purpose of the function do_something() is explained here, mentioning
     * the name and use of every parameter (e.g. `example`).  It returns TRUE if
     * conditions X or Y are met, and FALSE otherwise.
     *
     * BUG: Brief description of bug. (#12345)
     * TODO: Feature to implement. (#54321)
     */
    bool do_something(void *example)
```
#### Additional notes about the format
* Having the brief description separated out from the remainder of the comment means that Doxygen can pull it out without needing @brief tags.
* Variables should be referred to with surrounding backtick ('`') quotes.
* Functions should be referred to as function_name() -- ''with'' the brackets.
* In brief descriptions of classes and functions, use present tense (i.e. answer the question "What does this do?" with "It constructs / edits / calculates / returns...")
* In long descriptions, use passive mood to refer to variables (i.e. "The variables are normalised." as opposed to "This normalises the variables.")
* No UK/US spelling preference (i.e. the preference of the first commenter is adopted for that comment).
* (from "The Elements of Style") "A sentence should contain no unnecessary words, a paragraph no unnecessary sentences, for the same reason that a drawing should have no unnecessary lines and a machine no unnecessary parts. This requires not that the writer make all his sentences short, or that he avoid all detail and treat his subjects only in outline, but that every word tell."
