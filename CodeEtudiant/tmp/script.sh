#!/bin/sh
for file in `find . -name "*.dot"`
do
	dot -Tpdf $file -o $file.pdf
done
