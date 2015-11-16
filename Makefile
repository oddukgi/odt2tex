# ODT2TeX - Convert Open Document Files to LaTeX files
# Copyright (C) 2015  Simon Wilper
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

compiler = gcc
target = odt2tex
src = src
rm = rm
flags = -pipe -Wall -O2

objects = $(src)/main.o

.c.o:
	$(compiler) -c $(flags) -o "$@" "$<"

all: $(target)

$(target) : $(objects)
	$(compiler) -s -o $(target) $(objects) $(libs)
	
.PHONY : clean install

install :
	install $(target) $(HOME)/bin/$(target)

clean :
	-@$(rm) $(target) $(src)/*.o 2>/dev/null
