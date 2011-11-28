#FIXME - KUSP_INC = -I$(KUSP_INCLUDE)
DSUI_HEADER = dsui-header

IS_LOCAL_INSTALL = no

KUSP_INCLUDE_FLAGS = 

ifeq ($(IS_LOCAL_INSTALL), yes)
	KUSP_INCLUDE_FLAGS = -I/opt/kusp/include
endif

all: nachos_dsui.o

nachos_dsui.o: nachos_dsui.c
	gcc -DCONFIG_DSUI $(KUSP_INCLUDE_FLAGS) -o nachos_dsui.o -c nachos_dsui.c -m32

#FIXME - gcc -DCONFIG_DSUI $(KUSP_INC) -o nachos_dsui.o -c nachos_dsui.c

nachos.dsui: nachos_dsui.h
nachos_dsui.c: nachos_dsui.h
nachos_dsui.h: nachos.ns
	$(DSUI_HEADER) --prefix nachos --binfile nachos.dsui.bin


histo_postprocess:
	-rm -f *.histo
	postprocess f nachos.pipes




# Make calls for creating postscript files for various histograms.
sinactive:
	python histos2gnuplot.py sinactive.ps "Inactivity Durations with Static" [ABCD].inactive.histo | gnuplot

sactive:
	python histos2gnuplot.py sactive.ps "Activity Durations with Static" [ABCD].active.histo | gnuplot

a-inactive:
	python histos2gnuplot.py a-inactive.ps "Inactivity for A" A\@*.inactive.histo | gnuplot

a-active:
	python histos2gnuplot.py a-active.ps "Activity for A" A\@*.active.histo | gnuplot

b-inactive:
	python histos2gnuplot.py b-inactive.ps "Inactivity for B" B\@*.inactive.histo | gnuplot

b-active:
	python histos2gnuplot.py b-active.ps "Activity for B" B\@*.active.histo | gnuplot

c-inactive:
	python histos2gnuplot.py c-inactive.ps "Inactivity for C" C\@*.inactive.histo | gnuplot

c-active:
	python histos2gnuplot.py c-active.ps "Activity for C" C\@*.active.histo | gnuplot

d-inactive:
	python histos2gnuplot.py d-inactive.ps "Inactivity for D" D\@*.inactive.histo | gnuplot

d-active:
	python histos2gnuplot.py d-active.ps "Activity for D" D\@*.active.histo | gnuplot

static_histos:
	-rm -f *.histo
	../userprog/nachos -d nice --dsui-output nachos.dsui.bin -x ../test/nice_free
	postprocess f nachos.pipes
	python histos2gnuplot.py sinactive.ps "Inactivity Durations with Static" [ABCD].inactive.histo | gnuplot
	python histos2gnuplot.py sactive.ps "Activity Durations with Static" [ABCD].active.histo | gnuplot
	ps2pdf sinactive.ps
	ps2pdf sactive.ps

clean:
	rm -f nachos.dsui nachos_dsui.c nachos_dsui.h nachos_dsui.o *.histo
