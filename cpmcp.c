/* #includes */ /*{{{C}}}*//*{{{*/
#include "config.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <utime.h>  // add neko Java 2023.04.24

//#include "getopt_.h"		//neko Java 2023.04.11
#include "cpmfs.h"
/*}}}*/

const char cmd[]="cpmcp";
static int text=0;
static int preserve=0;

/**
 * Return the user number.
 * @param s CP/M filename in 0[0]:aaaaaaaa.bbb format.
 * @returns The user number or -1 for no match.
 */
//static int userNumber(const char *s) /*{{{*/
//{
//  if (isdigit(*s) && *(s+1)==':') return (*s-'0');
//  if (isdigit(*s) && isdigit(*(s+1)) && *(s+2)==':') return (10*(*s-'0')+(*(s+1)-'0'));
//  return -1;
//}
/*}}}*/

/**
 * Copy one file from CP/M to UNIX.
 * @param root The inode for the root directory.
 * @param src  The CP/M filename in 00aaaaaaaabbb format.
 * @param dest The UNIX filename.
 * @returns 0 for success, 1 for error.
 */
//static int cpmToUnix(const struct cpmInode *root, const char *src, const char *dest) /*{{{*/		//neko Java 2023.04.11
const char *cpmToUnix(const struct cpmInode *root, const char *src, const char *dest) /*{{{*/
{
  struct cpmInode ino;
  int exitcode=0;

  if (cpmNamei(root,src,&ino)==-1) { fprintf(stderr,"%s: can not open `%s': %s\n",cmd,src,boo); exitcode=1; }
  else
  {
    struct cpmFile file;
    FILE *ufp;

    cpmOpen(&ino,&file,O_RDONLY);
    if ((ufp=fopen(dest,text ? "w" : "wb"))==(FILE*)0) { fprintf(stderr,"%s: can not create %s: %s\n",cmd,dest,strerror(errno)); exitcode=2; }	//change exitcode neko Java 2023.04.11
    else
    {
      int crpending=0;
      int ohno=0;
      ssize_t res;
      char buf[4096];

      while ((res=cpmRead(&file,buf,sizeof(buf)))>0)
      {
        int j;

        for (j=0; j<res; ++j)
        {
          if (text)
          {
            if (buf[j]=='\032') goto endwhile;
            if (crpending)
            {
              if (buf[j]=='\n')
              {
                if (putc('\n',ufp)==EOF) { fprintf(stderr,"%s: can not write %s: %s\n",cmd,dest,strerror(errno)); exitcode=3; ohno=1; goto endwhile; }	//change exitcode neko Java 2023.04.11
                crpending=0;
              }
              else if (putc('\r',ufp)==EOF) { fprintf(stderr,"%s: can not write %s: %s\n",cmd,dest,strerror(errno)); exitcode=4; ohno=1; goto endwhile; }	//change exitcode neko Java 2023.04.11
              crpending=(buf[j]=='\r');
            }
            else
            {
              if (buf[j]=='\r') crpending=1;
              else if (putc(buf[j],ufp)==EOF) { fprintf(stderr,"%s: can not write %s: %s\n",cmd,dest,strerror(errno)); exitcode=5; ohno=1; goto endwhile; }	//change exitcode neko Java 2023.04.11
            }
          }
          else if (putc(buf[j],ufp)==EOF) { fprintf(stderr,"%s: can not write %s: %s\n",cmd,dest,strerror(errno)); exitcode=6; ohno=1; goto endwhile; }	//change exitcode neko Java 2023.04.11
        }
      }
      endwhile:
      if (res==-1 && !ohno) { fprintf(stderr,"%s: can not read %s (%s)\n",cmd,src,boo); exitcode=7; ohno=1; }	//change exitcode neko Java 2023.04.11
      if (fclose(ufp)==EOF && !ohno) { fprintf(stderr,"%s: can not close %s: %s\n",cmd,dest,strerror(errno)); exitcode=8; ohno=1; }	//change exitcode neko Java 2023.04.11
      if (preserve && !ohno && (ino.atime || ino.mtime))
      {
        struct utimbuf ut;

        if (ino.atime) ut.actime=ino.atime; else time(&ut.actime);
        if (ino.mtime) ut.modtime=ino.mtime; else time(&ut.modtime);
        if (utime(dest,&ut)==-1) { fprintf(stderr,"%s: can change timestamps of %s: %s\n",cmd,dest,strerror(errno)); exitcode=9; ohno=1; }	//change exitcode neko Java 2023.04.11
      }
    }
    cpmClose(&file);
  }
  //return exitcode; // neko Java 2023.04.11

  // neko Java add start 2023.04.11
  switch (exitcode) {
  	case 1:
    	return "can not open CP/M file";
  	case 2:
    	return "can not create Windows file";
  	case 3:
    	return "can not write Windows file";
  	case 4:
    	return "can not write Windows file";
  	case 5:
    	return "can not write Windows file";
  	case 6:
    	return "can not write Windows file";
  	case 7:
    	return "can not read CP/M file";
  	case 8:
    	return "can not close Windows file";
  	case 9:
    	return "can change timestamps";
  }
  return NULL;
  // neko Java add end 2023.04.11
}
/*}}}*/

//static void usage(void) /*{{{*/
//{
//  fprintf(stderr,"Usage: %s [-f format] [-p] [-t] image user:file file\n",cmd);
//  fprintf(stderr,"       %s [-f format] [-p] [-t] image user:file ... directory\n",cmd);
//  fprintf(stderr,"       %s [-f format] [-p] [-t] image file user:file\n",cmd);
//  fprintf(stderr,"       %s [-f format] [-p] [-t] image file ... user:\n",cmd);
//  exit(1);
//}
/*}}}*/

// main() ->  mod start neko Java 2023.04.12
const char *unixToCpm(const struct cpmInode *root, const char *src, const char *dest)
{
      /* copy from UNIX to CP/M */ /*{{{*/

      /* variables */ /*{{{*/
      //char *dest=(char*)0;
      FILE *ufp;
      int exitcode=0;
      int c;
      /*}}}*/

      const char cmd[]="unixToCpm"; // neko Java 2023.05.04

      //if ((ufp=fopen(argv[i],"rb"))==(FILE*)0) /* cry a little */ /*{{{*/
      if ((ufp=fopen(src,"rb"))==(FILE*)0) // neko Java 2023.04.24
      {
        //fprintf(stderr,"%s: can not open %s: %s\n",cmd,argv[i],strerror(errno));
        fprintf(stderr,"%s: can not open %s: %s\n",cmd,src,strerror(errno));
        exitcode=1;
      }
      /*}}}*/
      else
      {
        struct cpmInode ino;
        char cpmname[2+8+1+3+1]; /* 00foobarxy.zzy\0 */

        //char *translate;
        struct stat st;

        //stat(argv[i],&st);
        stat(src,&st);

        /*--------------------------------------------------------------
        if (todir)
        {
          if ((dest=strrchr(argv[i],'/'))!=(char*)0) ++dest; else dest=argv[i];
          snprintf(cpmname,sizeof(cpmname),"%02d%s",userNumber(argv[argc-1]),dest);
        }
        else
        {
          snprintf(cpmname,sizeof(cpmname),"%02d%s",userNumber(argv[argc-1]),strchr(argv[argc-1],':')+1);
        }

        translate=cpmname;
        while ((translate=strchr(translate,','))) *translate='/';
        --------------------------------------------------------------*/

        sprintf(cpmname,"%02d%s",0,dest);

        //if (cpmCreat(&root,cpmname,&ino,0666)==-1) /* just cry */ /*{{{*/
        if (cpmCreat(root,cpmname,&ino,0666)==-1) /* just cry */ /*{{{*/
        {
          fprintf(stderr,"%s: can not create %s: %s\n",cmd,cpmname,boo);
          exitcode=2;
        }
        /*}}}*/
        else
        {
          struct cpmFile file;
          int ohno=0;
          char buf[4096+1];

          cpmOpen(&ino,&file,O_WRONLY);
          do
          {
            ssize_t j;

            for (j=0; j<((ssize_t)sizeof(buf)/2) && (c=getc(ufp))!=EOF; ++j)
            {
              if (text && c=='\n') buf[j++]='\r';
              buf[j]=c;
            }
            if (text && c==EOF) buf[j++]='\032';
            if (cpmWrite(&file,buf,j)!=j)
            {
              fprintf(stderr,"%s: can not write %s: %s\n",cmd,dest,boo);
              ohno=1;
              exitcode=3;
              break;
            }
          } while (c!=EOF);
          if (cpmClose(&file)==EOF && !ohno) /* I just can't hold back the tears */ /*{{{*/
          {
            fprintf(stderr,"%s: can not close %s: %s\n",cmd,dest,boo);
            exitcode=4;
          }
          /*}}}*/
          if (preserve && !ohno)
          {
            struct utimbuf times;
            times.actime=st.st_atime;
            times.modtime=st.st_mtime;
            cpmUtime(&ino,&times);
          }
        }
        if (fclose(ufp)==EOF)
        {
          fprintf(stderr,"%s: can not close %s: %s\n",cmd,dest,strerror(errno));
          exitcode=5;
        }
      }

  /*}}}*/
  /*
  if (cpmUmount(&super)==-1)
  {
    fprintf(stderr,"%s: can not umount device: %s\n",cmd,boo);
    exitcode=6;
  }
  exit(exitcode);
  */

	switch (exitcode) {
  	case 1:
    	return "can not open Windows file";
  	case 2:
    	return "can not create CP/M file";
  	case 3:
    	return "can not write CP/M file";
  	case 4:
    	return "can not close CP/M file";
  	case 5:
    	return "can not close CP/M file";
  	//case 6:
    //	return "can not umount device";
	}
  return NULL;
}
// mod end neko Java 2023.04.12

