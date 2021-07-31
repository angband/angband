# Angband gh-pages site documentation

This is a simple script and kit of parts, originally to maintain the official Angband site, rephial.org, adapted to maintain essentially the same site but located at angband.github.io/angband.


## What do I do!?

Run `make` for help.


## How to update the gh-pages branch for a new release

1. Make sure the release is marked as a proper release on Github.
2. Edit `source/index` to replace the previous release URLs with the new ones.
3. Make a new `source/releases/<version number>` file (copy the previous one, change the URLs and description).
4. Run `git add source/releases/<version number>`.
5. Build the html files by running `make build`.
6. Manually copy `docs/release/<version number>.html` to docs/release.html.
7. (Optional) Complain about having to do step 6.
8. Run `git add docs/release/<version number>.html`.
9. Run `git commit -a -m "Your commit message"`.
10. Run `git push <remote> gh-pages`, where <remote> is your remote name for the official Angband site https://github.com/angband/angband.git.

Voila.
