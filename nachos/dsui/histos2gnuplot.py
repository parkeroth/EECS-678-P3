#!/bin/env python

# This program generates a GNUPlot script for displaying the
# histograms found in the working directory.
#
# usage: % ./<this file> <destination file> <title> <instrumentation sample period> <histogram files>
#
# will generate the gnuplot commands. Pipe them into gnuplot to
# actually create the file:
#
# usage: % ./<this file> ... | gnuplot
#

# % python histos2gnuplot.py histos.pdf 'A, B, and C' 10 A.histo B.histo C.histo
#
# ... will generate histos.pdf. The histogram spec should match the
# spec that was used to generate the histograms

# By: Nicolas Frisby. Mar 2006

terminalType = 'postscript'

def parseArgs():
    import sys
    import string

    if 4 > len(sys.argv):
        raise RuntimeError, 'usage: python %s <destination file> <title> <instrumentation sample period> <histogram files>' % sys.argv[0]
    
    fname = sys.argv[1]

    title = sys.argv[2]

#    ipp = int( sys.argv[3] )
#    hs = sys.argv[4:]
    ipp = 1
    hs = sys.argv[3:]

    hs.sort()

    return (fname, title, ipp, hs)



def gpHeader( fname , title ):
    print """
    set terminal %s landscape enhanced color
    set output "%s"

    set title "%s"

    set xtics nomirror

    set xlabel "Episode Duration (each bucket covers a range of durations in machine ticks)"
    set ylabel "Bucket Hit Count"
    """ % (terminalType, fname, title)



def gpPlots( hs , ipp , coln ):
    import string

    def getCol( coln , line ):
        try:
            field = (string.split(line))[coln]
            if field.isdigit():
                return int( field )
            else:
                return None
        except IndexError, e:
            return None

    # To place multiple bars (one from each histogram) in the same
    # bucket (i.e. near the same x-coordinate), we need to expand the
    # granularity of the x-axis to gain further precision. We us a
    # scalar to do so and then shift each histogram by a different
    # offset.
    scale = len( hs ) * 2


    # find the maximum value of the column and check that all
    # histogram specs are the same
    hspec = []
    maxv = 0
    for h in hs:
        f = open( h , 'r' )

        for line in f:
            v = getCol( coln + 1 , line )
            if None == v:
                if "#HSPEC:" == line[0:7]:
                    this_hspec = map(int, string.split(line[7:], ','))
                    if [] == hspec:
                        hspec[:] = this_hspec[:]
                    else:
                        if not (this_hspec[:] == hspec[:]):
                            raise RuntimeError, 'All histogram specifications must be the same; %s was different' % h
            else:
                maxv = max(maxv, v)
    (n,w,minv) = (hspec[0], hspec[1], hspec[2])


    # set the ranges
    print 'set xrange [ %d : %d ]' % ((-0.5*scale), (n+2)*scale) # -1 and +2 give the underflow/overflow bucket room on the graph
    print 'set yrange [ 0 : %d ]' % (maxv - (maxv % 5) + 5)

    # calculate label names
    labels = ['"under" 0']
    for x in range(1, n+1):
        base = (minv + w*(x-1)) * ipp
        labels.append( '"(%d,%d]" %d' % (base, base+w*ipp, x*scale) )
    labels.append( '"over" %d' % ((n+1)*scale) )

    print 'set xtics (%s) font "Helvetica, 2"' % string.join( labels, ', ' )

    # set the grid
    print 'set x2tics %d,%d,%d font "Helvetica, 1"' % ((-0.5)*scale, scale, (n+1.5)*scale)
    print 'set grid x2tics ytics'

    # plot the histograms
    plots = []
    c = 0
    shift = -1 * len(hs) + 1

    for h in hs:
        plots.append( '"%s" using ($1*%d+(%d)):2 with imp lw 5 lt %d title "%s"' % (h, scale, int(shift), c, h) )
        c = c + 1
        shift = shift + 2

    print 'plot %s' % string.join( plots , ', ' )


if __name__ == "__main__":
    import os
    import fnmatch

    (fname, title, ipp, hs) = parseArgs()

    gpHeader( fname , title )
    
    gpPlots( hs , ipp , 0 )
