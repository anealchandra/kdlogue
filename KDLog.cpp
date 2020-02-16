//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
USEFORM("ScanWn.cpp", Scan);
USEFORM("settings.cpp", SettingsWn);
USEFORM("Sample.cpp", SampleWn);
USEFORM("Prepare.cpp", PrepareWn);
USEFORM("Print.cpp", PrintWn);
USEFORM("Unit5.cpp", Form5);
USEFORM("Unit4.cpp", Form4);
USEFORM("Unit1.cpp", Form1);
USEFORM("Unit3.cpp", Form3);
USEFORM("main.cpp", MainWn);
USEFORM("login.cpp", LoginWn);
USEFORM("Wizard.cpp", WizardWn);
//---------------------------------------------------------------------------
extern "C" int FMXmain()
{
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TMainWn), &MainWn);
		Application->CreateForm(__classid(TLoginWn), &LoginWn);
		Application->CreateForm(__classid(TSettingsWn), &SettingsWn);
		Application->CreateForm(__classid(TForm4), &Form4);
		Application->CreateForm(__classid(TScan), &Scan);
		Application->CreateForm(__classid(TWizardWn), &WizardWn);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
