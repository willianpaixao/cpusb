/**
 * \author "Willian Paixao" <willian@ufpa.br>
 * \date 2010-05-17
 * \file cpusb.h
 * \version 0.0001
 * \brief Contains global variables and necessary libraries.
 * 
 * <p>An automatic synchronizer for removable media. 
 * Copyright (C) 2010 Willian Paixao <willian@ufpa.br></p>
 * 
 * <p>This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.</p>
 * 
 * <p>This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.</p>
 *
 * <p>You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.</p>
 */


#ifndef CPUSB_H_
#define CPUSB_H_

// The includes of life.
#include <confuse.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Length of Kilo byte.
#define Kb 1024

// The configuration file.
FILE *conf_file;

// The path of directories of device and source.
char *dev_path;
char *src_path;

//Funtions signatures

#endif /* CPUSB_H_ */
