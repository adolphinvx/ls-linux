
PROJECT 
------------------------------------------------------------------------------  
  ls.o - list directory contents
	as my summer project in linux programming course
	refer to: http://man7.org/linux/man-pages/
	

AUTHOR
------------------------------------------------------------------------------ 
Adolphin Huang

SYNOPSIS
------------------------------------------------------------------------------
ls [OPTION]... [FILE]...

PREPARATION
--------------------------------------------------------------------------------
"sudo apt install libbsd-dev "to install BSD library fisrt

DESCRIPTION
--------------------------------------------------------------------------------
List information about the FILEs (the current directory by default)
with one file per line.
Sort entries alphabetically if no option below is specified:
	     
	   -a, --all
              do not ignore entries starting with .

       -A, --almost-all
              do not list implied . and ..

       -c     with -lt: sort by, and show, ctime (time of last modification
              of file status information); with -l: show ctime and sort by
              name; otherwise: sort by ctime, newest first

       -d, --directory
              list directories themselves, not their contents

       -f     do not sort, enable -aU, disable -ls --color

       -F, --classify
              append indicator (one of */@|) to entries

       -g     like -l, but do not list owner

       -G, --no-group
              in a long listing, don't print group names

       -h, --human-readable
              with -l and/or -s, print human readable sizes (e.g., 1K 234M
              2G)

       -i, --inode
              print the index number of each file
			  
       -k, --kibibytes
              default to 1024-byte blocks for disk usage

       -l     use a long listing format

       -n, --numeric-uid-gid
              like -l, but list numeric user and group IDs

       -o     like -l, but do not list group information

       -p, --indicator-style=slash
              append / indicator to directories

       -Q, --quote-name
              enclose entry names in double quotes

       -r, --reverse
              reverse order while sorting

       -R, --recursive
              list subdirectories recursively

       -s, --size
              print the allocated size of each file, in blocks

       -S     sort by file size, largest first

       -t     sort by modification time, newest first


       -u     with -lt: sort by, and show, access time; with -l: show access
              time and sort by name; otherwise: sort by access time, newest
              first

       -U     do not sort; list entries in directory order
