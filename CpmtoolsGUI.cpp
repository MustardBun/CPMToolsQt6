//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USERES("CpmtoolsGUI.res");
USEUNIT("cpmfs.c");
USEUNIT("cpm_test.c");
USEFORM("MdiFrame.cpp", FrameForm);
USEUNIT("cpmcp.c");
USEUNIT("mkfs.cpm.c");
USEFORM("MkfsUnit.cpp", MkfsForm);
USEFORM("AboutDlg.cpp", About);
USEUNIT("device_posix.c");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "CP/M tools GUI";
		Application->CreateForm(__classid(TFrameForm), &FrameForm);
		Application->CreateForm(__classid(TMkfsForm), &MkfsForm);
		Application->CreateForm(__classid(TAbout), &About);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
