# Contributing to Angband

This document is a guide to contributing to Angband.  It is largely a compilation of previous advice from various authors, updated as needed.

## Offering your contribution to Angband

When you've fixed a bug or implemented a new feature, please let us know.  The preferred way to do this is to submit a pull request on Github.

### General git knowledge

git is a version control system designed to keep track of the progress of a software codebase.  This advice assumes you are using git on the command line (a terminal in Linux or MacOS, or a tool like Github's git shell for Windows).

To create a local copy of the official angband repository, use `git clone git://github.com/angband/angband.git`.  But if you want to participate in development, it is best not to do this straight away.  Instead, get an account at ​[Github](http://github.com), go to the official angband/angband repository and click Fork. This will create a new repository at ​`https://github.com/yourlogin/angband`.  This is the one you should clone locally using `git clone git://github.com/yourlogin/angband.git`.

This will create a local repository with several branches. Use `git branch -a` to see them:

* master
* origin/master
* (release branches and other things you don't need to worry about)

Do NOT do your work in your master branch: this is asking for trouble.  Create a new branch using `git checkout -b newbranch`, and do your work there.  Use `git commit -a` to commit your changes to your new branch and then build and test them.

Once you have tested your commits to your satisfaction, you can share them.  Assuming you have created a new branch and made your changes as described above, you can publish your changes to the world by using `git push origin newbranch` - this will make your new branch appear on Github for others to test. (It is advisable, but not essential, to use ssh keys for access to Github.)

Keep your thinking clear: separate your work into different branches for different things.  Create a branch called 'docs' if you want to work on some docs.  Create one called 'stores' if you want to make changes to stores.  And so on.  There is no limit to the number of branches you can have, and you can use `git checkout branchname` to switch between branches at any time. (Ideally you should commit any changes in the current branch before switching branches, but git does not like people undoing things so read up on `git stash` if you need to switch branches and don't want to either commit or lose the current changes.)

### Submitting a pull request

To submit a pull request, you will need to have a Github account and a repository "forked" from the offical angband repository.  We will call the repository "yourfork" and the branch with the bugfix or new feature "yourbranch".  We assume that you have called your remote of the official repo "official".

1. When you've finished and tested your work, publish it to Github using `git push origin yourbranch`.  Don't forget to rebase and fix any conflicts before pushing if necessary (`git fetch official; git rebase official/master`) - incorporating your work is much easier if it applies cleanly to the official/master branch.  Note that you must rebase before pushing, as rebase changes the commit IDs - this doesn't matter at all if the only place they exist is in your local repo. 

   * Please make sure that yourbranch contains only commits you want merged to the official repository.  If you have not kept your branches separate and there are commits relating to local changes or other work in yourbranch, things will get messy.  To solve this, create a copy of master (`git checkout official/master; git checkout -b yourbranch2`) and cherry-pick the commits you actually want to offer up (this method can also be used to avoid rebase conflicts, or at least isolate which commits are causing them.)  From yourbranch2 you can use `git rebase -i yourbranch` as a sort of batch cherry-pick mechanism: it offers you a list of all the commits which are in yourbranch but not in master, and you can choose the ones to add.  Note that rebase will say "nothing to do" if yourbranch will apply cleanly (i.e. fast-forward merge) to master - so this is a good test.

2. Go to your Github account in your browser and click on your fork. Don't forget to click the Branches tab and make sure you're looking at the right branch (Github will always default to the master branch).  Note that if you created yourbranch2 as described above, in order to tidy it up and offer only the right commits, then this is the one you need to look at. 

3. Click the "Pull Request" button, which is up near the top-right (just below the Search box). If you don't get a screen inviting you to write a description of the pull request, something has gone wrong (maybe you were not looking at the right branch, or your push didn't work). 

4. Write a description of what you're offering in the pull request. Please include details of any remaining issues (e.g. dependent on other tasks) or further related work you intend to do. Also, please include the issue number(s) (with a `#` in front of it) of any [issue](https://github.com/angband/angband/issues) that the request addresses (even if only partially).

5. Click the "Send Pull Request" button.

#### After you've issued the pull request

Often you'll think of some important fix or change after you've submitted a pull request. Don't worry - Github handles this very cleanly.  Just commit your additional changes to the branch from which you issued the pull request (i.e. from yourbranch, or from yourbranch2 if you had to cherry-pick and tidy it up), and push again.  Github will automatically add those commits to the pull request.

Please note, though, that you cannot rebase after submitting a pull request.  But that's not really your problem - once you've submitted the request, it's the Angband development team's job to review and merge it as soon as we can.  If we merge something else first, we'll fix any merge conflicts when we merge yours.

After your work is merged, please don't continue working on that branch.  Even if you're continuing development of the same feature, please fetch from official/master and start a new branch from there for your next pull request (there is no limit to the number of branches you can make).  If for any reason you don't want to do this, then you must rebase your branch on official/master before pushing and offering up your next pull request.  You should use rebase instead of merging from official/master, to avoid a proliferation of merge commits.

So the ideal loop is:

1. `git fetch official/master`
2. `git checkout official/master`
3. `git checkout -b newbranch`
4. ... do your work in newbranch ...
5. `git fetch official/master` again (to see if it has updated while you were working) 
   * (if it has) `git rebase official/master` (and fix any conflicts)
6. `git push origin newbranch`
7. Go to Github and open pull request
8. Wait for pull request to be merged (you can push more commits while waiting)
9. Go back to 1 and start again

### General tips

Don't be afraid to create and delete lots of branches.  They're totally expendable, and a new branch is a fresh start.

Always update your local copy of the official repository (`git fetch official`) before pushing your work.  If possible, use `git rebase official/master` to ensure that your changes are on top of the very latest commits from the official repo - that will make them easier to merge, and is much neater than merging official branches into your branches and having the Angband development team merge them back again.  If you are nervous about trying rebase on a branch with lots of your hard work in it, create a new copy of that branch first - so if the rebase goes horribly wrong, your original branch is untouched.  Do not use rebase if you have published your branch on Github though, as this will mess up anyone who is tracking it.

When submitting pull requests on Github, please ensure that you choose only the commits relevant to this request - try and avoid choosing merge commits or commits which are part of another pull request. 

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
