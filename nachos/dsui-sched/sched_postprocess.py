#! /bin/env python
"""
:program:`sched_postprocess.py`
=====================================

.. moduleauthor:: Dillon Hicks <hhicks@ittc.ku.edu>

Overview 
----------------------

This program executes the nice_free program with a series of
parameters in order to test the SCHED Assignment solution. The allowed
changeable parameters are the Reduction Factor and Inertia with which
to execute nice_free. After executing the nice_free, this program then
executes the DSUI post-processing in order to get the active and
inactive interval histograms data for each test. The interval
histogram files are then transformed by the
create_nachos_histograms.py program to create matplotlib graphs for
each pair of active and inactive intervals for each test.

Program Modification for Extended Testing
-------------------------------------------

To modify the series of test executed by this program, you will have
to modify the *test_parameters* tuple. The *test_parameters* tuple
contains additional tuples for each of its elements. The tuple
elements have the form::

    ( REDUCTION_FACTOR, INERTIA ) 

Such that test_parameters has the form::

    test_parameters = ( 
                      (REDUCTION_FACTOR_1, INERTIA_1),
                      (REDUCTION_FACTOR_2, INERTIA_2),
                      (REDUCTION_FACTOR_3, INERTIA_3),

                      ...

                      (REDUCTION_FACTOR_N, INERTIA_N)
                      )

Where each pair corresponds to the set of parameters for each
individual execution of the nice_free test program. 



"""
import sys
import os

##############################################
#     TESTS TO EXECUTE
##############################################

# The list of tests to run in the form:
#   ( REDUCTION_FACTOR, INERTIA) 
test_parameters = (
                  (0.0,  1), 
                  (0.25, 1),                                                
                  (0.5,  1),                                           
                  (0.75, 1),
                  (1.0,  1),
                  
                  # Extra Credit
                  #(0.0, 2),
                  #(0.0, 4),
                  #(0.0, 8)
                  )


############ END TESTS DATA ##################

##############################################
# DSUI CONFIGURATION AND POSTPROCESSING DATA
##############################################

# The postprocessing for each test is similar so the template provides
# a general outline that allows us to "fill in the blanks" using
# Python's clever string substitution. 
#
DSUI_CONFIGURATION_TEMPLATE = \
"""
<main>
filter_modules = "nachosfilters.py"
filters = {
	main = [
		head.input(
			namespace  = {
				THREAD_ACTIVITY = {
					desc = "thread activity intervals"
					entities = {
						ACTIVITY_PERIOD = interval(
							desc = "period of activity"
						)
						INACTIVITY_PERIOD = interval(
							desc = "period of inactivity"
						)
					}
				}
			}
			file = "%(DSUI_DATA_DIR)s/%(DSUI_PREFIX)s.dsui.bin"
		)

		nachosfilters.CalculateThreadIntervalsFilter()
		nachosfilters.ThreadStateIntervalFilter(activity_outfile="%(HISTO_OUTPUT_DIR)s/%(DSUI_PREFIX)s.active.histogram.bin",
                                                        inactivity_outfile="%(HISTO_OUTPUT_DIR)s/%(DSUI_PREFIX)s.inactive.histogram.bin")
	]
}
"""

# Each time a test of nice_free is executed the test designation
# string "R<reduction>i<inertia>" (i.e. "R0.24i1") is added to the dsui_test_outputs
# list in order to track each test that has been executed so that
# postprocessing can be performed on each of the dsui output files. 
dsui_test_outputs = []

######### END DSUI CONFIGURATION #############


##############################################
# INSTRUMENTATION DIRECTORIES
##############################################

# The output directory for the dsui binary files produced by each test
# execution.
#
DATA_DIR = "data"

# The intermediate directory of the pickled interval histograms
# produced by postprocessing the dsui binary file for each test. Each
# binary file will produce two histograms files, one for each the
# active and inactive interval state for each thread.
#
HISTOGRAM_DIR = "histograms"

# The final output directory for all pdfs created from the pickled
# interval histogram files.
PDFS_DIR = "pdfs"

TESTOUTS_DIR = "test-outs"

# The tuple of relevant directories for processing the dsui
# instrumentation data. Makes creating 'batch' algorithms a little
# easier.
processing_directories = ( DATA_DIR,
                           HISTOGRAM_DIR,
                           PDFS_DIR,
                           TESTOUTS_DIR
                         )

##### END INSTRUMENTATION DIRECTORIES ######

############################################
#  UTILITY FUNCTIONS
############################################

def vprint(v_string):
    if Params.verbose_level >= 1 or \
            Params.debug_level >= 1:
        print v_string

############################################
#  MAIN TEST PROGRAM FUNCTIONS
############################################

def prepare_directories():
    """
    Creates all of the necessary directories to hold the post
    processing binary fines, histogram interval files and final
    histogram graph .pdf files. If the directories already exist, they
    are destroyed and a new directory is created.
    """

    vprint("Preparing SCHED output directories:")
    for dir in processing_directories:
        vprint(dir)
        if os.path.exists(dir):
            os.system("rm -rf %s" % dir)
        os.mkdir(dir)    
    os.system("cp nachosfilters.py %s" % HISTOGRAM_DIR)
    

def execute_tests():
    """
    Executes each test of nice_free defined by *test_parameters* under
    the Nachos simulator. Each tests dsui_prefix is used as a unique
    identifier and subsequently added to the dsui_test_ouputs list for
    later use in post processing.
    """
    vprint("Executing SCHED Tests:")
    os.chdir(DATA_DIR)
    for (reduction, inertia) in test_parameters:
        dsui_prefix = "R%.2fi%i" % (reduction, inertia)
        vprint(dsui_prefix)
        dsui_test_outputs.append(dsui_prefix)    
        cmd = "../../userprog/nachos -d nice -x ../../test/nice_free " + \
              "--dsui-output %s.dsui.bin -R %2f -i %s > ../%s/r%.2fi%i.out" \
               % (dsui_prefix, reduction, inertia, 
                  TESTOUTS_DIR, reduction, inertia)
        vprint(cmd)
        os.system(cmd)
    os.chdir("../")

    
def execute_postprocess():
    """
    Postprocess each of the Data Stream binary files into two interval
    histogram files, one active interval file and one inactive
    interval file. Then output these histogram files into the
    HISTOGRAM_DIR.
    """
    os.chdir(HISTOGRAM_DIR)

    template_dict = { "DSUI_PREFIX" : None,
                      "HISTO_OUTPUT_DIR" : "../%s" % HISTOGRAM_DIR,
                      "DSUI_DATA_DIR" : "../%s" % DATA_DIR}    

    vprint('Postprocessing SCHED Tests:')

    for dsui_prefix in dsui_test_outputs:
        vprint(dsui_prefix)
        template_dict["DSUI_PREFIX"] = dsui_prefix
        pipes_filename = "%s.pipes" % dsui_prefix
        configfile_text = DSUI_CONFIGURATION_TEMPLATE % ( template_dict )
        dsui_configfile = open(pipes_filename, 'w')
        dsui_configfile.write(configfile_text)
        dsui_configfile.close()
        os.system("postprocess f %s >& /dev/null" % pipes_filename)
    
    os.chdir("../")



def graph_histograms():
    """
    Creates matplotlib histogram pdfs from the histogram interval
    files in HISTOGRAM_DIR with create_nachos_histograms.py.
    """
    os.chdir(PDFS_DIR)
    command_dict = { "DSUI_PREFIX" : None,
                     "HISTO_OUTPUT_DIR" : "../%s" % HISTOGRAM_DIR }
    vprint('Creating SCHED histogram graphs:')
    for dsui_prefix in dsui_test_outputs:
        command_dict["DSUI_PREFIX"] = dsui_prefix
        # vprint('%s Active' % dsui_prefix)
        # os.system("python ../create_nachos_histograms.py "
        #           "-f %(HISTO_OUTPUT_DIR)s/%(DSUI_PREFIX)s.active.histogram.bin "
        #           "-t \"%(DSUI_PREFIX)s Active Intervals\" "
        #           "--buckets=20 --min=60 --max=1200 "
        #           "-o %(DSUI_PREFIX)s.active.pdf" % command_dict)
        vprint('%s Inactive' % dsui_prefix)
        os.system("python ../create_nachos_histograms.py "
                  "-f %(HISTO_OUTPUT_DIR)s/%(DSUI_PREFIX)s.inactive.histogram.bin "
                  "-t \"%(DSUI_PREFIX)s Inactive Intervals\" "
                  "--buckets=20 "
                  "-o %(DSUI_PREFIX)s.inactive.pdf" % command_dict)
    os.chdir("../")

########## END TESTING FUNCTIONS ############




#############################################
#
# This program uses an easily extend able optparse template to allow
# for future customization via command line arguments. It already has
# implementations of the standard verbose and debug options as a
# guide.
#

if __name__ == '__main__':
    # imports required if this module is called as a
    # command
    import optparse, sys, os
    from pprint import *
    
    # Define the set of permitted parameters, including the
    # command arguments.  The initialization method creates
    # the parser and defines the defaults. The parse()
    # method actually parses the arguments one the command
    # line. This was done so that the instance of the class
    # could be global and thus available to all
    # routines. and then parse the arguments to this call
    # according to the specification
    class Params_Set:
        USAGE = "usage: %prog [options]"
    
        def __init__(self):
            # Create the argument parser and then tell it
            # about the set of legal arguments for this
            # command. The parse() method of this class
            # calls parse_args of the optparse module
            self.p = optparse.OptionParser(usage=self.USAGE)
    
            # Boring and totally standard verbose and
            # debugging options that should be common to
            # virtually any command
            #
            self.p.add_option("-d", action="store_const", const=1,        
                              dest="debug_level", 
                              help="Turn on diagnostic output at level 1")
            self.p.add_option("-D", action="store", type ="int",    
                              dest="debug_level", 
                              help="Turn on diagnostic output at level DEBUG_LEVEL")
            self.p.add_option("-v", action="store_const", const=1,        
                              dest="verbose_level", 
                              help="Turn on narrative output at level 1")
            self.p.add_option("-V", action="store", type ="int",    
                              dest="verbose_level", 
                              help="Turn on narrative output at level VERBOSE_LEVEL")
           

            # Now tell the parser about the default values of all the options
            # we just told it about
            self.p.set_defaults(
                debug_level     = 0,          
                verbose_level   = 0,
           
                )       
            
        def parse(self):
            self.options, self.args = self.p.parse_args()
            self.debug_level     = self.options.debug_level    
            self.verbose_level   = self.options.verbose_level  
                       
            # Output option details if debugging level is high enough
            if self.debug_level >= 3 :
                print
                print "Options: ", self.options
                print "Args: ", self.args
    
        # Defining this method defines the string representation of the
        # object when given as an argument to str() or the "print" command
        #
        def __str__(self):
            param_print_str = \
    """Parameters:
      debug_level    : %d
      verbose_level  : %d
    """ 
            str_output = param_print_str % \
                (self.debug_level, 
                 self.verbose_level )
            
            return str_output
        




    def main():
        # Global level params class instance was
        # created before calling main(). We make it
        # global so that other code can access the set
        # of Parameters, simply by accessing the Params
        # instance. Here, however, we call the parse()
        # method to actually get the arguments, since
        # we have been called from the command line.
        Params.parse()
        debug_level = Params.debug_level
        if Params.debug_level >= 2:
            vprint(Params)
            
       
        # Test program execution -- following the execution path of
        # these functions will help anyone wanting to majorly change
        # this program discern its structure if it is not clear.
        #     
        prepare_directories()
        execute_tests()
        execute_postprocess()
        graph_histograms()
        
    
    global Params
    Params = Params_Set()    
    main()

