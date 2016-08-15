#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <bsd/string.h>
#include <ctype.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fts.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <pwd.h>
#include <grp.h>


static void display(FTSENT*, FTSENT*);
int sort(const FTSENT**, const FTSENT**);
int digitlen(int num);
double logk(double x);


/*display flags*/
int f_all = 0;			/*list all files*/
int f_ctime = 0;	/*time mode*/
int f_atime = 0;
int f_mtime = 1;
int f_sort = 1;		/*sort the order*/
int f_order = 1;		/*the order of sort if -1 reverse*/
int f_type = 0;		/*display sign for different file type*/
int f_dtype = 0;	/*display sign for directory file type*/
int f_inode = 0;		/*display the index number of each file*/
int f_human = 0;	/*display human readable sizes*/
int f_long = 0;		/*list  long format*/
int f_group=1;		/*list group in long format*/
int f_owner=1;		/*list owner in long format*/
int f_rec = 0;		/*recursively list subdirectories*/
int f_id = 0;			/*list owner&group in id format*/
int f_sizeorder = 0;/*sort by size*/
int f_timeorder = 0;/*sort by time*/
int f_size = 0;		/*display number of file system blocks*/
int f_listdir = 0;		/*list directory itself*/
int f_quote = 0;	/*dispaly double quotes*/

long blocksize ;

/*
  *the funtion is used to diaplay the entires
 */
static void display(FTSENT* parent_p, FTSENT* child_p)
{
	/*max* are used to control display format*/
	FTSENT *tmp;
	int block = 0;
	int maxblock = 0;
	int sumblock = 0;
	int maxblocklen = 0;
	int maxlinks = 0;
	int maxlinkslen = 0;
	int unamelen = 0;
	int maxunamelen = 0;
	int gnamelen = 0;
	int maxgnamelen = 0;
	uintmax_t sizelen = 0;
	uintmax_t maxsizelen = 0;
	int maxpathlen = 0;
	int maxinode = 0;
	int maxinodelen = 0;
	time_t t;
	time_t curtime;
	double maxunit;
	uintmax_t sumsize;
	int i;
	
	char unit_char[] = " KMGTPEZY";

	
	if (parent_p != NULL)
	{/*read information first*/

		if (f_inode){	/*read inode*/
			for (tmp = child_p; tmp != NULL; tmp = tmp->fts_link){
				if ((tmp->fts_statp->st_ino > maxinode))
					maxinode = tmp->fts_statp->st_ino;
			}
		}

		if (f_size){	/*read size*/
			for (tmp = child_p; tmp != NULL; tmp = tmp->fts_link){
				if ((tmp->fts_statp->st_blocks * 512 + blocksize - 1) / blocksize > maxblock)
					maxblock = (tmp->fts_statp->st_blocks * 512 + blocksize - 1) / blocksize;
				sumblock += (tmp->fts_statp->st_blocks * 512 + blocksize - 1) / blocksize;
			}
		}

		if (f_long){
			for (tmp = child_p; tmp != NULL; tmp = tmp->fts_link){/*read links*/
				if (!f_size){
					sumblock += (tmp->fts_statp->st_blocks * 512 + blocksize - 1) / blocksize;
				}

				if (tmp->fts_statp->st_nlink > maxlinks)
					maxlinks = tmp->fts_statp->st_nlink;

				if ((getpwuid(tmp->fts_statp->st_uid) != NULL) && !f_id){/*read uname or uid*/
					if ((unamelen = strlen(getpwuid(tmp->fts_statp->st_uid)->pw_name)) > maxunamelen)
						maxunamelen = unamelen;
				}
				else{
					if ((unamelen = digitlen(tmp->fts_statp->st_uid)) > maxunamelen)
						maxunamelen = unamelen;
				}

				if ((getgrgid(tmp->fts_statp->st_gid) != NULL) && !f_id){/*read gname or gid*/
					if ((gnamelen = strlen(getgrgid(tmp->fts_statp->st_gid)->gr_name)) > maxgnamelen)
						maxgnamelen = gnamelen;
				}
				else{
					if ((gnamelen = digitlen(tmp->fts_statp->st_gid)) > maxgnamelen)
						maxgnamelen = gnamelen;
				}
				
				if (S_ISCHR(tmp->fts_statp->st_mode) || S_ISBLK(tmp->fts_statp->st_mode)){
					sizelen = digitlen(major(tmp->fts_statp->st_rdev)) +
						digitlen(minor(tmp->fts_statp->st_rdev)) + 1;
				}
				else{
					sizelen = digitlen(tmp->fts_statp->st_size);
				}
				if (sizelen > maxsizelen)
					maxsizelen = sizelen;

				if (tmp->fts_pathlen > maxpathlen)
					maxpathlen = (int)tmp->fts_pathlen;

			}
		}

		maxinodelen = digitlen(maxinode);
		maxblocklen = digitlen(maxblock);
		maxlinkslen = digitlen(maxlinks);
		
		sumsize = sumblock*blocksize;

		if (sumsize <= 0)
			maxunit = 0;
		else
			maxunit = floor(logk(sumsize));

		if (f_rec)
			printf("%s:\n", parent_p->fts_path);

		if (f_size || f_long){
			if (!f_human)
				printf("total %i\n", (int)sumblock);
			else{
				if ((sumsize / pow(1024, maxunit)) > 10)
					printf("total %i%c\n", (int)(sumsize / pow(1024, maxunit)), unit_char[(int)maxunit]);
				else
					printf("total %1.1f%c\n", (float)(sumsize / pow(1024, maxunit)), unit_char[(int)maxunit]);

			}
		}

		//
	}

	while (child_p != NULL)
	{/*output the information*/
		if (child_p->fts_name[0] != '.' || f_all){

			if (f_inode){
				printf("%*i ", (int)maxinodelen, (int)child_p->fts_statp->st_ino);
			}


			if (f_size){
				block = (uintmax_t)(child_p->fts_statp->st_blocks * 512 + blocksize - 1) / blocksize;
				if (!f_human)
					printf("%*i ", (int)maxblocklen, (int)block);
				else{
					sumsize = block*blocksize;
					if (sumsize <= 0)
						maxunit = 0;
					else
						maxunit = floor(logk(sumsize));
					if ((int)maxunit == 0){
							printf("%5i ", (int)(sumsize / pow(1024, maxunit)));
					}
					else{
						if ((sumsize / pow(1024, maxunit)) > 10)
							printf("%4i%c ", (int)(sumsize / pow(1024, maxunit)), unit_char[(int)maxunit]);
						else
							printf(" %1.1f%c ", (float)(sumsize / pow(1024, maxunit)), unit_char[(int)maxunit]);
					}

				}
			}
			if (f_long){
				char bp[100];
				strmode(child_p->fts_statp->st_mode, bp);
				printf("%s ", bp);
				printf("%*i ", maxlinkslen, (int)child_p->fts_statp->st_nlink);

				if(f_owner){
					if ((getpwuid(child_p->fts_statp->st_uid) != NULL) && !f_id)
						printf("%-*s ", maxunamelen, getpwuid(child_p->fts_statp->st_uid)->pw_name);
					else
						printf("%-*i ", maxunamelen, child_p->fts_statp->st_uid);
				}
				
				if(f_group){
					if ((getgrgid(child_p->fts_statp->st_gid) != NULL) && !f_id)
						printf("%-*s ", maxgnamelen, getgrgid(child_p->fts_statp->st_gid)->gr_name);
					else
						printf("%-*i ", maxgnamelen, child_p->fts_statp->st_gid);
				}
				
				if (!f_human){
					if (S_ISCHR(child_p->fts_statp->st_mode) || S_ISBLK(child_p->fts_statp->st_mode)){

						char devicenum[100];
						sprintf(devicenum, "%i/%i", major(child_p->fts_statp->st_rdev),
							minor(child_p->fts_statp->st_rdev));
						printf("%*s ", (int)maxsizelen, devicenum);
					}
					else{
						printf("%*ju ", (int)maxsizelen, (uintmax_t)child_p->fts_statp->st_size);
					}
				}
				else{
					if (S_ISCHR(child_p->fts_statp->st_mode) || S_ISBLK(child_p->fts_statp->st_mode)){

						char devicenum[100];
						sprintf(devicenum, "%i/%i", major(child_p->fts_statp->st_rdev),
							minor(child_p->fts_statp->st_rdev));
						printf("%*s ", (int)maxsizelen, devicenum);
					}
					else{
						sumsize = (double)child_p->fts_statp->st_size;
						if (sumsize <= 0)
							maxunit = 0;
						else
							maxunit = floor(logk(sumsize));
						if ((int)maxunit == 0){
							printf("%*i ", (int)maxsizelen, (int)(sumsize / pow(1024, maxunit)));
						}
						else{
							if ((sumsize / pow(1024, maxunit)) > 10)
								printf("%*i%c ", (int)maxsizelen - 1, (int)(sumsize / pow(1024, maxunit)), unit_char[(int)maxunit]);
							else
								printf(" %*.1f%c ", (int)maxsizelen-2,(float)(sumsize / pow(1024, maxunit)), unit_char[(int)maxunit]);
						}
					}
				}

				/*choose one of the time types*/
				if (f_ctime == 1)
					t = child_p->fts_statp->st_ctime;
				if (f_atime == 1)
					t = child_p->fts_statp->st_atime;
				if (f_mtime == 1)
					t = child_p->fts_statp->st_mtime;
				curtime = time(NULL);
				char timestring[100];
				if ((t + 31556952 / 2) > curtime){	/*shorter than six months*/
					strftime(timestring, sizeof(timestring), "%b %2d %H:%M", localtime(&t));
					printf("%s ", timestring);
				}
				else{
					strftime(timestring, sizeof(timestring), "%b %2d  %Y", localtime(&t));
					printf("%s ", timestring);
				}

			}

			
			if (!f_quote)	/*use double quotes*/
				printf("%s", child_p->fts_name);
			else{
				printf("\"%s\"", child_p->fts_name);
			}
			
			if(f_dtype){/*display types*/
				if (S_ISDIR(child_p->fts_statp->st_mode))
						printf("/");
				if (f_type){
					if(S_ISREG(child_p->fts_statp->st_mode))
						if ((child_p->fts_statp->st_mode)&S_IXUSR)
							printf("*");				
					if (S_ISLNK(child_p->fts_statp->st_mode))
						printf("@");
					if (S_ISSOCK(child_p->fts_statp->st_mode))
						printf("=");
					if (S_ISFIFO(child_p->fts_statp->st_mode))
						printf("|");
				}
			}

			

			if (f_long){
				if (S_ISLNK(child_p->fts_statp->st_mode)){	/*to display link sources*/
					char file[MAXPATHLEN + 1], path[MAXPATHLEN + 1];
					if (child_p->fts_level == FTS_ROOTLEVEL)
						snprintf(file, sizeof(file), "%s", child_p->fts_name);
					else
						snprintf(file, sizeof(file), "%s/%s",
						child_p->fts_parent->fts_accpath, child_p->fts_name);
					int linklen;
					if ((linklen = readlink(file, path, sizeof(path) - 1)) != -1){
						path[linklen] = '\0';
						printf(" -> ");
						printf("%s", path);

						
					}
				}
			}

			printf("\n");

		}
		child_p = child_p->fts_link;
	}
	if (f_rec)
		printf("\n");
}

/*
 *the funtion is used to compare between a&b by different orders
 */
int sort(const FTSENT **a, const FTSENT **b)
{
	int a_info, b_info;
	/*error case:if either of ab can't be read then return*/
	a_info = (*a)->fts_info;
	if (a_info == FTS_ERR)
		return (0);
	b_info = (*b)->fts_info;
	if (b_info == FTS_ERR)
		return (0);
	/*no stat case: if either one stat is not avaible*/
	if (a_info == FTS_NS || b_info == FTS_NS) {
		if (b_info != FTS_NS)
			return (1);
		else if (a_info != FTS_NS)
			return (-1);
		else
			return (strcmp((*a)->fts_name, (*b)->fts_name));
	}
	/*normal cases*/
	
	if (!f_timeorder && !f_sizeorder)	/*name order*/
		return (f_order*strcmp((*a)->fts_name, (*b)->fts_name));
	else if (f_sizeorder){		/*size order*/
		if ((*b)->fts_statp->st_size > (*a)->fts_statp->st_size)
			return(f_order * 1);
		if ((*a)->fts_statp->st_size > (*b)->fts_statp->st_size)
			return(f_order * -1);
		else
			return (f_order*strcmp((*a)->fts_name, (*b)->fts_name));
	}
	else if (f_mtime){	/*mtime order*/
		if ((*b)->fts_statp->st_mtime > (*a)->fts_statp->st_mtime)
			return(f_order * 1);
		if ((*a)->fts_statp->st_mtime > (*b)->fts_statp->st_mtime)
			return(f_order * -1);
		else
			return (f_order*strcmp((*a)->fts_name, (*b)->fts_name));
	}
	else if (f_atime){		/*atime order*/
		if ((*b)->fts_statp->st_atime > (*a)->fts_statp->st_atime)
			return(f_order * 1);
		if ((*a)->fts_statp->st_atime > (*b)->fts_statp->st_atime)
			return(f_order * -1);
		else
			return (f_order*strcmp((*a)->fts_name, (*b)->fts_name));
	}
	else if (f_ctime){	/*ctime order*/
		if ((*b)->fts_statp->st_ctime > (*a)->fts_statp->st_ctime)
			return(f_order * 1);
		if ((*a)->fts_statp->st_ctime > (*b)->fts_statp->st_ctime)
			return(f_order * -1);
		else
			return (f_order*strcmp((*a)->fts_name, (*b)->fts_name));
	}
	return (strcmp((*a)->fts_name, (*b)->fts_name));

}

/*
 *the funtion is used to return the length of a int-type digit number 
 */
int digitlen(int num){
	char testing[100];
	sprintf(testing, "%i", num);
	return strlen(testing);
}

/*
 *the funtion is used to return log1024() 
 */
double logk(double x){
	return log(x) / log(1024);
}





/*
 *the funtion is used to comprehend the input and set the flags
 */
int main(int argc, char *argv[])
{
	int fts_options;
	char ch;
	char *b;
	char *pwd[] = { ".",NULL};
	/*initialize*/	
	fts_options = FTS_PHYSICAL;	
	blocksize = 512;




	while ((ch = getopt(argc, argv, "AacdfFGghiklnopQRrSstuU1")) != -1){		//read options
		switch (ch){
		case 'A':
			f_all = 1;
			break;
		case 'a':
			f_all = 1;
			fts_options |= FTS_SEEDOT;
			break;
		case 'c':
			f_ctime = 1;
			f_atime = 0;
			f_mtime = 0;
			f_sizeorder = 0;
			f_timeorder = 1;
			break;
		case 'd':
			f_listdir = 1;
			f_rec = 0;
			break;
		case 'f':
			f_all = 1;
			fts_options |= FTS_SEEDOT;
			f_sort = 0;
			break;
		case 'F':
			f_type = 1;
			f_dtype= 1;
			break;
		case 'g':
			f_long = 1;
			f_owner=0;
			break;
		case 'G':
			f_group=0;
			break;
		case 'h':
			f_human = 1;
			break;
		case 'i':
			f_inode = 1;
			break;
		case 'k':
			blocksize = 1024;
			f_human = 0;
			break;
		case 'l':
			f_long = 1;
			break;
		case 'n':
			f_long = 1;
			f_id = 1;
			break;
		case 'o':
			f_long = 1;
			f_group=0;
			break;
		case 'p':
			f_dtype= 1;
			break;
		case 'Q':
			f_quote = 1;
			break;			
		case 'R':
			f_rec = 1;
			break;
		case 'r':
			f_order = -1;
			break;
		case 'S':
			f_sizeorder = 1;
			f_timeorder = 0;
			break;
		case 's':
			f_size = 1;
			break;
		case 't':
			f_sizeorder = 0;
			f_timeorder = 1;
			break;
		case 'u':
			f_ctime = 0;
			f_atime = 1;
			f_mtime = 0;
			f_sizeorder = 0;
			f_timeorder = 1;
			break;
		case 'U':
			f_sort = 0;
			break;
			break;
		case '1':
			break;
		case '?':
			return 0;
		}
	}
	argc -= optind;		//count the path number
	argv += optind;	//get the path




	if (!argc)
		argv =pwd;

	FTS *ftsp = NULL;
	FTSENT *parent_p = NULL;
	FTSENT *child_p = NULL;
	int ch_options = 0;

	if ((ftsp = fts_open(argv, fts_options, f_sort ? &sort : NULL)) == NULL){
		err(EXIT_FAILURE, NULL);
	}
		

	/*if -d , dispaly the path and quit*/
	if (f_listdir) {
		if(fts_children(ftsp, 0))
			display(NULL, fts_children(ftsp, 0));
		else 
			warnx("%s: %s", fts_children(ftsp, 0)->fts_name, strerror(fts_children(ftsp, 0)->fts_errno));
		(void)fts_close(ftsp);
		return 0;
	}


	while ((parent_p = fts_read(ftsp)) != NULL)
	{
		switch (parent_p->fts_info){		/*reference:http://man7.org/linux/man-pages/man3/fts.3.html*/
		case FTS_DC:						/*directory clcyle*/
			warnx("%s: directory causes a cycle", parent_p->fts_name);
			break;
		case FTS_D:						/*directory*/
			if (parent_p->fts_level != FTS_ROOTLEVEL &&
				parent_p->fts_name[0] == '.' && !f_all)
				break;

			child_p = fts_children(ftsp, ch_options);
			
			display(parent_p, child_p);
			if ((!f_rec )&&  (child_p != NULL))
			//if (!f_rec && child_p != NULL)
				(void)fts_set(ftsp, parent_p, FTS_SKIP);
			//printf("cased\n");
			break;
		case FTS_F:		        /*normal file*/
			//printf("casef\n");		
		case FTS_SL:        /*symbol link*/
			//printf("cases\n");	
		case FTS_DEFAULT: /*other file types*/
		case FTS_DOT:		
		case FTS_SLNONE:
		case FTS_NSOK:
			if(!f_rec)
			display(NULL, parent_p);
			break;
		case FTS_DNR:	/*errors*/
		case FTS_ERR:
		case  FTS_NS:
		
			warnx("%s: %s", parent_p->fts_name, strerror(parent_p->fts_errno));
			//printf("case5\n");
			break;
		}
	}

	(void)fts_close(ftsp);



	return 0;
}






