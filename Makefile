compiler = gcc
target = odt2tex
incdirs = -Iinc
src = src
rm = rm
inc = inc
flags = -pipe -Wall -O2
libs = -lexpat -lzip

# Objects for Main Binary
objects =\
  $(src)/list.o\
  $(src)/main.o\
  $(src)/map.o\
  $(src)/parser.o

.c.o:
	$(compiler) -c $(incdirs) $(flags) -o "$@" "$<"

all: $(target)

$(target) : $(objects)
	$(compiler) -s -Wl,-O2 -o $(target) $(objects) $(libs)
	
.PHONY : clean install test

clean :
	-@$(rm) $(target) $(src)/*.o 2>/dev/null

install:
	install -D $(target) $(DESTDIR)

test:
	./odt2tex in=testdoc.odt out=testdoc-out
