ODT to TeX converter

V0.0.1 - 2015-11-22
Simon Wilper

Plain C program that takes a ODT file and an output directory to write
the tex file to.

This program is in early development stage.

Dependencies are:

- ''libzip'' for reading the contents.xml from the zipped ODT file
- ''expat'' for running the XML parse process

Build system used is tup. After clone run

> tup init
> tup
