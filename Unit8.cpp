//---------------------------------------------------------------------------
#include <fmx.h>
#pragma hdrstop
#include "unit1.h"
#include "Unit8.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm8 *Form8;
//---------------------------------------------------------------------------
__fastcall TForm8::TForm8(TComponent* Owner) : TForm(Owner) {
}
//---------------------------------------------------------------------------
void __fastcall TForm8::FormClose(TObject *Sender, TCloseAction &Action) {
Obj->Parent = Form1->TabItem1;
if (Obj != Form1->GroupBox10) {
	Obj->Position->X = 160;
	if (Obj == Form1->GroupBox4) {
		Obj->Position->Y = 328;
	}
}
S->Enabled = true; //only allow 1 settings window to exist
Action = TCloseAction::caFree;
}
//---------------------------------------------------------------------------
void TForm8::Scale(TGroupBox *obj, TObject *s) {
S = reinterpret_cast<TButton*>(s);
S->Enabled = false;
Obj = obj;
Obj->Parent = Layout1;
Obj->Position->X = 5;

Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;

if (Obj == Form1->GroupBox11) {
	ClientHeight = (Obj->Height + 115) * Layout1->Scale->Y;
}
else if (Obj == Form1->GroupBox10) {
	ClientHeight = (Obj->Height) * Layout1->Scale->Y;
	Obj->Position->Y = 5;
	Obj->Width = 450;
}
else if (Obj == Form1->GroupBox4) {
    Form1->GroupBox4->Visible = true;
	Obj->Position->Y = 5;
	ClientHeight = (Obj->Height + 20) * Layout1->Scale->Y;
}
else {
	ClientHeight = (Obj->Height + 50) * Layout1->Scale->Y;
}
ClientWidth = (Obj->Width + 11) * Layout1->Scale->X;
Left = (Form1->Left) + (Form1->Width - Width)/2;
Top = (Form1->Top) + (Form1->Height - Height)/2;
}
//---------------------------------------------------------------------------

