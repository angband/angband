@echo off
copy %1.bak %1.3
copy %1 %1.bak
angband %2 %3 %4 %5 %6 %7 %8 %9 %1
