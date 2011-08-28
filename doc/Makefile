MKPATH=../mk/
include $(MKPATH)buildsys.mk

HELPSRCPATH := ../lib/help/

SECTIONS := general birth playing attack dungeon customize option command \
            debug thanks version 330

RSTFILESWITHOUTPATH := $(addsuffix .txt, $(SECTIONS))

RSTFILES := $(addprefix $(HELPSRCPATH), $(RSTFILESWITHOUTPATH))

manual.html: $(RSTFILES)
	rst2html $(HELPSRCPATH)/index.txt > manual.html

manual.tex: $(RSTFILES)
	rst2latex $(HELPSRCPATH)/index.txt > manual.tex

manual.pdf: manual.tex
	pdflatex manual.tex
	makeindex manual.tex
	pdflatex manual.tex

