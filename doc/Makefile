MKPATH=../mk/
include $(MKPATH)buildsys.mk

HELPSRCPATH := ../lib/help/

RST2HTML := rst2html
RST2LATEX := rst2latex
PDFLATEX := pdflatex
MAKEINDEX := makeindex

SECTIONS := general birth playing attack dungeon customize option command \
            debug thanks version 330

INDEX := index

RSTFILESWITHOUTPATH := $(addsuffix .txt, $(SECTIONS))
RSTINDEXWITHOUTPATH := $(addsuffix .txt, $(INDEX))

RSTFILES := $(addprefix $(HELPSRCPATH), $(RSTFILESWITHOUTPATH))
RSTINDEX := $(addprefix $(HELPSRCPATH), $(RSTINDEXWITHOUTPATH))

manual.html: $(RSTINDEX) $(RSTFILES)
	${COMPILE_STATUS}
	@if $(RST2HTML) $(HELPSRCPATH)/index.txt > manual.html; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

manual.tex: $(RSTINDEX) $(RSTFILES)
	${COMPILE_STATUS}
	@if $(RST2LATEX) $(HELPSRCPATH)/index.txt > manual.tex; then \
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

clean:
	for i in manual.ind manual.ilg manual.toc manual.tex manual.log \
	manual.aux; do \
		if test -f $$i -o -d $$i; then \
			if rm -fr $$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

