#!/bin/sh

# Author: Nicolas Frisby, May 2006
#
# Run with no arguments.
#
# Runs each of the versions of nachos and checks against the results
# as kept in the nachos-src/regressions_expected/ directory.
#
#
# Note:
#
# Find a clean way to use the ADHOC mechanism of the make system
# instead of generating tarballs, untarring them, and finally deleting
# them. E.g.
#        make "ADHOC=-D_SYNCH -D_ECHO"
# would work for pa2-1-assignment without any use of tarballs.
#
# This is an option that might provide efficiency worth it eventually,
# but for now it's a non-issue. In fact, going all the way to tarballs
# is a good thing in terms of regression.


if test -x make-assignment.sh
then
	echo "running regression test"
else
	chmod +x make-assignment.sh
fi

# make the tarballs (these tag lists need to match those in the ASSIGNMENTS file)
./make-assignment.sh pa2-1-assignment.tar SYNCH ECHO DSUI
./make-assignment.sh pa2-1-solution.tar SYNCH ECHO PRIORITY DSUI

./make-assignment.sh pa2-2-assignment.tar PRIORITY SYNCH ECHO THREAD_IP METRICS DSUI
./make-assignment.sh pa2-2-solution.tar PRIORITY SYNCH ECHO THREAD_IP METRICS DYNAMICP DSUI

./make-assignment.sh pa3-assignment.tar PRIORITY SYNCH ECHO REFRESH_WSS DSUI
./make-assignment.sh pa3-solution.tar PRIORITY SYNCH ECHO REFRESH_WSS PRP DSUI



# function for running a test
function dotest {
    # indicate progress
    echo ===== $1 =====

    # untar the file, build nachos, make regress.all, and record
    # differences between actual and expected output
    tar -zxf $1.tar.gz && ((cd nachos && make -s clean && make -s && make -s regress.all) &> $1.make.output && \rm $1.make.output) && \
	mv ./nachos/regress.all $1.regress.all && (diff $1.regress.all regression_expected/$1.all > $1.diff)

    # if there was a difference, spout a message
    # it not, delete the empty diff file and useless regress.all file
    # since its just a copy of the expected
    (test -s $1.diff && echo "REGRESSION FAILED; see $1.diff") || (\rm $1.diff $1.regress.all)

    # remove the nachos directory that was created when we untarred the tarball
    \rm -r nachos/
}



# run tests
dotest 'pa2-1-assignment'
dotest 'pa2-1-solution'

dotest 'pa2-2-assignment'
dotest 'pa2-2-solution'

dotest 'pa3-assignment'
dotest 'pa3-solution'


# remove the tarballs
\rm pa*.tar.gz
