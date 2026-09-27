//#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "cpmdir.h"
#include "cpmfs.h"

#include "cpmcp.h"
#include "mkfs.cpm.h"

/*----------------
user 0: fixed
----------------*/

const char *err=NULL;
struct cpmSuperBlock drive;
const char *devopts=NULL;
struct cpmInode root;

extern char defpath[];
extern char cur_defpath[];

//-----------------------------------------------------------------------------------------
const char *cpm_ls_test(char image[], char format[], char pat[], int *gargc, char ***gargv)
{
    static char * star[1];
    star[0] = pat;

	err=Device_open(&drive.dev,image,O_RDONLY,devopts);
    if(err!=NULL) {Device_close(&drive.dev); return err;}
	err=cpmReadSuper(&drive,&root,format,0);
    if(err!=NULL) {Device_close(&drive.dev); return err;}
	cpmglob(0,1,star,&root,gargc,gargv);
    cpmUmount2(&drive);
    return err;
}

//--------------------------------------------------------------------------------------------
const char *cpm_fmt_test(int *gargc, char ***gargv)
{
  char line[256];
  FILE *fp;
  int found=0;

  int gargcap=0;

  *gargv=(char**)0;
  *gargc=0;


  if ((fp=fopen(cur_defpath,"r"))==(FILE*)0 && (fp=fopen(defpath,"r"))==(FILE*)0)
  {
		return "Diskdefs file not found.";
  }
  while (fgets(line,sizeof(line),fp)!=(char*)0)
  {
    int argc;
    char *argv[2];

    for (argc=0; argc<1 && (argv[argc]=strtok(argc ? (char*)0 : line," \t\n")); ++argc);
    if ((argv[argc]=strtok((char*)0,"\n"))!=(char*)0) ++argc;
    if (argc==2 && strcmp(argv[0],"diskdef")==0)
    {
        if (*gargc==gargcap) *gargv=realloc(*gargv,sizeof(char*)*(gargcap ? (gargcap*=2) : (gargcap=16)));
        (*gargv)[*gargc]=strcpy(malloc(strlen(argv[1])+1),argv[1]);
        ++*gargc;
        ++found;
    }
  }
  fclose(fp);
  if (!found)
  {
    	return "Can not find format.";
  }
  return 0;
}

//---------------------------------------------------------------------------------
const char *cpm_to_win_test(char image[], char format[], char *src, char *dest)
{
    char cpmname[2+8+1+3+1]; /* 00foobarxy.zzy\0 */
    sprintf(cpmname,"%02d%s",0,src);


	err=Device_open(&drive.dev,image,O_RDONLY,devopts);
    if(err!=NULL) {Device_close(&drive.dev); return err;}
	err=cpmReadSuper(&drive,&root,format,0);
    if(err!=NULL) {Device_close(&drive.dev); return err;}

    err=cpmToUnix(&root,cpmname,dest);
	
    cpmUmount2(&drive);
    return err;
}


//---------------------------------------------------------------------------------

const char *win_to_cpm_test(char image[], char format[], char *src, char *dest)
{
	err=Device_open(&drive.dev,image,O_RDWR,devopts);
    if(err!=NULL) {Device_close(&drive.dev); return err;}
	err=cpmReadSuper(&drive,&root,format,0);
    if(err!=NULL) {Device_close(&drive.dev); return err;}


    err=unixToCpm(&root,src,dest);


    cpmUmount(&drive);

    return err;
}

//---------------------------------------------------------------------------------
const char *cpm_rm_test(char image[], char format[], char *src)
{
    int exitcode=0;
    char cpmname[2+8+1+3+1]; /* 00foobarxy.zzy\0 */

    sprintf(cpmname,"%02d%s",0,src);

	err=Device_open(&drive.dev,image,O_RDWR,devopts);
    if(err!=NULL) {Device_close(&drive.dev); return err;}
	err=cpmReadSuper(&drive,&root,format,0);
    if(err!=NULL) {Device_close(&drive.dev); return err;}


    exitcode=cpmUnlink(&root,cpmname);


    cpmUmount(&drive);


    if(exitcode) return "No such file";
    return err;
}

//---------------------------------------------------------------------------------
const char *creat_cpm_test(char image[], char format[], char **boot)
{
	/* creat_cpm() takes a const-qualified array; the cast keeps the original
	   char** signature used by the existing callers. */
	err=creat_cpm(image, format, (const char **)boot, 0);
    return err;
}
