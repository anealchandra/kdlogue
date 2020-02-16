//---------------------------------------------------------------------------

#ifndef ScanWnH
#define ScanWnH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>

#include <FMX.ExtCtrls.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Edit.hpp>

#include <FMX.ListBox.hpp>
#include <FMX.Objects.hpp>
#include <FMX.Media.hpp>
#include <FMX.Memo.hpp>
#include <FMX.Dialogs.hpp>
#include "Winsoft.FireMonkey.Barcode.hpp"
#include "Winsoft.FireMonkey.Obr.hpp"
//#include "Winsoft.FireMonkey.Obr.hpp"
//#include "Winsoft.FireMonkey.Barcode.hpp"
//#include "Winsoft.FireMonkey.Obr.hpp"
//---------------------------------------------------------------------------
class TScan : public TForm
{
__published:	// IDE-managed Components
	TEdit *Edit1;
	TMemo *Memo;
	TTimer *Timer1;

	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TMemo *Memo1;
	TScrollBox *ScrollBox1;
	TMediaPlayer *MediaPlayer1;
	TFObr *FObr1;
	TLayout *Layout1;

	void __fastcall CameraComponent1SampleBufferReady(TObject *Sender, const __int64 ATime);
	void __fastcall FObr1BarcodeDetected(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);

private:	// User declarations
TImage *im1;

public:
void Beep(bool error);
void Stop();
void Start();
TCameraComponent *CameraComponent1;
bool wait;
bool found;
bool StopCamera;
void Scale();
	__fastcall TScan(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TScan *Scan;
//---------------------------------------------------------------------------
#endif
