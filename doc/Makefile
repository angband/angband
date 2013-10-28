MKPATH=../mk/
include $(MKPATH)buildsys.mk

HELPSRCPATH := ../lib/help/

RST2HTML := rst2html
RST2LATEX := rst2latex
PDFLATEX := pdflatex
MAKEINDEX := makeindex

SECTIONS := general birth playing attack dungeon customize option command \
            debug ../../thanks version readme

INDEX := index

CLEAN = manual.ind manual.ilg manual.toc manual.tex manual.log manual.aux \
	manual.tex manual.pdf manual.html manual.out

RSTFILESWITHOUTPATH := $(addsuffix .txt, $(SECTIONS))
RSTINDEXWITHOUTPATH := $(addsuffix .txt, $(INDEX))

RSTFILES := $(addprefix $(HELPSRCPATH), $(RSTFILESWITHOUTPATH))
RSTINDEX := $(addprefix $(HELPSRCPATH), $(RSTINDEXWITHOUTPATH))

STY := $(addprefix $(HELPSRCPATH), angband.sty)
CSS := $(addprefix $(HELPSRCPATH), angband.css)

manual.html: $(RSTINDEX) $(RSTFILES) $(CSS)
	${COMPILE_STATUS}
	@if $(RST2HTML) $(HELPSRCPATH)/index.txt --stylesheet-path $(CSS) > manual.html; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

manual.tex: $(RSTINDEX) $(RSTFILES) $(STY)
	${COMPILE_STATUS}
	@if $(RST2LATEX) $(HELPSRCPATH)/index.txt > manual.tex --stylesheet-path=$(STY) --documentclass=scrartcl; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

manual.pdf: manual.tex
	${COMPILE_STATUS}
	@$(PDFLATEX) manual.tex > /dev/null
	@$(MAKEINDEX) manual.tex > /dev/null 2>&1
	@if $(PDFLATEX) manual.tex > /dev/null; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

