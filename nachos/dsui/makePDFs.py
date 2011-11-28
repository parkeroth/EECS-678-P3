import sys
import os

# This script generates a number of histogram graphs.
g = []



# FORMAT:
#
# I is the instrumentation sampling period
# R is the dynamic priority advantage reduction factor
# i is the number of inertial quanta
#
# n, width, and min are histoggram specifications
#
# d[(I, R, i) = ((n1,width1,min1), (n2,width2,min2))

# These 5 graphs are for the main project
g.append((0.0, 1))
g.append((0.25, 1))
g.append((0.5, 1))
g.append((0.75, 1))
g.append((1.0, 1))

# These 2 graphs are for the inertia extra credit
g.append((0.0, 2))
g.append((0.0, 4))
g.append((0.0, 8))

# These 4 graphs are for the profiling period extra credit
# n/a now

def main( prefix ):
	for (R, i) in g:
		fname = os.path.join(prefix, 'R%.2f-i%d' % (R, i))
		os.system("source ./doDPMetricsRun.sh %d %f %s 'R=%.2f, i=%d Durations'" % (i, R, fname, R, i))
#    for ((I, R, i), ((n1, w1, m1), (n2, w2, m2))) in d.items():
#        # os.system( "source makePDFs.sh i R H I Filename Title" )
#        fname = os.path.join( prefix , 'I%d-R%.2f-i%d' % (I, R, i) )        
#
#        os.system( "source doDPMetricsRun.sh %d %f '%d,%d,%d;%d,%d,%d' %d %s 'I=%d, R=%.2f, i=%d Durations'" % (i, R, n1, w1, m1, n2, w2, m2, I, fname, I, R, i ) )

if __name__ == "__main__":
    main( sys.argv[1] )
