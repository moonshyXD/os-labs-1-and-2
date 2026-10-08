MAIN   = report
LATEX ?= xelatex
FLAGS  = -interaction=nonstopmode -halt-on-error
SOURCES = $(wildcard ../src/*.c ../src/*.h) ../src/Makefile

$(MAIN).pdf: $(MAIN).tex preamble.tex titlepage.tex $(wildcard inc/*) $(SOURCES)
	$(LATEX) $(FLAGS) $(MAIN).tex
	$(LATEX) $(FLAGS) $(MAIN).tex

xelatex:
	$(MAKE) LATEX=xelatex

clean:
	rm -f *.aux *.log *.out *.lol *.toc *.fls *.fdb_latexmk *.synctex.gz

.PHONY: xelatex clean
