##
# \author "Willian Paixao" <willian@ufpa.br>
# \date 2010-05-17
# \file Makefile
# \version 0.001
# \brief Yet do all.
#
# An automatic synchronizer for removable media. 
# Copyright (C) 2010 Willian Paixao <willian@ufpa.br>
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
#

CC      = gcc
FLAGS   = -ggdb -Wall -pass-exit-codes
LIB     = -lconfuse

# INFILE is the source code to be compiled.
INFILE  = src/cpusb.c

# OUTFILE is the path and name of binary
OUTFILE = build/cpusb

all: build build-doc
	@echo 'Finished.'

build:
	mkdir build/
	$(CC) $(FLAGS) $(LIB) -o $(OUTFILE) $(INFILE)

build-doc:
	mkdir -p doc/devel/
	doxygen doxyfile

rebuild: clean build
	@echo 'Done.'

clean: clean-build clean-doc
	@echo 'All cleaned.'

clean-build:
	rm -rf build/
	@echo 'Build clean.'

clean-doc:
	rm -rf doc/
	@echo 'Documentation clean.'

