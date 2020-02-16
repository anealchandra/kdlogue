//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.TreeView.hpp>
#include <FMX.Types.hpp>
#include <Xml.XMLDoc.hpp>
#include <Xml.xmldom.hpp>
#include <Xml.XMLIntf.hpp>
#include <System.Classes.hpp>
#include <FMX.Grid.hpp>
#include <FMX.TabControl.hpp>
#include <FMX.Edit.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Memo.hpp>
#include <FMX.Objects.hpp>


//---------------------------------------------------------------------------
class TMainWn : public TForm
{
__published:	// IDE-managed Components
	TButton *LoginBut;
	TButton *SettingsBut;
	TTabControl *TabControl1;
	TStatusBar *StatusBar1;
	TLayout *Layout1;
	TAniIndicator *AniIndicator1;
	TTabControl *TabControl2;
	TTabItem *TabItem1;
	TMemo *Memo1;
	TTabItem *TabItem2;
	TStringGrid *StringGrid1;
	TLabel *TextScale;
	TTrackBar *ScaleTrack;
	TLabel *Text1;
	TButton *DeleteBut;
	TButton *KDLogBut;
	TImage *Image1;
	TLabel *LabelDaRT;
	void __fastcall LoginButClick(TObject *Sender);
	void __fastcall TreeView1Change(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall SettingsButClick(TObject *Sender);
	void __fastcall ScaleTrackChange(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall DeleteButClick(TObject *Sender);
	void __fastcall StringGrid1DblClick(TObject *Sender);
	void __fastcall KDLogButClick(TObject *Sender);

private:	// User declarations

//String filterstr; //for appending to some DAL operations
public:		// User declarations
String KDVersion;
bool first_activated;
int rowcount;
void addTreeNode(IXMLNode *xmlNode, TTreeViewItem *treeNode);
	__fastcall TMainWn(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainWn *MainWn;
//---------------------------------------------------------------------------
#endif
