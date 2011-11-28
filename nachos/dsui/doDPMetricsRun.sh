#!/bin/sh

echo "Making PDFs for: $3"

# if there are histograms delete them
ls *histo &> /dev/null && \rm *histo

../userprog/nachos -x ../test/nice_free --dsui-config nachos.dsui -i $1 -R $2 > temp.out
#make -sf Makefile.histos
make histo_postprocess

python histos2gnuplot.py $3-inactive.ps "$4 INACTIVITY" [ABCD].inactive.histo | gnuplot
python histos2gnuplot.py $3-active.ps "$4 ACTIVITY" [ABCD].active.histo | gnuplot
ps2pdf $3-inactive.ps
ps2pdf $3-active.ps

# if the output directory already exists, remove it first
test -d $3.data && \rm -r $3.data
mkdir $3.data && mv *histo $3.data && mv temp.out $3.data

../userprog/nachos -x ../test/nice_free --dsui-config nachos.dsui -i $1 -R $2 > /dev/null
#make -sf Makefile.histos
make histo_postprocess

python histos2gnuplot.py $3-dinactive.ps "$4 INACTIVITY of D" D\@*.inactive.histo | gnuplot
python histos2gnuplot.py $3-dactive.ps "$4 ACTIVITY of D" D\@*.active.histo | gnuplot
ps2pdf $3-inactive.ps
ps2pdf $3-active.ps

# clean up the last histograms
\rm *histo
