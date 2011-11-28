import sys
import string
import subprocess
from os.path import abspath, join

pp_config_outer_tmpl = """
<VM-Postprocessing>
filter_modules = "prpfilters.py"
filters = {
%s
}
"""

pp_config_inner_tmpl = """
    f%d = [
        head.input (
            file = "%s"
        )
        prpfilters.prp()
    ]
"""

dsui_config_tmpl = """
<dsui>
output_file = "%s"

<dsui_enabled>
EXCEPTION = [
    UserTicksSinceLastPageFault
]
STATS = [
    PAGE_FAULTS
    PAGE_INS
    PAGE_OUTS
]
ARGS = [
    PRP
    PROGRAM
    DELTA
]
"""

bin_output_dir = abspath("bins/")
conf_output_dir = abspath("configs/")
swap_dir = abspath("swaps/")

#
# Behavior drivers
#
prps = ['dumb','fifo','lru','secondchance'] # which page replacement policies to run
deltas = [1,2,4,8]          # sizes of the reference history window
#programs = [ 'access1', 'access2', 'matmult', 'access3', 'access4' ]    # which apps to run
programs = [ 'access1', 'access2', 'access3', 'access4' ]    # which apps to run

### create option sets
option_sets = [ { 'prp' : prp
                , 'delta' : delta
                , 'program' : program
        , 'tag' : "%s_%s_%d" % (program, prp, delta)
                }
                for program in programs
                for prp in prps
                for delta in deltas
             ]

def genProcess( option_set ):
    # prepare the list of arguments
    swapfilename = 'swaps/swapfile_%(prp)s_%(delta)s_%(program)s' % option_set
    arg_list = [ '../../userprog/nachos'
               , '-S'
               , swapfilename
           , '--dsui-config'
           , "%s.dsui" % (join(conf_output_dir, option_set['tag']),)
           , '-prp'
               , str( option_set['prp'] )
               , '-delta'
               , str( option_set['delta'] )
               , '-x'
               , "../../test/%s" % str( option_set['program'] )
               ]

    # create the process
    cmdStr = string.join(arg_list)
    print "Starting subprocess: %s" % cmdStr
    process = subprocess.Popen( arg_list , stdout=subprocess.PIPE , stderr=subprocess.STDOUT )

    # return the process label and process object
    label = ( option_set['program'] , option_set['prp'] , option_set['delta'] )
    pair = ( label , process )

    return pair

def genDSUIConfs():
    pp_config = open("prp.pipes", "w")
    #makefile = open("Makefile.prp", "w")
    #makefile.write("all:\n")
    #makefile.write("\trm -rf prp.pipes.out\n")
    #makefile.write("\trm -rf data/*\n")
    pp_configs = ""
    for option,i in zip(option_sets, range(0,len(option_sets))):
        tag = option['tag']
        dsui_conf_filename = join(conf_output_dir, "%s.dsui" % (tag,))
        dsui_bin_filename = join(bin_output_dir, "%s.bin" % (tag,))
        dsui_conf_file = open(dsui_conf_filename, 'w')
        dsui_conf_file.write(dsui_config_tmpl % (dsui_bin_filename,))
        dsui_conf_file.close()
        pp_configs += (pp_config_inner_tmpl % (i, dsui_bin_filename))
        #makefile.write("\tpostprocess f prp.pipes %s >> prp.pipes.out&\n" % tag)
        pass
        #makefile.write("\tpostprocess f prp.pipes>> prp.pipes.out&\n")
    pp_config.write(pp_config_outer_tmpl % pp_configs)
    pp_config.close()
    #makefile.close()
    pass

### create processes
genDSUIConfs()
#processes = [ genProcess( option_set )
#              for option_set in option_sets
#              ]

# while gathering, collect all stdout streams in one file
aggregate_stdout = file( 'results' , 'w' )

for i in range(0, len(option_sets), 4):
    processes = [ genProcess(option_set)
            for option_set in option_sets[i:(i+4)]
            ]

    #read the stdouts of each subprocess
    for pair in processes:
        (label,process) = pair  # unpack
        (program,prp,delta) = label # unpack
    
        retcode = process.wait();
        # start a new chunk in the aggregate stdout file
        aggregate_stdout.write( '-' * 80 + '\n' )
        aggregate_stdout.write( str( label ) + '\n' * 2 )
        for line in process.stdout:
            aggregate_stdout.write( line )
            pass
        pass
    pass
