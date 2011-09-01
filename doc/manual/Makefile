MKPATH=../../mk/
include $(MKPATH)buildsys.mk

SECTIONS := general birth playing attack dungeon customize option command \
	debug thanks version
TEXFILES := $(SECTIONS:=.tex)

PDFLATEX  := pdflatex
MAKEINDEX := makeindex 

CLEAN = $(SECTIONS:=.aux) manual.aux manual.idx manual.ilg \
	manual.ind manual.log manual.out manual.toc

manual: manual.pdf

manual.pdf: manual.tex $(TEXFILES)
	${COMPILE_STATUS}
	@$(PDFLATEX) $< >/dev/null
	@$(MAKEINDEX) $(<:.tex=.idx) >/dev/null 2>&1
	@if $(PDFLATEX) $< >/dev/null; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi
