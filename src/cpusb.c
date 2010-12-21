/**
 * \mainpage cpusb Documentation
 * An automatic synchronizer for removable media.<br/>
 *
 * <p>
 * Copyright (C) 2010 Willian Paixao <willian@ufpa.br>
 * 
 * This program is free software: you can redistribute it and/or modify<br/>
 * it under the terms of the GNU General Public License as published by<br/>
 * the Free Software Foundation, either version 3 of the License, or<br/>
 * (at your option) any later version.<br/>
 * 
 * This program is distributed in the hope that it will be useful,<br/>
 * but WITHOUT ANY WARRANTY; without even the implied warranty of<br/>
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br/>
 * GNU General Public License for more details.<br/>
 *
 * You should have received a copy of the GNU General Public License<br/>
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * </p>
 *
 * \file cpusb.c
 * \author "Willian Paixao" <willian@ufpa.br>                                                                                           
 * \date 2010-05-17
 * \version 0.001
 * \brief Main source file.
 * Contains all source and do all work.
 */

/**
 * \def CPUSBH
 * The old header file, cpusb.h.
 */
#ifndef CPUSBH
#define CPUSBH true

/**
 * \def Kb
 * Length of Kilo byte.
 */
#define Kb 1024

/* The includes of life. */
#include <confuse.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/fs.h>
#include <pwd.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

/**
 * \var char *dev_path
 * The path of device directory.
 */
char *dev_path;

/**
 * \var char *src_path
 * The path of source directory.
 */
char *src_path;

#endif

/**
 * \brief prints mensages for non-critical errors.
 * \param msg String to be printed.
 * \param err contains <code>errno</code>.
 * \see errno.h
 */
void
report (const char *msg, const int err)
{
        printf ("cpusb: %s.\ncpusb: %s.\n", msg, strerror (err));
}

/**
 * \brief Print error message and exit the program.
 * For error that can't be handled, fatal is call for prints the message and
 * close the program.
 *
 * \param msg String to be printed.
 * \param err contains <code>errno</code>.
 */
void
fatal (const char *msg, const int err)
{
        report (msg, err);
        exit (EXIT_FAILURE);
}

/**
 * \brief Change the current directory, make it if no exist.
 * Some functions needs to change the current directory,
 * <code>cwdir</code> turns it more flexible and centralized.
 * If the destination directory no exist, is created.
 *
 * \param dir_cur the base directory.
 * \param dir the target, can be created.
 * \return <code>dir</code>, the target directory.
 */
char *                                                        
cwdir (const char *dir_cur, const char *dir)
{
        char *cur, *cwd;

        cur = getcwd (NULL, 0);
        if (chdir (dir_cur))
                fatal ("Can't access the directory", errno);

        if (chdir (dir))
        {
                if (errno == EACCES)
                        fatal ("Can't access such directory", errno);
                else if (errno == ENOENT)
                {
                        if (mkdir (dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
                                fatal ("Can't access such directory", errno);
                        else
                                chdir (dir);
                }

        }
        cwd = getcwd (NULL, 0);
        /* This function don't change the current directory. */
        chdir (cur);

        /* cwd contains the path of dir. */
        return cwd;
}

/**
 * \brief Open and read the configuration file.
 * Get <code>conf_path</code>, open and read
 * the settings file, collecting user options.
 *
 * \param conf_path Path of configuration file.
 * \return Not implemented yet.
 * \see confuse.h
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
        if (cwdir (cwd, conf_path))
                chdir (conf_path);

        conf_file = fopen (".cpusb", "a");
        if (!conf_file)
                fatal ("Can't open configuration file", errno);
        else
                fclose (conf_file);

        cfg = cfg_init (opts, 0);
        cfg_parse (cfg, ".cpusb");
        cfg_free(cfg);

        if (cwdir (cwd, dev_path) == NULL)
                fatal ("Can't access the device directory", errno);
        if (cwdir (cwd, src_path) == NULL)
                fatal ("Can't access the source directory", errno);

        /**
         * \todo Implement the return stament 
         */
        return 1;
}

void
install_conf (const char *conf_path)
{
        char *cwd;
        FILE *conf_file;

        cwd = getcwd (NULL, 0); 
        cwdir (conf_path, NULL);
        chdir (conf_path);
        conf_file = fopen (".cpusb", "w+");
        if (!conf_file)
                fatal ("Can't open the configuration file", errno);

        fflush (stdin);

        dev_path = readline ("Device directory: ");
        fprintf (conf_file, "device_path = %s\n", dev_path);

        src_path = readline ("Destination directory: ");
        fprintf (conf_file, "source_path = %s\n", src_path);

        if (fclose (conf_file))
                report ("Configuration file was closed with error", errno);

        chdir (cwd);	
}

/**
 * \brief Performs the copy between the device and source.
 * Receiving a source and a destination directory, performs the copy in the 
 * direction of the device to the source. First loading the file into a dynamic 
 * buffer size, creating the destination folder, if necessary, create or 
 * truncate the target file, copy and releases the buffer.
 *
 * \param dir_dev Initial directory of origin file
 * \param dir_src Directory of copied file
 * \param file File to be copied
 **/
int
copy(const char *dir_dev, const char *dir_src, const char *file)
{
        FILE *file_dev;
        FILE *file_src;
        char *buf, *cwd;
        int fd;
        __off_t file_size;
        size_t mult, rest, cnt;
        struct stat file_meta;

        cwd = getcwd (NULL, 0);
        chdir (dir_dev);

        fd = open (file, O_RDONLY);
        file_dev = fdopen (fd, "r");
        if (fd < 0 || file_dev == NULL)
                report ("can't open origin file", errno);
        fstat (fd, &file_meta);
        file_size = file_meta.st_size;
        if (file_size <= 0)
        {
                report ("Origin file corrupted", errno);
                return -1;
        }

        if (chdir (dir_src))
                if (errno == ENOENT)
                        if (mkdir (dir_src, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
                                if (chdir (dir_src))
                                        fatal ("Can't access the source directory", errno);
        file_src = fopen (file, "w");
        if (file_src == NULL)
        {
                report ("Can't open file for copy", errno);
                return -1;
        }

        /* If the file is heavier than a Mega(buffer size),
         * the buffer will contain pointers for space dinamically allocated.*/
        if (file_size > Kb)
        {
                mult = file_size / Kb;
                rest = file_size % Kb;
                for(cnt = 0; cnt < mult;cnt++)
                {
                        buf = malloc (Kb);
                        /* setvbuf (file_src, buf, _IOFBF, Kb); */
                        fread (buf, Kb, 1, file_dev);
                        fwrite (buf, Kb, 1, file_src);
                        free (buf);
                }
                buf = malloc (rest);
                /* setvbuf (file_src, buf_index, _IOFBF, rest); */
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

        if (chmod (file, file_meta.st_mode))
                report ("Can't change the permissions of copied file", errno);                        
        if (chown (file, file_meta.st_uid, file_meta.st_gid))
                report ("Can't change the ounwer of copied file", errno);                        
        close (fd);
        fclose (file_dev);
        fclose (file_src);
        chdir (cwd);

        return 0;
}

/**
 * \brief Reads the <code>from_path</code>, for coping your contents.
 * Open <code>from_path</code>, make a search, taking each file or directory,
 * For each file or directory, try to find on other location, between device and
 * source directory. Look to your content and do copy from the newest to the oldest.
 * 
 * \param from_path Origin location, generally the device directory.
 * \param to_path Destination folder, generally the source directory.
 * \return Not implemented yet.
 */
int
read_dir (const char *from_path, char *to_path)
{
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
                                        copy (from_path, to_path, entry->d_name);
                                else
                                        copy (to_path, from_path, entry->d_name);
                        }
                        else
                                copy (from_path, to_path, entry->d_name);
                }
        }
        closedir (dir);

        /**
         * \todo Implement the return stament 
         */
        return 1;
}

int
find_file (const char *dir_path, const char *file)
{
        DIR *dir_cur;
        int ret = 0; /* Default, file not found. */
        struct dirent *entry;

        dir_cur = opendir (dir_path);
        if (dir_cur)
        {
                while ((entry = readdir (dir_cur)))
                {
                        if (!strcmp (entry->d_name, file))
                        {
                                /* I found it! */
                                ret = 1;
                                break;
                        }
                }
                closedir (dir_cur);
        }

        return ret;
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
        fd_dev = open (file, O_RDONLY | O_APPEND | O_RSYNC);
        fstat (fd_dev, &file_meta_dev);
        chdir (dir_path_src);
        fd_src = open (file, O_RDONLY | O_APPEND | O_RSYNC);
        fstat (fd_src, &file_meta_src);
        chdir (root_dir);
        if (file_meta_dev.st_mtime > file_meta_src.st_mtime)
                m_time = 1;
        else
                m_time = 0;

        return m_time;
}

/**
 * \brief Create an event and stay watching.
 * Initialize, e add a <code>inotify_event</code>.
 * Waiting <code>dev_path</code> for changes, then call 
 * <code>read_dir()</code> for start the copy. Do it recursively.
 */
void cpusb_daemon ()
{
        char buf[Kb]__attribute__((aligned(4)));
        int fd;
        ssize_t wd, len = 0, i = 0;

        fd = inotify_init ();
        if (fd == -1)
                fatal ("Can't initialize inotify", errno);

        wd = inotify_add_watch (fd, dev_path, IN_ATTRIB | IN_CLOSE_WRITE);
        if (wd == -1)
                fatal ("Can't add a watch event", errno);

        len = read (fd, buf, Kb);

        while (i < len)
        {
                struct inotify_event *event = (struct inotify_event *) &buf[i];

                if (event->mask & IN_CLOSE_WRITE || event->mask & IN_ATTRIB)
                {
                        printf ("\n\nops.. permissions changed\n");
                        /* Start the copy. */
                        chdir (dev_path);
                        read_dir (dev_path, src_path);
                        
                        cpusb_daemon ();
                }

                i += sizeof (struct inotify_event) + event->len;
        }
}

void
read_args (int argc, char *argv[])
{
        char c;
        const char *conf_path, *home;
        /* String with list of short options. */
        const char *short_options = "fi:h";
        int option_index = 0;
        struct passwd *pw;
        /* Options of arguments for cpusb. */
        static struct option long_options[]=
        {
                {"file", optional_argument, NULL, 'f'},
                {"help", no_argument, NULL, 'h'},
                {"install", optional_argument, NULL, 'i'},
                {NULL, 0, NULL, 0}
        };

                /* Gets the home of user.
                 * The home directory is used for read the configuration file.
                 */
                pw = getpwuid (getuid ());
                home = pw->pw_dir;
                

        /* If no arguments, just run. */
        if (argc == 1)
        {
                read_option (home);

                daemon (0, 0);

                /* Start the copy. */
                chdir (dev_path);
                read_dir (dev_path, src_path);

                cpusb_daemon ();

        }
        /* If there are arguments, do you job. */
        else
                /* The loop should run until end arguments. */
                while ((c = (char) getopt_long (argc, argv, short_options, 
                                                long_options, 
                                                &option_index)) != -1)
                {
                        option_index = 0;
                        switch (c)
                        {
                                case 'f':
                                        conf_path = optarg;
                                        read_option (conf_path);

                                        /* Start the copy. */
                                        chdir (dev_path);
                                        read_dir (dev_path, src_path);

                                        break;

                                case 'h':
                                        /** 
                                         * \todo Implement the help() function.
                                         */
                                        printf("--help\n");
                                        break;

                                case 'i':
                                        /** 
                                         * \todo Improve the install() function.
                                         */
                                        if (optarg)
                                                install_conf(optarg);
                                        else
                                                install_conf(home);
                                        break;

                                default:
                                        /** TODO: There should be an error handling. */
                                        break;
                        }
                }
}

/**
 * \brief The main function.
 * Just call other functions.
 *
 * \param argc Number of arguments.
 * \param argv The arguments.
 * \return EXIT_SUCCESS if completed correctly.
 */
int
main (int argc, char *argv[])
{
        read_args (argc, argv);

        return (EXIT_SUCCESS);
}
