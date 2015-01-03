MKPATH=../mk/
include $(MKPATH)buildsys.mk

HELPSRCPATH := ../lib/help/

MAKEINDEX := makeindex

DOC = manual.html manual.pdf

SECTIONS := general birth playing attack dungeon customize option command \
            debug ../../thanks version

INDEX := index

CLEAN = manual.ind manual.ilg manual.toc manual.tex manual.log manual.aux \
	manual.tex manual.pdf manual.html manual.out

RSTFILESWITHOUTPATH := $(addsuffix .txt, $(SECTIONS))
RSTINDEXWITHOUTPATH := $(addsuffix .txt, $(INDEX))

RSTFILES := $(addprefix $(HELPSRCPATH), $(RSTFILESWITHOUTPATH))
RSTINDEX := $(addprefix $(HELPSRCPATH), $(RSTINDEXWITHOUTPATH))

STY := $(addprefix $(HELPSRCPATH), angband.sty)
CSS := $(addprefix $(HELPSRCPATH), angband.css)

ifneq ($(RST2HTML),NOTFOUND)
manual.html: $(RSTINDEX) $(RSTFILES) $(CSS)
	${COMPILE_STATUS}
	@if $(RST2HTML) $(HELPSRCPATH)/index.txt --stylesheet-path $(CSS) > manual.html; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi
else
manual.html:
	echo rst2html not found. Cannot build HTML manual
endif

ifneq ($(RST2LATEX),NOTFOUND)
manual.tex: $(RSTINDEX) $(RSTFILES) $(STY)
	${COMPILE_STATUS}
	@if $(RST2LATEX) $(HELPSRCPATH)/index.txt > manual.tex --stylesheet-path=$(STY) --documentclass=scrartcl; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

ifneq ($(PDFLATEX),NOTFOUND)
manual.pdf: manual.tex
	${COMPILE_STATUS}
	@$(PDFLATEX) manual.tex > /dev/null
	@$(MAKEINDEX) manual.tex > /dev/null 2>&1
	@if $(PDFLATEX) manual.tex > /dev/null; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi
else
manual.pdf: manual.tex
	echo pdflatex not found. Cannot build PDF manual
endif
else
manual.pdf:
	echo rst2latex not found. Cannot build PDF manual
endif

install: manual.html
