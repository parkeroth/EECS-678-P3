#!/bin/sh

echo "Making PDFs for: $5"

# if there are histograms delete them
ls *histo &> /dev/null && \rm *histo

../userprog/nachos -x nice_free -d metrics -i $1 -R $2 -H "$3" -I $4 > temp.out
make -sf Makefile.histos

python histos2gnuplot.py $5-inactive.pdf "$6 INACTIVITY" $4 *.last.inactive.histo | gnuplot
python histos2gnuplot.py $5-active.pdf "$6 ACTIVITY" $4 *.last.active.histo | gnuplot


# if the output directory already exists, remove it first
test -d $5.data && \rm -r $5.data
mkdir $5.data && mv *histo $5.data && mv temp.out $5.data

../userprog/nachos -x nice_free -d metrics:reset_histos -i $1 -R $2 -H "$3" -I $4 > /dev/null
make -sf Makefile.histos

python histos2gnuplot.py $5-dinactive.pdf "$6 INACTIVITY of D" $4 D\@*.inactive.histo | gnuplot
python histos2gnuplot.py $5-dactive.pdf "$6 ACTIVITY of D" $4 D\@*.active.histo | gnuplot

# clean up the last histograms
\rm *histo
