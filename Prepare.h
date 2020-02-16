//---------------------------------------------------------------------------

#ifndef PrepareH
#define PrepareH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Dialogs.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Colors.hpp>
#include <FMX.Edit.hpp>
//---------------------------------------------------------------------------
class TPrepareWn : public TForm
{
__published:	// IDE-managed Components
	TListBox *ListBox1;
	TLabel *Label1;
	TLayout *Layout1;
	TPanel *Panel1;
	TStyleBook *StyleBook2;
	void __fastcall AddButClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ListBox1Change(TObject *Sender);
	void __fastcall ListBox1ApplyStyleLookup(TObject *Sender);
private:	// User declarations
TListBoxItem *prevItem;
public:		// User declarations
void Setup();
void Scale();

	__fastcall TPrepareWn(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPrepareWn *PrepareWn;
//---------------------------------------------------------------------------
#endif
