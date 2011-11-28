"""
:program:`create_nachos_histograms.py`
=======================================
    :synopsis: Create various histograms from pickled Nachos interval by thread dictionaries.

.. seealso:: :mod:`nachosfilters`

.. moduleauthor:: Dillon Hicks <hhicks@ittc.ku.edu>

TODO
--------

* Per Thread interval Filters.
* More dynamic statistics based histogram ranges.
* Display extra histogram statistics 
    (hits outside range, hit %, etc)

"""
import cPickle
import matplotlib.pyplot as plt
from string import uppercase

# Currently when logging N different Nachos threads with Data Streams,
# DSUI logs N+1 threads where the first thread is the NACHOS
# Administrative Thread that will (generally) throw off the generation
# of histograms and their graphical representations. It is best to
# skip this thread since we know it to always be the first thread.
# (i.e. Thread PID 1)
SKIP_ADMIN_THREAD_ONE = True



def create_histograms_from_pickled_file(filepath, outfile, title):
    """
    Creates a histogram from the pickled interval dictionary file.

    :param filepath: The filepath of the pickled interval file.
    :param outfile: The filepath to the outfile to 
        which to save the generated image.
    :param title: The title of the histogram graph.
    """
    intervals_by_thread_dict = cPickle.load(open(filepath, 'r'))

    if SKIP_ADMIN_THREAD_ONE:
        del intervals_by_thread_dict[1]
    
    all_intervals = intervals_by_thread_dict.values()

    # Creates a list of minimum values where the list is the minimum
    # value for the intervals for each thread. Then find the minimum
    # value in the list of minimum values, this is the minimum value
    # to be used by the histogram.
    min_value = min( min(thread_intervals) 
                     for thread_intervals in all_intervals )

    # Creates a list of maximum values where the list is the maximum
    # value for the intervals for each thread. Then find the maximum
    # value in the list of maximum values, this will be ~95% of the
    # maximum value for the range of the histogram.
    max_value = max( max(thread_intervals) 
                     for thread_intervals in all_intervals )

    # Increase the maximum value by a bit to have some padding.
    max_value = int(max_value*1.05)
    
    n, bins, patches = plt.hist( all_intervals,
                                 10, range=(min_value, max_value), 
                                 histtype='bar')
    
    # A minor hack because of the nature of our histograms and matplot
    # lib. This gets all of the histogram color rectangles that are
    # used to color code the intervals by thread. To note, this list
    # is in the same order as the list of interval list sent to
    # plt.hist to make the histograms. Since order is preserved we can
    # use this information with the color blocks to label/legend the
    # histogram.
    histogram_patches = []
    for patch_list in patches:
        histogram_patches.append(patch_list[0])
        
    # Axes labels, graph title and display grid options.
    plt.xlabel("Nachos Machine Timer Ticks (tsc)\nEach Bucket Represents an Interval of Machine Ticks")
    plt.ylabel("Bucket Hit Count")
    plt.title(title)
    plt.grid(True)

    # Create the ledgend using the histogram color patches and
    # matching them to the thread_id (or thread_name) values.
    plt.legend(histogram_patches, ("%s"%thread_id for thread_id in intervals_by_thread_dict.keys()))
    
    # Save the histogram file to OUTFILE.
    plt.savefig(outfile)



########################################################
#
# All of the test modules will need the same options front
# end, so this can be imported where one would normally
# place the optparse template. 
#
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
            self.p.add_option('-f', action='store', type='string',
                              dest="filepath", 
                              help="The FILEPATH to the pickled file that contains the"
                                   " interval data from which to make the histograms.")
            self.p.add_option('-t', action='store', type='string',
                              dest='title',
                              help='The TITLE to give to the created image.')
            self.p.add_option('-o', action='store', type='string',
                              dest='outfile',
                              help='The OUTFILE of the image.')

            # Now tell the parser about the default values of all the options
            # we just told it about
            self.p.set_defaults(
                debug_level     = 0,          
                verbose_level   = 0,
                filepath        = None,
                title           = '',
                outfile         = None
                )       
            
        def parse(self):
            self.options, self.args = self.p.parse_args()
            self.debug_level     = self.options.debug_level    
            self.verbose_level   = self.options.verbose_level  
            self.filepath        = self.options.filepath
            self.title           = self.options.title
            self.outfile         = self.options.outfile
            
            # Output option details if debugging level is high enough
            if self.debug_level >= 3 :
                print
                print "Options: ", self.options
                print "Args: ", self.args
    
        # Defining this method defines the string representation of the
        # object when given as an argument to str() or the "print" command
        #cd
        def __str__(self):
            param_print_str = \
    """Parameters:
      debug_level    : %d
      verbose_level  : %d
      filepath       : %s
      title          : %s
      outfile        : %s
    """ 
    
            str_output = param_print_str % \
                (self.debug_level, 
                 self.verbose_level,
                 self.filepath,
                 self.title,
                 self.outfile)  
            
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
            print Params
            
        # The program requires a filepath and an outfile to execute
        # correctly. If either of these are not specified, then
        # display an error and exit.
        #
        if Params.filepath is None:
            print "Error (1): Must specify a filepath is -f <filepath>"
            sys.exit(1)

        if Params.outfile is None:
            print "Error (2): Must specify an outfile -o <outfile>"
            sys.exit(2)

        create_histograms_from_pickled_file(Params.filepath,
                                            Params.outfile,
                                            Params.title)
        
    
    global Params
    Params = Params_Set()    
    main()

