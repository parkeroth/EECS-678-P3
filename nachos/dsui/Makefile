#KUSP_INC_PATH = -I/yggnfs/nwatkins/kusproot/include
#DSUI_HEADER = /yggnfs/nwatkins/kusproot/bin/dsui-header
#
#DSUI_HEADER = dsui-header



#all: nachos_dsui.o

#nachos_dsui.o: nachos_dsui.c
#	gcc -DCONFIG_DSUI $(KUSP_INC_PATH) -o nachos_dsui.o -c nachos_dsui.c

#nachos.dsui: nachos_dsui.h
#nachos_dsui.c: nachos_dsui.h
#nachos_dsui.h: nachos.ns
#	$(DSUI_HEADER) --prefix nachos nachos.ns dstream_admin.ns
#	mv nachos.dsui nachos.dsui.tmp
#	mv nachos.pp nachos.pp.tmp
#	python fixup.py nachos.dsui.tmp nachos.dsui
#	python fixup.py nachos.pp.tmp nachos.pp
#	rm -f nachos.pp.tmp nachos.dsui.tmp

#histo_postprocess:
#	-rm *.histo
#	dstream_pipeline -c nachos.pp activity-hist
#	dstream_pipeline -c nachos.pp inactivity-hist
#	dstream_pipeline -c nachos.pp activity-periodic-hist
#	dstream_pipeline -c nachos.pp inactivity-periodic-hist

clean:
#	rm -f nachos.dsui nachos_dsui.c nachos_dsui.h nachos_dsui.o
