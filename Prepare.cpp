//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "Prepare.h"
#include "unit1.h"
#include "login.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TPrepareWn *PrepareWn;
//---------------------------------------------------------------------------
__fastcall TPrepareWn::TPrepareWn(TComponent* Owner)
	: TForm(Owner) {
	prevItem = NULL;
}
//---------------------------------------------------------------------------
void TPrepareWn::Scale() {
Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;
ClientHeight = ClientHeight * Layout1->Scale->Y;
ClientWidth = ClientWidth * Layout1->Scale->X;
Left = (Form1->Left) + (Form1->Width - Width)/2;
Top = (Form1->Top) + (Form1->Height - Height)/2;
}
//---------------------------------------------------------------------------
void __fastcall TPrepareWn::AddButClick(TObject *Sender) {
/*ListBox2->ItemIndex = -1;
for (int i = ListBox1->Items->Count - 1; i >= 0; i--) {
	if (ListBox1->ItemByIndex(i)->IsSelected) {
		TStringList* pl = (TStringList*)ListBox1->ItemByIndex(i)->Data;
		ListBox2->ItemIndex = ListBox2->Items->AddObject("new", pl);

		break;
	}
} */
}
//---------------------------------------------------------------------------
void __fastcall TPrepareWn::FormClose(TObject *Sender, TCloseAction &Action) {
//Action = TCloseAction::caFree;
}
//---------------------------------------------------------------------------
void TPrepareWn::Setup() {
for (int i = 0; i < Form1->TreeView1->Count; i++) {
	TListBoxItem* t = Form1->TreeView1->ItemByIndex(i);
	NodeData *d = (NodeData*)(t->Data);
	ListBox1->Items->AddObject(t->Text, d->final);

}
}
//---------------------------------------------------------------------------
void __fastcall TPrepareWn::ListBox1Change(TObject *Sender) {
ListBox1ApplyStyleLookup(ListBox1->Selected);
if (prevItem) ListBox1ApplyStyleLookup(prevItem);
Form1->Button24Click(0); //reset the search db tab
Form1->sNameBox->ItemIndex = Form1->sNameBox->Items->IndexOf("SpecimenId");
Form1->sOpBox->ItemIndex = Form1->sOpBox->Items->IndexOf("=");
Form1->valEdit->Text = ListBox1->Selected->Text;
Form1->ComboBox2->ItemIndex = 0;
//sort by descending amount
Form1->ItSort->ItemIndex = Form1->ItSort->Items->IndexOf("Amount");
Form1->SortChoice->ItemIndex = 2;

Form1->SearchButClick(0);
prevItem = (TListBoxItem *)Sender;
if (Form1->sg) {
	Form1->sg->Parent = Panel1;
	ResizeGrid(Form1->sg);
}
if (ListBox1->Selected->Tag == 1) Form1->Label1->Text = "Review and select 1 corresponding item";
}
//---------------------------------------------------------------------------
void __fastcall TPrepareWn::ListBox1ApplyStyleLookup(TObject *Sender) {
TListBoxItem *Item;

Item = (TListBoxItem *)Sender;
if (Item->Tag == 0) {
	Item->StylesData["text.Color"] = TValue::From<unsigned int>(TAlphaColors::Green);
}
else if (Item->Tag == 1){
	Item->StylesData["text.Color"] = TValue::From<unsigned int>(TAlphaColors::Orange);
}
else if (Item->Tag == 2){
	Item->StylesData["text.Color"] = TValue::From<unsigned int>(TAlphaColors::Red);
}
}
//---------------------------------------------------------------------------

