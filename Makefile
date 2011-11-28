# This make file exists only to standardize the creation of the TAR
# file for handing your work in to be grades
#

KUID = CHANGE_ME_TO_YOUR_KUID

all:
	@echo ""
	@echo "**************************************************************"
	@echo "Use \"bash> make tar-file\" to create the TAR file to hand in"
	@echo "Make sure you have redefined the KUID variable in this "
	@echo "   make file to be your KUID and renamed the directory"
	@echo "   containing this make file to be <your-kuid>_pa_vm"
	@echo "**************************************************************"

clean_nachos: 
	cd nachos; make clean

tar-file:
	@if test -d ../$(KUID)_pa_vm ; then \
		( cd .. ; \
		  tar cz $(KUID)_pa_vm >$(KUID)_pa_vm.tar.gz ; \
	  	  echo ""; \
		  echo "**************************************************************"; \
		  echo "The TAR file for you to test and hand in "; \
		  echo "   has been made in the directory above:"; \
		  echo "     " $(KUID)_pa_vm.tar.gz ; \
		  echo "**************************************************************"; \
		) ; \
	else \
		( echo "*** ERROR ** ERROR ** ERROR ** ERROR ** ERROR ** ERROR ***"; \
		  echo "The <your KUID>_pa_vm directory was not found. "; \
		  echo "Remember that you should rename the "; \
		  echo "\"starter-code\" directory created by the "; \
		  echo "starter TAR file and set the KUID " ; \
		  echo "variable in the Makefile"; \
		) ; \
	fi
