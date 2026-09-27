/* #includes */ /*{{{C}}}*//*{{{*/
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
/*}}}*/

/* 2019.03.15 modified for MITS 88-DISK. neko Java. */
#include "altair.h"
const char MitsFillDat[] = {MITSFLB,MITSFLB,MITSFLB,MITSFLB,MITSFLB,MITSFLB};
extern char mits_137_sect_flg; //added neko Java 2019.04.05

/* Device_open           -- Open an image file                      */ /*{{{*/
const char *Device_open(struct Device *this, const char *filename, int mode, const char *deviceOpts)
{
  if (deviceOpts != NULL)
  {
    return "POSIX driver accepts no options (build compiled without libdsk)";
  }
#if !HAVE_UNISTD_H
  mode |= O_BINARY; // add neko Java 2023.04.30
#endif
  this->fd=open(filename,mode);
  this->opened=(this->fd==-1?0:1);
  return ((this->fd==-1)?strerror(errno):(const char*)0);
}
/*}}}*/
/* Device_setGeometry    -- Set disk geometry                       */ /*{{{*/
const char *Device_setGeometry(struct Device *this, int secLength, int sectrk, int tracks, off_t offset, const char *libdskGeometry)
{
  this->secLength=secLength;
  this->sectrk=sectrk;
  this->tracks=tracks;
  this->offset=offset;
  return NULL;
}
/*}}}*/
/* Device_close          -- Close an image file                     */ /*{{{*/
const char *Device_close(struct Device *this)
{
  this->opened=0;
  return ((close(this->fd)==-1)?strerror(errno):(const char*)0);
}
/*}}}*/
/* Device_readSector     -- read a physical sector                  */ /*{{{*/
const char *Device_readSector(const struct Device *this, int track, int sector, char *buf)
{
  int res;
  
   //add neko Java start 2023.04.23 --------------------------------------------------
  off_t offset;

  if (mits_137_sect_flg){ // modified neko Java 2019.04.05
        offset = (off_t)(((sector+track*this->sectrk)*MITS_PH_SECLEN)+this->offset);
        offset += MITS_SECHDRLEN;
  }else{
        offset = (off_t)(((sector+track*this->sectrk)*this->secLength)+this->offset);
  }
  //add neko Java end 2023.04.23-------------------------------------------------------

  assert(this);
  assert(sector>=0);
  assert(sector<this->sectrk);
  assert(track>=0);
  assert(track<this->tracks);
  assert(buf);
  
  //if (lseek(this->fd,(off_t)(((sector+track*this->sectrk)*this->secLength)+this->offset),SEEK_SET)==-1) 
  if (lseek(this->fd,offset,SEEK_SET)==-1) //mod neko Java 2023.04.23
  {
    return strerror(errno);
  }
  if ((res=read(this->fd, buf, this->secLength)) != this->secLength) 
  {
    if (res==-1)
    {
      return strerror(errno);
    }
    else
{
//printf("len %d\n",this->secLength-res);
 memset(buf+res,0,this->secLength-res); /* hit end of disk image */
}
  }
  return (const char*)0;
}
/*}}}*/
/* Device_writeSector    -- write physical sector                   */ /*{{{*/
const char *Device_writeSector(const struct Device *this, int track, int sector, const char *buf)
{
   //add neko Java start 2023.04.23 --------------------------------------------------
  off_t offset;

  if (mits_137_sect_flg){ // modified neko Java 2019.04.05
        offset = (off_t)(((sector+track*this->sectrk)*MITS_PH_SECLEN)+this->offset);
  }else{
        offset = (off_t)(((sector+track*this->sectrk)*this->secLength)+this->offset);
  }
  //add neko Java end 2023.04.23-------------------------------------------------------

  assert(sector>=0);
  assert(sector<this->sectrk);
  assert(track>=0);
  assert(track<this->tracks);
  //if (lseek(this->fd,(off_t)(((sector+track*this->sectrk)*this->secLength)+this->offset),SEEK_SET)==-1)
  if (lseek(this->fd,offset, SEEK_SET)==-1) //mod neko Java 2023.04.23
  {
    return strerror(errno);
  }
  
  //if (write(this->fd, buf, this->secLength) == this->secLength) return (const char*)0;
// 2023.04.23 modified for MITS 88-DISK. neko Java. start-----------------
  if (mits_137_sect_flg)
  {
  	if (write(this->fd, MitsFillDat, MITS_SECHDRLEN) == MITS_SECHDRLEN &&
  	    write(this->fd,         buf, MITS_LG_SECLEN) == MITS_LG_SECLEN &&
  	    write(this->fd, MitsFillDat, MITS_SECTRLLEN) == MITS_SECTRLLEN) return (const char*)0;
  }
  else
  {
  	if (write(this->fd, buf, this->secLength) == this->secLength) return (const char*)0;
  }
// 2023.04.23 modified for MITS 88-DISK. neko Java. end --------------------
  
  return strerror(errno);
}
/*}}}*/
