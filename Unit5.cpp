//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "Unit5.h"
#include "login.h"
#include "Unit4.h"
#include "Unit1.h"
#include "wizard.h"
#include "ScanWn.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm5 *Form5;
//---------------------------------------------------------------------------
__fastcall TForm5::TForm5(TComponent* Owner)
	: TForm(Owner)
{
ignore = true;
ScaleTrack->Value = Form1->Layout1->Scale->X;
ScaleTrackY->Value = Form1->Layout1->Scale->Y;
ignore = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm5::ScaleTrackChange(TObject *Sender) {
if (!ignore) {// change scale
  Form1->Layout1->Scale->X = ScaleTrack->Value;
  Form1->Layout1->Scale->Y = ScaleTrackY->Value;
  Form1->FormResize(Sender);
  LoginWn->Scale();
  Form4->Scale();
  WizardWn->Scale();
  Scan->Scale();
}
TextScale->Text = IntToStr((int)(ScaleTrack->Value * 100)) + "%";
TextScaleY->Text = IntToStr((int)(ScaleTrackY->Value * 100)) + "%";
}
//---------------------------------------------------------------------------
void __fastcall TForm5::FormClose(TObject *Sender, TCloseAction &Action) {
Action = TCloseAction::caFree;
}
//---------------------------------------------------------------------------

