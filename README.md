# ODT to TeX converter

V0.0.11 - 2016-07-16
Simon Wilper

Plain C program that takes a ODT file and an output directory to write
the tex file to.

This program is in early development stage. For changes see CHANGES.MD

Dependencies are:

- **libzip** for reading the contents.xml from the zipped ODT file
- **expat** for running the XML parse process

The build system used is plain make as it is still available on most
platforms and I don't want to talk people into installing a fancy one
just to get turned this couple of source files turned into a binary.

To get the binary `odt2tex` in the same directory as the Makefile type

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

