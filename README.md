# ODT to TeX converter

V0.0.8 - 2016-02-01
Simon Wilper

Plain C program that takes a ODT file and an output directory to write
the tex file to.

This program is in early development stage.

Dependencies are:

- **libzip** for reading the contents.xml from the zipped ODT file
- **expat** for running the XML parse process

Build system used is tup or make.

With tup:

```
> tup init
> tup
```

With make:

```
> make
```

then copy the binary odt2tex somewhere that is also in your PATH.

## Usage

As a minimum you can run odt2tex with an input odt file an an out
parameter pointing to a directory:

```
odt2tex in=testdoc.odt out=/home/jerry/documents/out
```

Other optional paremeters to tweak the output are:

* enc: TeX Encoding (defaults to UTF-8)
* lang: Language for Babel Hyphenation (defaults to en)
* captionoffset: Number of characters to strip from caption (0-128,
  default 2) This is due to as default the characters ": " are inserted
between the object type and the description like "Figue 2: Description"
* tablecolwidth: Table column Width (0-200, default 30)

