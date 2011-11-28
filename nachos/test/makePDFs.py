import sys
import os

# This script generates a number of histogram graphs.
d = {}



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
d[(10, 0.0, 1)] = ((20, 10, 1), (10, 10, 1))
d[(10, 0.25, 1)] = ((20, 10, 1), (10, 10, 1))
d[(10, 0.5, 1)] = ((20, 10, 1), (10, 10, 1))
d[(10, 0.75, 1)] = ((20, 10, 1), (10, 10, 1))
d[(10, 1.0, 1)] = ((20, 10, 1), (10, 10, 1))


# These 2 graphs are for the inertia extra credit
d[(10, 0.0, 2)] = ((20, 15, 1), (10, 20, 1))
d[(10, 0.0, 4)] = ((20, 20, 1), (10, 20, 1))
d[(10, 0.0, 8)] = ((20, 30, 1), (10, 20, 1))


# These 4 graphs are for the profiling period extra credit
d[(1, 0.0, 1)] = ((20, 100, 1), (10, 100, 1))
d[(50, 0.0, 1)] = ((20, 2, 1), (10, 2, 1))
d[(100, 0.0, 1)] = ((20, 1, 1), (10, 1, 1))
d[(200, 0.0, 1)] = ((10, 1, 1), (5, 1, 1))



def main( prefix ):
    for ((I, R, i), ((n1, w1, m1), (n2, w2, m2))) in d.items():
        # os.system( "source makePDFs.sh i R H I Filename Title" )
        fname = os.path.join( prefix , 'I%d-R%.2f-i%d' % (I, R, i) )
        
        os.system( "source doDPMetricsRun.sh %d %f '%d,%d,%d;%d,%d,%d' %d %s 'I=%d, R=%.2f, i=%d Durations'" % (i, R, n1, w1, m1, n2, w2, m2, I, fname, I, R, i ) )

if __name__ == "__main__":
    main( sys.argv[1] )
