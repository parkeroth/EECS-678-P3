
.PHONY: histos tests prp_postprocess table

tests:
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
	make -f Makefile.prp

histos:
	bash prp_histos histos

table:
	python prp_table.py
