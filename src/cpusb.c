/**
 * \mainpage
 * An automatic synchronizer for removable media.
 * Copyright (C) 2010 Willian Paixao <willian@ufpa.br>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file cpusb.c
 * \author "Willian Paixao" <willian@ufpa.br>
 * \date 2010-05-17
 * \version 0.0001
 * \brief Yet do all.
 */

/**
 * \headerfile cpusb.h
 * \brief Contains global variables and necessary libraries.
 */
#include "cpusb.h"

/**
 * \brief Print error messages, like warnings.
 * \details
 * Prints simple messages, but don't exit the program.
 *
 * \var msg Message to be printed
 */
void
report (const char *msg, const int err)
{
        printf ("cpusb: %s.\ncpusb: %s.\n", msg, strerror (err));
}

/**
 * \brief Print error message and exit the program.
 * \details
 * For error that can't be handled, fatal is call for prints the message abd 
 * close the program.
 *
 * \var msg Message to be printed
 */
void
fatal (const char *msg, const int err)
{
        report (msg, err);
        exit (EXIT_FAILURE);
}

/**
 * \brief Change the current directory, make it if no exist.
 * \details
 * Some functions needs to change the current directory,
 * <code>cwdir</code> turns it more flexible and centralied.
 * If the destination directory no exist, is created.
 * \param dir_cur the base directory
 * \param dir the target, can be created
 * \return <code>dir</code>, the target directory
 */
char *                                                        
cwdir (const char *dir_cur, const char *dir)
{
        char *cur, *cwd;

        cur = getcwd (NULL, 0);
        chdir (dir_cur);

        if (chdir (dir))
        {
                if (errno == EACCES)
                        fatal ("Can't access the directory", errno);
                else if (errno == ENOENT)
                {
                        if (mkdir (dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
                                fatal ("Can't access the directory of configuration file", errno);
                        else
                                chdir (dir);
                }

        }
        cwd = getcwd (NULL, 0);
        chdir (cur);

        return cwd;
}

/**
 * \brief Open and read the configuration file.
 * \details
 * Get <code>conf_path</code>, open and read
 * the settings file, collecting user options.
 *
 * \param conf_path Path of configuration file
 * \return Not implemented yet
 */
int
read_option (const char *conf_path)
{
        char *cwd;
        FILE *conf_file;
        cfg_t *cfg;
        cfg_opt_t opts[] = {
                CFG_SIMPLE_STR ("device_path", &dev_path),
                CFG_SIMPLE_STR ("source_path", &src_path),
                CFG_END()
        };

        cwd = getcwd (NULL, 0);
        if (chdir (conf_path))
        {
                if (errno == EACCES)
                        fatal ("Can't access the directory of configuration file", errno);
                else if (errno == ENOENT)
                {
                        if (mkdir (conf_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
                        {
                                if (errno == EACCES)
                                        fatal ("Can't access the directory of configuration file", errno);
                        }
                        else
                                chdir (conf_path);
                }

        }
        conf_file = fopen (".conf", "a");
        if (!conf_file)
                fatal ("Can't open configuration file", errno);
        else
                fclose (conf_file);
        cfg = cfg_init (opts, 0);
        cfg_parse (cfg, ".conf");
        cfg_free(cfg);
        if (chdir (dev_path))
                if (errno == ENOENT)
                        if (mkdir (dev_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
                                fatal ("Can't access the device directory", errno);
        if (chdir (src_path))
                if (errno == ENOENT)
                        if (mkdir (src_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
                                fatal ("Can't access the source directory", errno);
        chdir (cwd);
}

/**
 * \brief
 * \detals
 * \param conf_path
 * \return 
 */
int
install_conf (const char *conf_path)
{
        char *cwd;

        cwd = getcwd (NULL, 0); 
        cwdir (conf_path, NULL);
        conf_file = fopen (".conf", "w+");
        if (!conf_file)
                fatal ("Can't open the configuration file", errno);

        printf ("Device directory: ");
        scanf ("%s", &dev_path);
        fprintf (conf_file, "device_path = %s\n", dev_path);

        printf ("Destination directory: ");
        scanf ("%s", &src_path);
        fprintf (conf_file, "source_path = %s\n", src_path);

        chdir (cwd);	

        if (!fclose (conf_file))
                report ("Configuration file was closed with error", errno);
}

int
read_dir (const char *from_path, char *to_path)
{
        char srcpath[PATH_MAX], *cwd;
        DIR *dir = NULL;
        struct dirent *entry;

        dir = opendir (from_path);
        while ((entry = readdir (dir)) != NULL)
        {
                if (entry->d_type == DT_DIR)
                {
                        if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
                                continue;
                        else
                        {
                                chdir (entry->d_name);
                                read_dir (getcwd (NULL, 0), cwdir (to_path, entry->d_name));
                                chdir ("..");
                        }
                }
                else if (entry->d_type == DT_REG)
                {
                        if (find_file (to_path, entry->d_name))
                        {
                                if (cmp_stat (from_path, to_path, entry->d_name))
                                        do_copy (from_path, to_path, entry->d_name);
                                else
                                        do_copy (to_path, from_path, entry->d_name);
                        }
                        else
                                do_copy (from_path, to_path, entry->d_name);
                }
        }
        closedir (dir);
}

        int
find_file (const char *dir_path, const char *file)
{
        DIR *dir_cur;
        int ret = 0;
        struct dirent *entry;

        if (dir_cur = opendir (dir_path))
        {
                while (entry = readdir (dir_cur))
                {
                        if (!strcmp (entry->d_name, file))
                        {
                                ret = 1;
                                break;
                        }
                }
                closedir (dir_cur);
                return ret;
        }
}

/**
 * \brief Compare <code>m_time</code> of two files.
 * \details
 * The <code>m_time in struct stat</code> defined in <code>bits/stat.h</code> 
 * included in <code>sys/stat.h</code>. The field provides the last modification time. 
 * The function returns true if the file has been modified <code>dir_path_dev</code> 
 * finally, false otherwise.
 *
 * \param dir_path_dev A folder on <i>device</i>
 * \param dir_path_src A folder on <i>source</i>
 * \param file Filename. Notice that only one name is passed, because it must be 
 * the same file in different folders.
 * \return m_time True if file <code>dir_path_dev</code> is the newest, false otherwise.
 */
        int
cmp_stat (const char *dir_path_dev, const char *dir_path_src, const char *file)
{
        char *root_dir;
        int fd_dev, fd_src, m_time;
        struct stat file_meta_dev, file_meta_src;

        root_dir = getcwd (NULL, 0);
        chdir (dir_path_dev);
        fd_dev = open (file, O_RDWR | O_APPEND | O_RSYNC);
        fstat (fd_dev, &file_meta_dev);
        chdir (dir_path_src);
        fd_src = open (file, O_RDWR | O_APPEND | O_RSYNC);
        fstat (fd_src, &file_meta_src);
        chdir (root_dir);
        if (file_meta_dev.st_mtime > file_meta_src.st_mtime)
                m_time = 1;
        else
                m_time = 0;

        return m_time;
}

/**
 * \brief Performs the copy between the device and source.
 * \details
 * Receiving a source and a destination directory, performs the copy in the 
 * direction of the device to the source. First loading the file into a dynamic 
 * buffer size, creating the destination folder, if necessary, create or 
 * truncate the target file, copy and releases the buffer.
 *
 * \param dir_path_dev Directory path origin
 * \param dir_path_src Directory path end
 * \param origin_file File to be copied
 **/
int
do_copy(const char *dir_path_dev, const char *dir_path_src, const char *origin_file)
{
        FILE *file_dev;
        FILE *file_src;
        char *buf;
        char *cwd;
        int fd, ok;
        __off_t file_size;
        struct stat file_meta;
        size_t mult, rest, cnt;
        cwd = getcwd (NULL, 0);
        chdir (dir_path_dev);
        fd = open (origin_file, O_RDONLY);
        file_dev = fdopen (fd, "r");
        fstat (fd, &file_meta);
        file_size = file_meta.st_size;

        if (chdir (dir_path_src))
                if (errno == ENOENT)
                        if (mkdir (dir_path_src, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
                                if (chdir (dir_path_src))
                                        fatal ("Can't access the source directory", errno);
        file_src = fopen (origin_file, "w");

        /* If the file is heavier than a Mega(buffer size),
         * the buffer will contain pointers for space dinamically allocated.*/
        if (file_size > Kb)
        {
                mult = file_size / Kb;
                rest = file_size % Kb;
                for(cnt = 0; cnt < mult;cnt++)
                {
                        buf = malloc (Kb);
                        //setvbuf (file_src, buf, _IOFBF, Kb);
                        fread (buf, Kb, 1, file_dev);
                        fwrite (buf, Kb, 1, file_src);
                        free (buf);
                }
                buf = malloc (rest);
                //setvbuf (file_src, buf_index, _IOFBF, rest);
                fread (buf, rest, 1, file_dev);
                fwrite (buf, rest, 1, file_src);
                free (buf);
        }
        else
        {
                buf = malloc (file_size);
                fread (buf, file_size, 1, file_dev);
                fwrite (buf, file_size, 1, file_src);
                free (buf);
        }

        close (fd);
        fclose (file_dev);
        fclose (file_src);
        chdir (cwd);
}

/**
 * \brief The main function.
 * \details
 * The <code>main</code> function checks the arguments passed to cpusb as options and 
 * modes of synchronization. After collecting and processing arguments, other 
 * functions will be in charge of doing what was asked.
 *
 * \param argc Number of arguments
 * \param argv The arguments
 * \return EXIT_SUCCESS if completed correctly
 */
        int
main (int argc, char **argv)
{
        char c;
        // Folder containing the configuration file. Should be modified before compiling.
        const char *conf_path = "/home/willian/devel/cpusb/src";
        // String with list of short options.
        const char *short_options = "i:Bh";
        int ok;
        int option_index = 0;
        // Options of arguments for cpusb.
        static struct option long_options[]=
        {
                {"background", no_argument, NULL, 'B'},
                {"help", no_argument, NULL, 'h'},
                {"install", optional_argument, NULL, 'i'},
                {NULL, 0, NULL, 0}
        };

        // If no arguments, just run.
        if (argc == 1)
        {
                // Reads the options of configuration file.
                read_option (conf_path);

                // Starts copying.
                chdir (dev_path);
                read_dir (dev_path, src_path);
        }
        // If there are arguments, do you job.
        else
                // The loop should run until end arguments.
                while ((c = (char) getopt_long (argc, argv, short_options, long_options, &option_index)) != -1)
                {
                        option_index = 0;
                        switch (c)
                        {
                                case 'B':
                                        // TODO: All background.
                                        printf("Running in background.\n");
                                        break;

                                case 'h':
                                        // TODO: All help.
                                        printf("--help\n");
                                        break;

                                case 'i':
                                        // TODO: It is poor. Improve it.
                                        if (optarg)
                                                install_conf(optarg);
                                        else
                                                install_conf(conf_path);
                                        break;

                                default:
                                        // TODO: There should be an error handling.
                                        break;
                        }
                }
        return (EXIT_SUCCESS);
}
