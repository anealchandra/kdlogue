//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "Unit2.h"
#include "Unit1.h"
#include "Login.h"
#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm2 *Form2;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{


}
//---------------------------------------------------------------------------
void __fastcall TForm2::TreeView1Change(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------
void TForm2::PrepareList() {
//get a list of devices
String pstring = "list/deviceregistration";
bool success = LoginWn->DoQuery(pstring, true);
if (!success) {//possible bug
	Form1->Label1->Text = pstring;
	return;
}
LoginWn->FillTable(pstring);
//extract storageIDs and populate tree
int id = GetCol("StorageId", MainWn->StringGrid1);
int fcount = 0;
for (int i = 0; i < MainWn->StringGrid1->RowCount; i++) {
	String sid = MainWn->StringGrid1->Cells[id][i];
	if (!sid.IsEmpty()) {
		bool found = false;
		for (int i = 0; i < TreeView1->Count; i++) {
			TTreeViewItem* t = TreeView1->Items[i];
			if (t->Text == sid) {
				found = true;
				break;
			}
		}
		if (!found) {
			TTreeViewItem *tNode = new TTreeViewItem(this);
			tNode->Text = sid;
			tNode->Parent = TreeView1;
			fcount++;
		}
	}
}


}
