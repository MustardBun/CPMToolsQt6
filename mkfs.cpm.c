/* #includes */ /*{{{C}}}*//*{{{*/
#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#include "getopt_.h"
#include "cpmfs.h"
/*}}}*/
/* #defines */ /*{{{*/
#ifndef O_BINARY
#define O_BINARY 0
#endif
/*}}}*/

/* 2019.03.18 modified for MITS 88-DISK. neko Java start */
#include "altair.h"
extern const char MitsFillDat[];
char FullSizeFlg=1;
char BootSkewFlg=1;
extern char mits_137_sect_flg; //added neko Java 2019.04.05
/* 2019.03.28 modified. neko Java  end */

/* mkfs -- make file system */ /*{{{*/
static int mkfs(struct cpmSuperBlock *drive, const char *name, const char *format, const char *label, char *bootTracks, int timeStamps, int uppercase)
{
  /* variables */ /*{{{*/
  unsigned int i;
  char buf[128];
  char firstbuf[128];
  int fd;
  unsigned int bytes;
  unsigned int trkbytes;
  /*}}}*/

  const char cmd[]="mkfs.cpm"; // neko Java 2023.05.04

  /* open image file */ /*{{{*/
  if ((fd = open(name, O_BINARY|O_CREAT|O_WRONLY, 0666)) < 0)
  {
    boo=strerror(errno);
    return -1;
  }
  /*}}}*/
  /* write system tracks */ /*{{{*/
  /* this initialises only whole tracks, so it skew is not an issue */
  trkbytes=drive->secLength*drive->sectrk;
  /*
  for (i=0; i<trkbytes*drive->boottrk; i+=drive->secLength) if (write(fd, bootTracks+i, drive->secLength)!=(ssize_t)drive->secLength)
  {
    boo=strerror(errno);
    close(fd);
    return -1;
  }
  */
/* 2023.05.03 modified for MITS 88-DISK. neko Java. start */
  if(mits_137_sect_flg)
  {
    for (i=0; i<trkbytes*drive->boottrk; i+=drive->secLength)
    if (write(fd, MitsFillDat ,   MITS_SECHDRLEN)!=MITS_SECHDRLEN   ||
        write(fd, bootTracks+i, drive->secLength)!=drive->secLength ||
        write(fd, MitsFillDat ,   MITS_SECTRLLEN)!=MITS_SECTRLLEN)
    {
      boo=strerror(errno);
      close(fd);
      return -1;
    }
  }
  else
  {
    for (i=0; i<trkbytes*drive->boottrk; i+=drive->secLength) if (write(fd, bootTracks+i, drive->secLength)!=(ssize_t)drive->secLength)
    {
      boo=strerror(errno);
      close(fd);
      return -1;
    }
  }
/* 2023.05.03 modified for MITS 88-DISK. neko Java. end */

  /*}}}*/
  /* write directory */ /*{{{*/
  memset(buf,0xe5,128);
/* 2019.03.28 modified for full size image. neko Java. start */
  if (FullSizeFlg){
    bytes=drive->secLength*drive->sectrk*(drive->tracks - drive->boottrk);
    }else{
    bytes=drive->maxdir*32;
  }
/* 2019.03.28 modified for full size image. neko Java. end */
  //bytes=drive->maxdir*32;
  if (bytes%trkbytes) bytes=((bytes+trkbytes)/trkbytes)*trkbytes;
  if (timeStamps && (drive->type==CPMFS_P2DOS || drive->type==CPMFS_DR3)) buf[3*32]=0x21;
  memcpy(firstbuf,buf,128);
  if (drive->type==CPMFS_DR3)
  {
    time_t now;
    struct tm *t;
    int min,hour,days;

    firstbuf[0]=0x20;
    for (i=0; i<11 && *label; ++i,++label) firstbuf[1+i]=toupper(*label&0x7f);
    while (i<11) firstbuf[1+i++]=' ';
    firstbuf[12]=timeStamps ? 0x11 : 0x01; /* label set and first time stamp is creation date */
    memset(&firstbuf[13],0,1+2+8);
    if (timeStamps)
    {
      int year;

      /* Stamp label. */
      time(&now);
      t=localtime(&now);
      min=((t->tm_min/10)<<4)|(t->tm_min%10);
      hour=((t->tm_hour/10)<<4)|(t->tm_hour%10);
      for (year=1978,days=0; year<1900+t->tm_year; ++year)
      {
        days+=365;
        if (year%4==0 && (year%100!=0 || year%400==0)) ++days;
      }
      days += t->tm_yday + 1;
      firstbuf[24]=firstbuf[28]=days&0xff; firstbuf[25]=firstbuf[29]=days>>8;
      firstbuf[26]=firstbuf[30]=hour;
      firstbuf[27]=firstbuf[31]=min;
    }
  }

  /*
  for (i=0; i<bytes; i+=128) if (write(fd, i==0 ? firstbuf : buf, 128)!=128)
  {
    boo=strerror(errno);
    close(fd);
    return -1;
  }
  */
/* 2019.03.18 modified for MITS 88-DISK. neko Java. start */
  if(mits_137_sect_flg)
  {
    for (i=0; i < bytes; i += 128)
    if (write(fd,MitsFillDat, MITS_SECHDRLEN)!=MITS_SECHDRLEN ||
        write(fd, i==0 ? firstbuf : buf, 128)!=128            ||
        write(fd,MitsFillDat, MITS_SECTRLLEN)!=MITS_SECTRLLEN)
    {
      boo=strerror(errno);
      close(fd);
      return -1;
    }
  }
  else
  {
    for (i=0; i < bytes; i += 128) if (write(fd, i==0 ? firstbuf : buf, 128)!=128)
    {
      boo=strerror(errno);
      close(fd);
      return -1;
    }
  }
/* 2019.03.18 modified for MITS 88-DISK. neko Java. end */

  /*}}}*/
  /* close image file */ /*{{{*/
  if (close(fd)==-1)
  {
    boo=strerror(errno);
    return -1;
  }
  /*}}}*/
  if (timeStamps && !(drive->type==CPMFS_P2DOS || drive->type==CPMFS_DR3))
  {
    int offset,j;
    struct cpmInode ino, root;
    static const char sig[] = "!!!TIME";
    unsigned int records;
    struct dsDate *ds;
    struct cpmSuperBlock super;
    const char *err;

    if ((err=Device_open(&super.dev,name,O_RDWR,NULL)))
    {
      fprintf(stderr,"%s: can not open %s (%s)\n",cmd,name,err);
      exit(1);
    }
    cpmReadSuper(&super,&root,format,uppercase);

    records=root.sb->maxdir/8;
    if (!(ds=malloc(records*128)))
    {
      cpmUmount(&super);
      return -1;
    }
    memset(ds,0,records*128);
    offset=15;
    for (i=0; i<records; i++)
    {
      for (j=0; j<7; j++,offset+=16)
      {
        *((char*)ds+offset) = sig[j];
      }
      /* skip checksum byte */
      offset+=16;
    }

    /* Set things up so cpmSync will generate checksums and write the
     * file.
     */
    if (cpmCreat(&root,"00!!!TIME&.DAT",&ino,0)==-1)
    {
      fprintf(stderr,"%s: Unable to create DateStamper file: %s\n",cmd,boo);
      return -1;
    }
    root.sb->ds=ds;
    root.sb->dirtyDs=1;
    cpmUmount(&super);
  }
  /*}}}*/

  return 0;
}
/*}}}*/

//const char cmd[]="mkfs.cpm";

//int main(int argc, char *argv[]) /*{{{*/
const char *creat_cpm(char *image, const char *format, const char *boot[], int uppercase)	// neko Java 2023.04.19
{
  //char *image;
  //const char *format;
  //int uppercase=0;
  //int c,usage=0;
  int c;
  struct cpmSuperBlock drive;
  struct cpmInode root;
  const char *label="unlabeled";
  int timeStamps=0;
  size_t bootTrackSize,used;
  char *bootTracks;
  //const char *boot[4]={(const char*)0,(const char*)0,(const char*)0,(const char*)0};

  int s=0; size_t t=0; // 2023.05.05 added for boot image skew. neko Java.

/*----------------------------------------------------------------
  if (!(format=getenv("CPMTOOLSFMT"))) format=FORMAT;
  while ((c=getopt(argc,argv,"b:f:L:tuh?"))!=EOF) switch(c)
  {
    case 'b':
    {
      if (boot[0]==(const char*)0) boot[0]=optarg;
      else if (boot[1]==(const char*)0) boot[1]=optarg;
      else if (boot[2]==(const char*)0) boot[2]=optarg;
      else if (boot[3]==(const char*)0) boot[3]=optarg;
      else usage=1;
      break;
    }
    case 'f': format=optarg; break;
    case 'L': label=optarg; break;
    case 't': timeStamps=1; break;
    case 'u': uppercase=1; break;
    case 'h':
    case '?': usage=1; break;
  }

  if (optind!=(argc-1)) usage=1;
  else image=argv[optind++];

  if (usage)
  {
    fprintf(stderr,"Usage: %s [-f format] [-b boot] [-L label] [-t] [-u] image\n",cmd);
    exit(1);
  }
----------------------------------------------------------------------------*/
  drive.dev.opened=0;
  cpmReadSuper(&drive,&root,format,uppercase);
  bootTrackSize=drive.boottrk*drive.secLength*drive.sectrk;
  if ((bootTracks=malloc(bootTrackSize))==(void*)0)
  {
    //fprintf(stderr,"%s: can not allocate boot track buffer: %s\n",cmd,strerror(errno));
    //exit(1);
    return "can not allocate boot track buffer";
  }
  memset(bootTracks,0xe5,bootTrackSize);
  used=0;
  for (c=0; c<4 && boot[c]; ++c)
  {
    int fd;
    //size_t size;
    int size;  //neko Java 2023.05.05

    if ((fd=open(boot[c],O_BINARY|O_RDONLY))==-1)
    {
      //fprintf(stderr,"%s: can not open %s: %s\n",cmd,boot[c],strerror(errno));
      //exit(1);
      return "can not open boot files";
    }
/* 2019.03.28 modified for boot image skew. neko Java. start */
    if (BootSkewFlg==1){
      do
      {
        if (t+drive.skewtab[s]*drive.secLength > bootTrackSize) break;
        size=read(fd,bootTracks+t+drive.skewtab[s]*drive.secLength,drive.secLength);
        if (size!=0) s++;
        if (s==drive.sectrk){
            s=0;
            t+=drive.secLength*drive.sectrk;
        }
      } while (size==drive.secLength);
    }else{
        size=read(fd,bootTracks+used,bootTrackSize-used);
        if (size%drive.secLength) size=(size|(drive.secLength-1))+1;
        used+=size;
    }
/* 2019.03.28 modified for boot image skew. neko Java. end */
/*
    size=read(fd,bootTracks+used,bootTrackSize-used);
#if 0
    fprintf(stderr,"%d %04x %s\n",c,used+0x800,boot[c]);
#endif
    if (size%drive.secLength) size=(size|(drive.secLength-1))+1;
    used+=size;
*/
    close(fd);
  }
  if (mkfs(&drive,image,format,label,bootTracks,timeStamps,uppercase)==-1)
  {
    //fprintf(stderr,"%s: can not make new file system: %s\n",cmd,boo);
    //exit(1);
    return "can not make new file system";
  }
  else //exit(0);
  return NULL;
}
/*}}}*/
