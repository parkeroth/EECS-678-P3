# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

MAKE = make
LPR = lpr

.PHONY:	all clean print quick refreshDepend p3-table

all: refreshDepend quick

quick:
	$(MAKE) -C dsui_tmp
	$(MAKE) -C dsui-sched
	$(MAKE) -C dsui-vm
	$(MAKE) -C userprog nachos
	$(MAKE) -C lib
	$(MAKE) -C test

refreshDepend:
	$(MAKE) -C dsui_tmp
	$(MAKE) -C dsui-sched
	$(MAKE) -C dsui-vm
	$(MAKE) -C userprog depend 

p3-table: clean all
	cd dsui-vm/page_replacement_policy; make clean; make regress; make postprocess; make table



### Regression Tests
# Each regress.# includes a more complex behavior of nachos and mostly
# tests for segfaults and consistency in execution stats

# The output of a regression will be a text file with some of the execution information

# Can nachos build and execute a simple arithmetic program?
regress.0:
	(./userprog/nachos -x ./test/matmult || echo "Regress 0 failed to run") &> regress.0

# Can it run threads?
regress.1:
	(./userprog/nachos -x ./test/nice_free || echo "Regress 1 failed to run") &> regress.1

# Can it run all of the page replacement algorithms?
regress.2:
	((cd ./test && python run-nachos-tests.py) &> regress.2 && cat ./test/results >> regress.2) || (echo "Regress 2 failed to run" > regress.2)
	-\rm -f ./test/results ./test/*histo ./test/swapfile*

# Runs all of the above regressions and aggregates results
regress.all: regress.0 regress.1 regress.2
	echo '===== Regress 0 (basic arithmetic) =====' > ./regress.all
	cat regress.0 >> regress.all

	echo '===== Regress 1 (multi-threaded) =====' >> ./regress.all
	cat regress.1 >> regress.all

	echo '===== Regress 2 (PRPs) =====' >> ./regress.all
	cat regress.2 >> regress.all




# don't delete executables in "test" in case there is no cross-compiler
clean:
	-rm -f *~ */{core*,nachos,DISK,*.o,swtch.s,swap,swapfile*,.depend,.gdb_history} test/{*.coff,*~} bin/{coff2flat,disassemble,out,*~} lib/lib.a regress.* *.regress.all
	cd test; make clean
	cd dsui_tmp; make clean
	cd dsui-sched; make clean
	cd dsui-vm; make clean

print:
	$(LPR) Makefile* */Makefile
	$(LPR) threads/*.h threads/*.cc threads/*.s
	$(LPR) userprog/*.h userprog/*.cc
	$(LPR) filesys/*.h filesys/*.cc
	$(LPR) network/*.h network/*.cc
	$(LPR) machine/*.h machine/*.cc
	$(LPR) bin/coff.h
	$(LPR) test/*.h test/*.c test/*.s

TAGS:
	etags -l c++ --members bin/elf.h debug/*.{cc,h} filesys/*.{cc,h} \
	machine/*.{cc,h} network/*.{cc,h} threads/*.{cc,h} userprog/*.{cc,h}
