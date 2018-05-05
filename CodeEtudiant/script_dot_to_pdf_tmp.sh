#!/bin/sh
for file in `find ./tmp/ -name "*.dot"`
do
	dot -Tpdf $file -o $file.pdf
done
