//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Edit.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Memo.hpp>
#include <FMX.TabControl.hpp>
#include <FMX.TreeView.hpp>
#include <FMX.ListView.hpp>
#include <FMX.ListView.Types.hpp>
#include <FMX.Colors.hpp>
#include <FMX.Ani.hpp>
#include <FMX.Objects.hpp>
#include <FMX.Dialogs.hpp>
#include <FMX.Effects.hpp>
#include <FMX.ExtCtrls.hpp>
#include <FMX.Menus.hpp>
#include "Winsoft.FireMonkey.Barcode.hpp"
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdHTTP.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <FMX.Grid.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>    // std::sort

class NodeData: TObject {
	public:
	TStringList *orig;
	TStringList *final;
	NodeData() {
		orig = new TStringList();
		final = new TStringList();
	}
};

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TGroupBox *ItemDetailBox;
	TEdit *ItemBarcode;
	TLabel *clt;
	TMemo *ItemNote;
	TButton *Update1;
	TLabel *nlab;
	TLabel *Label6;
	TEdit *Amount;
	TLabel *Label8;
	TPanel *Panel1;
	TTabControl *TabControl1;
	TTabItem *ShortlistTab;
	TTabItem *SearchTab;
	TTabItem *SettingsTab;
	TStatusBar *StatusBar1;
	TProgressBar *ProgressBar1;
	TLabel *Label1;
	TButton *Add;
	TAniIndicator *AniIndicator1;
	TButton *Delete;
	TPanel *Panel2;
	TGroupBox *GroupBox3;
	TButton *SearchBut;
	TComboBox *Oplist;
	TButton *UpdateAll;
	TGroupBox *SeSettings;
	TButton *OnBut;
	TButton *OffBut;
	TButton *SaveShortBut;
	TButton *ClearShortBut;
	TGroupBox *ShSettings;
	TButton *LoadShortBut;
	TButton *Button6;
	TMemo *Memo2;
	TImage *LogIn;
	TImage *LogOut;
	TLabel *LogStat;
	TSaveDialog *SaveDialog1;
	TOpenDialog *OpenDialog1;
	TImage *Image1;
	TImage *Image2;
	TImage *CancelBut;
	TButton *LogStatBut;
	TGroupBox *GroupBox5;
	TButton *UndoBut;
	TButton *ShortListLocBut;
	TButton *Button5;
	TButton *reweighBut;
	TLabel *Label9;
	TLabel *Label10;
	TLabel *Label11;
	TLabel *Label12;
	TLabel *Label13;
	TLabel *Label14;

	TButton *generaltype0;
	TComboBox *StorageId;
	TComboBox *ContainerTypeId;
	TButton *storage;
	TComboBox *ItemStateId;
	TButton *generaltype2;
	TComboBox *SpecimenId;
	TButton *specimen;
	TComboBox *ItemSourceId;
	TButton *contact;
	TComboBox *ItemTypeId;
	TButton *generaltype1;
	TComboBox *ItemUnitId;
	TButton *itemunit;
	TComboBox *ScaleId;
	TButton *deviceregister;
	TMemo *iNameList;
	TSplitter *Splitter1;
	TComboBox *sNameBox;
	TComboBox *sOpBox;
	TComboBox *sFnBox;
	TCheckBox *CheckBox1;
	TGroupBox *GroupBox1;
	TEdit *Edit1;
	TLabel *Label7;
	TLabel *Label15;
	TGroupBox *GroupBox2;
	TComboBox *StSet;
	TComboBox *ItSet;
	TMemo *sNameList;
	TLabel *Label16;
	TLabel *Label17;
	TComboBox *TrialUnitSpecimenId;
	TButton *trialunitspecimen;
	TLabel *Label18;
	TComboBox *ItemOperation;
	TButton *Button9;
	TLabel *Label19;
	TCheckBox *CheckBox4;
	TCheckBox *CheckBox5;
	TCheckBox *CheckBox6;
	TCheckBox *CheckBox7;
	TCheckBox *CheckBox8;
	TCheckBox *CheckBox9;
	TCheckBox *CheckBox10;
	TCheckBox *CheckBox11;
	TGroupBox *GroupBox4;
	TButton *Button10;
	TButton *PrintSBarBut;
	TButton *Button12;
	TButton *Button13;
	TComboBox *ComboBox1;
	TComboEdit *valEdit;
	TLabel *Label2;
	TLabel *Label20;
	TLabel *Label21;
	TLabel *Label22;
	TLayout *Layout1;
	TToolBar *ToolBar2;
	TButton *firstBut;
	TButton *prevBut;
	TButton *next2But;
	TButton *lastBut;
	TLabel *Label23;
	TComboBox *SortChoice;
	TComboBox *ItSort;
	TLabel *Label24;
	TGroupBox *GroupBox6;
	TButton *UndoAllBut;
	TPopupMenu *PopupMenu1;
	TGroupBox *GroupBox7;
	TButton *SaveResBut;
	TButton *LoadResBut;
	TButton *Button18;
	TButton *Button19;
	TMenuItem *RemoveNode;
	TMenuItem *MenuItem2;
	TMenuItem *MenuItem3;
	TTabItem *TabItem1;
	TTabItem *LogTab;
	TListBox *log;
	TToolBar *ToolBar1;
	TButton *ClearLogButton;
	TButton *Button8;
	TButton *HelpBut;
	TImage *Image3;
	TButton *UpdateBut;
	TButton *Button16;
	TButton *PrintIBarBut;
	TButton *PrintSetupBut;
	TPrinterSetupDialog *PrinterSetupDialog1;
	TFBarcode *Barcode1;
	TComboBox *ComboBoxSymbology;
	TLabel *Label25;
	TButton *Button17;
	TLabel *sLabel5;
	TEdit *ScalesSetting;
	TIdHTTP *IdHTTP1;
	TComboBox *itemparentid;
	TButton *itemparent;
	TLabel *Label26;
	TTimer *Timer1;
	TButton *Button21;
	TGroupBox *GroupBox8;
	TComboBox *ShSort;
	TComboBox *ComboBox3;
	TButton *Button22;
	TButton *Button23;
	TButton *Button24;
	TGroupBox *GroupBox9;
	TComboBox *ComboBox2;
	TButton *Button11;
	TComboBox *TranslationCombo;
	TGroupBox *GroupBox10;
	TComboBox *ComboBox4;
	TListBox *TreeView1;
	TImage *Image4;
	TComboBox *cbStyles;
	TLabel *appLab;
	TStyleBook *StyleBook1;
	TTabItem *TabItem2;
	TStringGrid *offStringGrid;
	TLabel *tLabel3;
	TStringColumn *StringColumn1;
	TStringColumn *StringColumn2;
	TStringColumn *StringColumn3;
	TStringColumn *StringColumn4;
	TStringColumn *StringColumn5;
	TTabControl *TabControl2;
	TTabItem *TabItem3;
	TTabItem *TabItem4;
	TStringGrid *onStringGrid;
	TStringColumn *StringColumn6;
	TStringColumn *StringColumn7;
	TStringColumn *StringColumn8;
	TStringColumn *StringColumn9;
	TStringColumn *StringColumn10;
	TLabel *xLabel3;
	TToolBar *ToolBar3;
	TButton *WizardBut;
	TButton *DelBut;
	TLabel *fLabel4;
	TEdit *seEdit;
	TStringColumn *StringColumn55;
	TStringColumn *StringColumn91;
	TLabel *lanLabel;
	TGroupBox *GroupBox11;
	TButton *setSBut;
	TLabel *zzLabel3;
	TButton *PrButa;
	TLabel *nnLabel3;
	TComboBox *modeComboBox5;
   //	TLayout *ScaleRoot;
   //	TLayout *ControlRoot;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall Update1Click(TObject *Sender);
	void __fastcall TreeView1Click(TObject *Sender);
	void __fastcall UpdateAllClick(TObject *Sender);
	void __fastcall AddClick(TObject *Sender);
	void __fastcall DeleteClick(TObject *Sender);
	void __fastcall CancelButClick(TObject *Sender);
	void __fastcall SearchButClick(TObject *Sender);
	void __fastcall SettingsChange(TObject *Sender);
	void __fastcall OnButClick(TObject *Sender);
	void __fastcall OffButClick(TObject *Sender);
	void __fastcall CheckBoxChange(TObject *Sender);
	void __fastcall ShortButClick(TObject *Sender);
	void __fastcall EdKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
		  TShiftState Shift);
	void __fastcall Button6Click(TObject *Sender);
	void __fastcall SaveNow(TObject *Sender);
	void __fastcall LoadNow(TObject *Sender);
	void __fastcall ClearShortButClick(TObject *Sender);
	void __fastcall LogStatButClick(TObject *Sender);
	void __fastcall LogStatButMouseEnter(TObject *Sender);
	void __fastcall ButMouseLeave(TObject *Sender);
	void __fastcall LogTabClick(TObject *Sender);
	void __fastcall SaveShortButClick(TObject *Sender);
	void __fastcall LoadShortButClick(TObject *Sender);
	void __fastcall ClearLogButtonClick(TObject *Sender);
	void __fastcall Update1MouseEnter(TObject *Sender);
	void __fastcall ssgClick(TObject *Sender);
	void __fastcall UndoButClick(TObject *Sender);
	void __fastcall ShortListLocButClick(TObject *Sender);
	void __fastcall ItemBarcodeMouseEnter(TObject *Sender);
	void __fastcall ComboMouseEnter(TObject *Sender);
	void __fastcall ItemNoteMouseEnter(TObject *Sender);
	void __fastcall AmountMouseEnter(TObject *Sender);
	void __fastcall AddFieldButClick(TObject *Sender);
	void __fastcall EdChange(TObject *Sender);
	void __fastcall AddButMouseEnter(TObject *Sender);
	void __fastcall sFnBoxChange(TObject *Sender);
	void __fastcall CheckBox1Change(TObject *Sender);
	void __fastcall ItSetChange(TObject *Sender);
	void __fastcall HelpButClick(TObject *Sender);
	void __fastcall ShortlistDispChange(TObject *Sender);
	void __fastcall Button8Click(TObject *Sender);
	void __fastcall Button10Click(TObject *Sender);
	void __fastcall Button12Click(TObject *Sender);
	void __fastcall Button13Click(TObject *Sender);
	void __fastcall sOpBoxChange(TObject *Sender);
	void __fastcall sNameBoxChange(TObject *Sender);
	void __fastcall ComboBox1Change(TObject *Sender);
	void __fastcall NavButClick(TObject *Sender);
	void __fastcall SortChange(TObject *Sender);
	void __fastcall UndoAllButClick(TObject *Sender);
	void __fastcall UpdateButClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Button19Click(TObject *Sender);
	void __fastcall TreeView1MouseMove(TObject *Sender, TShiftState Shift, float X,
          float Y);
	void __fastcall TreeView1MouseEnter(TObject *Sender);
	void __fastcall TreeView1MouseLeave(TObject *Sender);
	void __fastcall LoadResButClick(TObject *Sender);
	void __fastcall SaveResButClick(TObject *Sender);
	void __fastcall valEditKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
		  TShiftState Shift);
	void __fastcall TabControl1Change(TObject *Sender);
	void __fastcall Button17Click(TObject *Sender);
	void __fastcall TreeView1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  float X, float Y);
	void __fastcall RemoveNodeClick(TObject *Sender);
	void __fastcall ZoomClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall TreeView1Resize(TObject *Sender);
	void __fastcall PrintSetupButClick(TObject *Sender);
	void __fastcall ComboBoxSymbologyChange(TObject *Sender);
	void __fastcall PrintIBarButClick(TObject *Sender);
	void __fastcall reweighButClick(TObject *Sender);
	void __fastcall SubsampleClick(TObject *Sender);
	void __fastcall MenuItem3Click(TObject *Sender);
	void __fastcall valEditKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall Button21Click(TObject *Sender);
	void __fastcall ComboBox3Change(TObject *Sender);
	void __fastcall Button22Click(TObject *Sender);
	void __fastcall ShSortChange(TObject *Sender);
	void __fastcall Button23Click(TObject *Sender);
	void __fastcall Button24Click(TObject *Sender);
	void __fastcall ComboBox2Change(TObject *Sender);
	void __fastcall StSetChange(TObject *Sender);
	void __fastcall Button11Click(TObject *Sender);
	void __fastcall Button18Click(TObject *Sender);
	void __fastcall TranslationComboChange(TObject *Sender);
	void __fastcall WizardButClick(TObject *Sender);
	void __fastcall ComboBox4Change(TObject *Sender);
	void __fastcall TreeView1Compare(TListBoxItem *Item1, TListBoxItem *Item2, int &Result);
	void __fastcall cbStylesClick(TObject *Sender);
	void __fastcall offStringGridSelChanged(TObject *Sender);
	void __fastcall TabControl2Change(TObject *Sender);
	void __fastcall onStringGridSelChanged(TObject *Sender);
	void __fastcall DelButClick(TObject *Sender);
	void __fastcall seEditKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
	void __fastcall setSButClick(TObject *Sender);
	void __fastcall PrButaClick(TObject *Sender);
	void __fastcall modeComboBox5Change(TObject *Sender);
	void __fastcall Splitter1MouseEnter(TObject *Sender);
	void __fastcall Splitter1MouseLeave(TObject *Sender);

private:	// User declarations
//void __fastcall RestThreadTerminated(TObject *Sender);
int SpecialSort(String s1, String s2);
void extractxy(int &x, int &y, String &s);
void __fastcall selRow(TObject *Sender);
bool Login();
void InitSearch();
void UpdateButtonGraphic(TComboBox* combo);
String operation;
int maxpages;
int SearchPage;
void DoSearch();
void ClearMetaData();
String GetSearchComboText(String searchStorage);
TStringList* GetSearchComboObject(String searchStorage);
String GetStorageLabelType();
String getListStrFromObject(TListBoxItem *tNode, String id);
void AddDALOperators(TComboBox *comb);
void SaveSettings2();
void AddCheckBoxes();
void SetResource(TStringGrid *ssg);
TColorButton *selButton;
int itemsTBU;
bool updateAll; //flag to allow user to cancel a multiple commit
TCheckBox* visF[30];  //visible fields  - todo make vector
void CreateSearchCols();
void UpdateItemsTBU();
void StartShortList();
String hintText;
String exptxt;
bool GetResultsVisible(String name);
String getBarcodeType(String Search);
TStringList *translationList[10]; //max of 10 languages supported
bool updating;
int CountDestinationItems(int &empty);
void AddLineToGrid(String line, TStringGrid *ssg);
bool singleSelect;
void LoadTables();
bool drag;
public:		// User declarations
bool CheckandLogin(String message);
TCheckBox* itemR[10000]; //item results - todo make vector
void UpdateSeedProgress();
int pstartpos;
TListBoxItem* AddChildNode(int pos, NodeData *data);//temp
TColorButton* GetButton(TListBoxItem *v); //temp //kludge function because displaying of TreeView alters order of child button!
int getIdOfTitle(TComboBox *combo, String searchtext);
void SetProgressBar();
void RemoveTreeItem(TListBoxItem *t2);
TStringGrid *sg;
bool DontSaveYet;
void TreeViewClear();
bool LoadTranslations();
String getTreeStrFromObject(TListBoxItem *tNode, String id);
String ReadScale(String &units);
void FindInTree(String searchStr);
void SaveLoggedStatus();
bool BarcodeI;  				//barcode input detected
String BarcodeSearch;           //barcode string (non edit controls)
void ValidateEntry(TEdit *e, System::WideChar &KeyChar, String orig, String &current);
TObject* UseCase;
TImage *im2;
String GetItemLabelType();
void ConvertImageToBitmap(TImage *image);
void ShowBarcode(TObject *control);
void SaveWindowSizeSettings();
bool ignore;
TListBoxItem* AddLocBlankShortList();
int getIndex(TComboBox *combo, String search);
String getMapping(TComboBox *combo);
void DoStart();
String updateFile;
bool logTabSeen;
void SetUpLoggedInWindow();
void SetUpLoggedOutWindow();
void CopyControl(TControl* c1, TControl * c2);
TComboBox *sName[20];
TComboBox *sOperator[20];
TComboBox *sField[20];
TComboEdit* sValue[20];
void EnabBut();
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
