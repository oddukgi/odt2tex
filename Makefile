compiler = gcc
target = odt2tex
incdirs = -Iinc
src = src
rm = rm
inc = inc
optimize = -O0
flags = -pipe -g -ggdb -Wall $(optimize)
libs = -lexpat -lzip
#strip = -s
strip =

# Objects for Main Binary
objects =\
  $(src)/list.o\
  $(src)/main.o\
  $(src)/map.o\
  $(src)/parser.o\
  $(src)/util.o

.c.o:
	$(compiler) -c $(incdirs) $(flags) -o "$@" "$<"

all: $(target)

$(target) : $(objects)
	$(compiler) $(strip) -Wl,$(optimize) -o $(target) $(objects) $(libs)
	
.PHONY : clean install test

clean :
	-@$(rm) $(target) $(src)/*.o 2>/dev/null

install:
	install -D $(target) $(DESTDIR)

test:
	./odt2tex in=testdoc.odt out=testdoc-out
