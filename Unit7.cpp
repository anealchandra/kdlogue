#ifdef _Windows
#include <shlobj.h>
#pragma link "shell32.lib"
#else
#include <MacApi.AppKit.hpp>
#include <MacApi.Foundation.hpp>
#include <MacApi.CocoaTypes.hpp>
#endif
#pragma hdrstop

#include "unit7.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


#ifdef _Windows
int CALLBACK BrowseCallback (HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData){
if ((BFFM_INITIALIZED == uMsg) && (lParam != 0)) ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lParam);
return 0;
}
#endif

bool __fastcall mySelectDir(String &_rsPath, String const _sTitle, String const _sStartPath) {

#ifdef _Windows

wchar_t szDisplayName[MAX_PATH+1] = {0};


PIDLIST_ABSOLUTE pidlRoot;
HRESULT hR= SHParseDisplayName(_sStartPath.c_str(), 0, &pidlRoot, 0, 0); // can ignore the attributes-related parms

BROWSEINFO BrowseInfo = {0};
BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
BrowseInfo.pidlRoot= pidlRoot;

BrowseInfo.pszDisplayName = szDisplayName;
BrowseInfo.lpszTitle = _sTitle.c_str();
BrowseInfo.lpfn = &BrowseCallback;
LPITEMIDLIST ItemID = SHBrowseForFolder(&BrowseInfo);       // display  the dialog

if (ItemID) {                                               //if the user pressed OK
	wchar_t DirPath[MAX_PATH+1] = {0};
	if (SHGetPathFromIDList(ItemID, DirPath)) {        		// get the actual path
		_rsPath = ExcludeTrailingBackslash(DirPath);  		// set the FeedRoot with the ending backslash
		ILFree(ItemID);
		return true;
	}
	ILFree(ItemID);                        					// free the object created by the call to "SHBrowseForFolder"
}
return false;

#else
String ADir;
String _sbut = "Choose";
_di_NSURL LInitialDir;

_di_NSOpenPanel LOpenDir;
LOpenDir = TNSOpenPanel::Wrap(TNSOpenPanel::OCClass->openPanel());
LOpenDir->setAllowsMultipleSelection(false);
LOpenDir->setCanChooseFiles(false);
LOpenDir->setCanChooseDirectories(true);
LOpenDir->setTitle(TNSString::Wrap(TNSString::OCClass->stringWithCharacters((char16_t*)_sTitle.c_str(),_sTitle.Length())));
LOpenDir->setPrompt(TNSString::Wrap(TNSString::OCClass->stringWithCharacters((char16_t*)_sbut.c_str(),_sbut.Length())));
ADir = TNSURL::Wrap(LOpenDir->URLs()->objectAtIndex(0))->relativePath()->UTF8String();
LInitialDir = TNSURL::Create();
LInitialDir->initFileURLWithPath(NSSTR(ADir));
LOpenDir->setDirectoryURL(LInitialDir);


if (NSOKButton == LOpenDir->runModal()) {
    _rsPath = TNSURL::Wrap(LOpenDir->URLs()->objectAtIndex(0))->relativePath()->UTF8String();
	return true;
}
else return false;

#endif
}
