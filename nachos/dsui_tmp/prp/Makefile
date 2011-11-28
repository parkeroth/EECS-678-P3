
.PHONY: histos tests prp_postprocess table

prp_tests:
	-mkdir data
	-mkdir bins
	-mkdir histos
	-mkdir swaps
	-mkdir pdfs
	-mkdir configs
	rm -f bins/*
	rm -f swaps/*
	python prp_tests.py

prp_postprocess:
	#make -f Makefile.prp
	-rm -rf prp.pipes.out
	-rm -rf data/*
	-postprocess f prp.pipes


histos:
	bash prp_histos histos

table:
	python prp_table.py

clean:
	rm -rf data histos bins configs pdfs swaps results
