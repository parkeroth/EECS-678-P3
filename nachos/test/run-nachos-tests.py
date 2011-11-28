# Nicolas Frisby
# 13 Nov 2005

# This script is a suitable replacement for run-nachos-test.  It runs
# NachOS simluations on the test access loads and outputs tables
#

import sys
import subprocess
import string
import re

#
# Behavior drivers
#
prps = ['dumb','fifo','lru','secondchance']	# which page replacement policies to run
deltas = [1,2,4,8]			# sizes of the reference history window
programs = [ 'access1', 'access2', 'matmult', 'access3', 'access4' ]	# which apps to run

# these have been determined via the TA's solution; your mileage may vary
hspecs = { 'access1' : '76,10,0;20,10,0'
           , 'access2' : '41,1,1;20,10,0'
           , 'access3' : '76,10,0;20,10,0'
           , 'access4' : '78,10,0;20,10,0'
           , 'matmult' : '47,1000,0;20,10,0'
           }

#
# checkDependencies
#
# Checks if NachOS and test programs are available as expected
#
def checkDependencies():
    class ReqNotFound(Exception):
        def __init__(self,missing):
            self.missing = missing or "a required file"
        def __str__(self):
            return ("could not find '%s'" % self.missing)

    reqs = [ '../userprog/nachos' ]
    reqs.extend( programs )

    found = True

    try:
        for req in reqs:
            try:
                if not file( req ):
                    raise ReqNotFound, req
            except IOError:
                raise ReqNotFound, req
    except ReqNotFound, missing_req:
        print ( "Aborting: %s" % str( missing_req ) )
        found = False

    return found

#
# genProcess
#
# This script initially spawned multiple instances of NachOS and
# allowed them to interleave their execution.  However, the NachOS instances  would munge the swapfile.
#
# The -S option to NachOS allows the name of the swapfile to be
# set. Thus each process is given a unique swapfile so that they can
# run concurrently. At the current time, this causes a test suite of
# 32 NachOS instances to run in 13s instead of 19s.
#
def genProcess( option_set ):
    # prepare the list of arguments
    swapfilename = 'swapfile_%(prp)s_%(delta)s_%(program)s' % option_set
    arg_list = [ '../userprog/nachos'
               , '-S'
               , swapfilename
               , '-prp'
               , str( option_set['prp'] )
               , '-delta'
               , str( option_set['delta'] )
               , '-x'
               , str( option_set['program'] )
               , '-H'
               , str( hspecs[ option_set['program'] ] )
               , '-d'
               , 'pfhisto'
               ]

    # create the process
    cmdStr = string.join(arg_list)
    print "Starting subprocess: %s" % cmdStr
    process = subprocess.Popen( arg_list , stdout=subprocess.PIPE , stderr=subprocess.STDOUT )

    # return the process label and process object
    label = ( option_set['program'] , option_set['prp'] , option_set['delta'] )
    pair = ( label , process )

    return pair

#
# writeTable
#
# Writes out a table of results
#
def writeTable( table , result_type , extraction_re ):
    # FIXME: make these calculations at runtime
    min_prp_width = 15
    min_results_width = 15
    num_deltas = len( deltas )

    for program in programs:
        print "\nProgram = %s:" % program
        print '-' * ( min_prp_width+3 + (min_results_width+2)*num_deltas + num_deltas)

        # add headers to table
        table[program][''] = {}
        for delta in deltas:
            table[program][''][delta] = {}
            table[program][''][delta][result_type] = 'delta = %d' % delta

        # make list of rows
        row_labels = ['']
        row_labels.extend( prps )

        # print each row
        for row in row_labels:
            t = table[program][row]

            print ("%s ||" % string.ljust( row , min_prp_width ) ),

            for delta in deltas:
                re_match = extraction_re.search( t[delta][result_type] )

                if re_match:
                    results = "%s/%s" % re_match.groups()
                elif not -1 == string.find( t[delta][result_type] , '=' ):
                    results = t[delta][result_type]
                else:
                    results = '---'

                print ("%s |" % string.ljust( results , min_results_width ) ),

            print # end the row

### MAIN ###

if not checkDependencies():
    sys.exit( 1 )

### create option sets
option_sets = [ { 'prp' : prp
                , 'delta' : delta
                , 'program' : program
                , 'hspec' : hspecs[ program ]
                }
                for program in programs
                for prp in prps
                for delta in deltas
             ]

### create processes
processes = [ genProcess( option_set )
              for option_set in option_sets
              ]

### gather results
# while gathering, collect all stdout streams in one file
aggregate_stdout = file( 'results' , 'w' )

# patterns for extracting the stdout lines of interest
paging_re = re.compile( r"Paging:" )
ticks_re = re.compile( r"Ticks:" )

# create empty table
table = {}
for program in programs:
    table[program] = {}
    for prp in prps:
        table[program][prp] = {}
        for delta in deltas:
            table[program][prp][delta] = {}
            table[program][prp][delta]['paging'] = None
            table[program][prp][delta]['ticks'] = None

# read the stdouts of each subprocess
for pair in processes:
    (label,process) = pair	# unpack
    (program,prp,delta) = label	# unpack

    retcode = process.wait();

    # start a new chunk in the aggregate stdout file
    aggregate_stdout.write( '-' * 80 + '\n' )
    aggregate_stdout.write( str( label ) + '\n' * 2 )
    
    for line in process.stdout:
        aggregate_stdout.write( line )

        # store results in table
        # (could catch more kinds of output in the future)
        result_type = None
        if paging_re.match( line ):
            result_type = 'paging'
        elif ticks_re.match( line ):
            result_type = 'ticks'
        table[program][prp][delta][result_type] = string.rstrip( line )

# print the table
extract_paging_re = re.compile( r"faults (\d+), pageins \d+, pageouts (\d+)" )
extract_ticks_re = re.compile( r"total \d+, idle \d+, system (\d+), user (\d+)" )

print
print "Entries are of the form: page faults/pageouts"
writeTable( table , 'paging' , extract_paging_re )

#print
#print "Entries are of the form: system ticks/user ticks"
#writeTable( table , 'ticks' , extract_ticks_re )
