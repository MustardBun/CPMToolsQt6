# ---------------------------------------------------------------------------
VERSION = BCB.01
# ---------------------------------------------------------------------------
!ifndef BCB
BCB = $(MAKEDIR)\..
!endif
# ---------------------------------------------------------------------------
PROJECT = CpmtoolsGUI.exe
OBJFILES = CpmtoolsGUI.obj cpmfs.obj cpm_test.obj MdiFrame.obj cpmcp.obj \
   mkfs.cpm.obj MkfsUnit.obj AboutDlg.obj device_posix.obj
RESFILES = CpmtoolsGUI.res
RESDEPEN = $(RESFILES) MdiFrame.dfm MkfsUnit.dfm AboutDlg.dfm
LIBFILES = 
DEFFILE = 
# ---------------------------------------------------------------------------
CFLAG1 = -Od -Hc -w -k -r- -y -v -vi- -c -a4 -b- -w-par -w-inl -Vx -Ve -x
CFLAG2 = -I.;$(BCB)\projects;"c:\documents and settings\êŒìcçféi\my documents";$(BCB)\include;$(BCB)\include\vcl \
   -H=$(BCB)\lib\vcld.csm 
PFLAGS = -AWinTypes=Windows;WinProcs=Windows;DbiTypes=BDE;DbiProcs=BDE;DbiErrs=BDE \
   -U.;$(BCB)\projects;"c:\documents and settings\êŒìcçféi\my documents";$(BCB)\lib\obj;$(BCB)\lib \
   -I.;$(BCB)\projects;"c:\documents and settings\êŒìcçféi\my documents";$(BCB)\include;$(BCB)\include\vcl \
   -v -$Y -$W -$O- -JPHNV -M     
RFLAGS = -i.;$(BCB)\projects;"c:\documents and settings\êŒìcçféi\my documents";$(BCB)\include;$(BCB)\include\vcl 
LFLAGS = -L.;$(BCB)\projects;"c:\documents and settings\êŒìcçféi\my documents";$(BCB)\lib\obj;$(BCB)\lib \
   -aa -Tpe -x -v -V4.0 
IFLAGS = 
LINKER = ilink32
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) vcl.lib import32.lib cp32mt.lib 
# ---------------------------------------------------------------------------
.autodepend

$(PROJECT): $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES) 
!

.pas.hpp:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.pas.obj:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.cpp.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $* 

.c.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $**

.rc.res:
    $(BCB)\BIN\brcc32 $(RFLAGS) $<
#-----------------------------------------------------------------------------
