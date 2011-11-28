DSUI_HEADER = dsui-header

IS_LOCAL_INSTALL = no

KUSP_INCLUDE_FLAGS = 

ifeq ($(IS_LOCAL_INSTALL), yes)
	KUSP_INCLUDE_FLAGS = -I/opt/kusp/include
endif

all: nachos_dsui.o

nachos_dsui.o: nachos_dsui.c
	gcc -DCONFIG_DSUI $(KUSP_INCLUDE_FLAGS) -o nachos_dsui.o -c nachos_dsui.c -m32


nachos.dsui: nachos_dsui.h
nachos_dsui.c: nachos_dsui.h
nachos_dsui.h:
	$(DSUI_HEADER) --prefix nachos --binfile nachos.dsui.bin


regress:
	-rm -f *.histo
	python sched_postprocess.py -v

clean:
	rm -f nachos.dsui nachos_dsui.c nachos_dsui.h nachos_dsui.o *.histo
