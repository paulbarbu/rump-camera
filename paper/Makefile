all: dvi pdf ps

dvi:
	latex Barbu-Paul-Gheorghe-ICDD-2016.tex
	latex Barbu-Paul-Gheorghe-ICDD-2016.tex

pdf:
	pdflatex Barbu-Paul-Gheorghe-ICDD-2016.tex
	pdflatex Barbu-Paul-Gheorghe-ICDD-2016.tex

ps: dvi
	dvips Barbu-Paul-Gheorghe-ICDD-2016.dvi

clean:
	rm -rf *.aux *.log *.pdf
