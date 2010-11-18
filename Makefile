#
# \author "Willian Paixao" <willian@ufpa.br>
# \date 2010-05-17
# \file Makefile
# \version 0.0001
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

all: build build-doc
	@echo 'finished.'

build:
	mkdir build/
	gcc src/cpusb.c -o build/cpusb -lconfuse -g -pass-exit-codes

build-doc:
	cd src/
	doxygen src/doxyfile
	cd ../

clean-build:
	rm -rf build/
	@echo 'build clean.'

clean-doc:
	rm -rf doc/
	@echo 'documentation clean.'

clean: clean-build clean-doc
	@echo 'All cleaned.'
