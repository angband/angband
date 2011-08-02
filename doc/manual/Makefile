MKPATH=../../mk/
include $(MKPATH)buildsys.mk

SECTIONS := general birth playing attack dungeon customize option command \
	debug thanks version
TEXFILES := $(SECTIONS:=.tex)

PDFLATEX  := pdflatex
MAKEINDEX := makeindex 

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
	

clean::
	for i in $(SECTIONS:=.aux) manual.aux manual.idx manual.ilg \
		manual.ind manual.log manual.out manual.toc; do \
		if test -f $$i -o -d $$i; then \
			if rm -fr $$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done
