#ifndef __SKIN_H_
#define __SKIN_H_

#include <ddraw.h>
#pragma comment(lib,"ddraw.lib")
#include "HWR_API.H"

//#define		MODE_FULLSCREEN

#define		SYSTEMW		800
#define		SYSTEMH		480
#define		VIBRATEPIX	20
#define		FASTMOVEPIX	18

#define		WM_CLICKBUTTON			WM_USER+101
#define		WM_HOLDDOWNBUTTON		WM_USER+102
#define		WM_SWITCHCHANGE			WM_USER+103

#define		WM_BUTTONMOVEOUT		WM_USER+104
#define		WM_BUTTONMOVEIN			WM_USER+105
#define		WM_PROGPOSCHANGE		WM_USER+106
#define		WM_CLICKPAGE			WM_USER+107

#define		WM_FASTMOVE				WM_USER+108
#define		WM_CLICKLISTBUTTON		WM_USER+109
#define		WM_HANDWRITERESULT		WM_USER+110


#define		NONESTOP	0x00
#define		LEFTSTOP	0x01
#define		RIGHTSTOP	0x02
#define		UPSTOP		0x04
#define		DOWNSTOP	0x08

enum EOPENMODE
{
	REFRESH_V_MID,
	REFRESH_V_TOP,
	REFRESH_V_MIX,
	SLIDE_V_TOP,
	SLIDE_H_RIGHT,
	SLIDE_V_MID,
	TRANS_V_TOP,
	OPEN
};

enum EButtonState
{
	UP,
	DOWN,
	DISABLED,
	REMIND
};
enum EButtonFlag
{
	BUTTFLAG_NONE,
	BUTTFLAG_HIGHLIGHT
};
enum ESwitchState
{
	ON,
	OFF
};
enum EMoveDirection
{
	HORIZONTAL,
	VERTICAL
};

enum EGRAPLIST
{
	GRAP_PREV,
	GRAP_MID,
	GRAP_NEXT
};

enum EFASTMOVE
{
	FASTMOVE_UP,
	FASTMOVE_DOWN,
	FASTMOVE_LEFT,
	FASTMOVE_RIGHT
};

enum CONTROLTYPE
{
	CONTROL_BUTT,
	CONTROL_TEXT,
	CONTROL_EDIT,
	CONTROL_PICT,
	CONTROL_PROG,
	CONTROL_LIST,
	CONTROL_GRAP,
	CONTROL_HAND,
	CONTROL_SWIT,
	CONTROL_BINDLIST

};
typedef  void (*FUNCCALLBACK)(void);	
class CSkin
{
	public:
		CSkin						();
		RECT						GetRect();
		bool						IsClick(POINT point);
		bool						IsWindowVisible();
		void						ShowWindow(int nCmdShow,bool bReDraw=true);
		void						LButtonDown();
		void						MoveWindow(int x,int y,int dx,int dy,bool bReDraw=true);
		void						OffSetWindow(int x,int y,bool bReDraw=true);
		virtual void				MouseMove(POINT point,EMoveDirection direction,POINT lastMovePoint);
		void						SetMovable(bool movable);

	protected:
		RECT						m_rct;
		RECT						m_lastDownRCT;
		int							m_nShow;
		bool						m_movable;
		int							m_rctOffset;
};

class CStack
{
	public:
		CStack						();
		void						Push(int data);
		int							GetTop();
		bool						Pop();

	protected:
		struct NODE
		{
			int data;
			NODE *next;
		} *head;
};
class CList
{
public:
	CList						();
	bool						InsertItem(int nItem,TCHAR *szItem);
	bool						ChangeItemText(int nItem,TCHAR *szItem);
	TCHAR						*GetItemText(int nItem);
	int							GetItemIndex(TCHAR *szItem);
	void						DeleteItem(int nItem);
	void						DeleteAllItems();
	int							GetItemCount();

protected:
	struct NODE
	{
		TCHAR *data;
		NODE *next;
	} *head;
	int							m_nItemCount;
};
class CListBox:public CList
{
public:
	CListBox					();
	bool						InsertItem(int nItem,TCHAR *szItem,bool bReDraw=true);
	bool						ChangeItemText(int nItem,TCHAR *szItem,bool bReDraw);
	void						DeleteItem(int nItem,bool bReDraw=true);
	void						DeleteAllItems(bool bReDraw=true);
	bool						LoadListBox(UINT id,int cnt,int x,int y,int dx,int dy1,TCHAR *ResPath,TCHAR *ResFile);
	void						LoadTextPerItem(int id,int cnt,int x,int y ,int dx,int dy,UINT TextFormat=DT_CENTER|DT_VCENTER|DT_WORD_ELLIPSIS,COLORREF TextColor=RGB(255,255,255),DOUBLE TextYard=1.0);
	bool						LoadPictPerItem(int id,int x,int y,int dx,int dy,int ddx,TCHAR *ResPath,TCHAR *ResFile);
	bool						LoadButtPerItem(int id,int cnt,int x,int y,int dx,int dy,TCHAR *ResPath,TCHAR *ResFileUp,TCHAR *ResFileDown,TCHAR *ResFileRemind,TCHAR *ResFileDisabled);
	bool						LoadProg(int dx,TCHAR *ResPath,TCHAR *ResFileBack,TCHAR *ResFileFore);
	void						LButtonDown(POINT point);
	void						MouseMove(POINT point);
	void						LButtonUp(POINT point);
	void						NextPage();
	void						LastPage();
	void						SetToTop();
	void						SetToBottom();
	int							GetTopIndex();
	int							GetCurrentIndex();
	void						SetCurrentIndex(int index,bool bReDraw=true);
	void						SetGesture(bool gesture);
	void						Draw();

protected:
	bool						m_gesture;
	LPDIRECTDRAWSURFACE			m_pImageTmp;
	UINT						m_uID;//本列表的id
	RECT						m_rct;//列表
	RECT						m_rctList;//列表的矩形
	RECT						m_rctProg;//进度条的矩形
	RECT						m_rctDown;
	int							m_pixFirstItem;//第0条的相对y坐标
	int							m_pixPerItem;//一条的高度
	int							m_cntShowItem;//能显示多少条(大条)
	int							m_indexDown;//当前被按下的小条号-1表示没有被按下的
	int							m_indexDownId;
	POINT						m_pointMoveList;
	POINT						m_pointLastMoveList;
	POINT						m_pointLastMoveList2;

	int							m_cntTextPerItem;//一项中包含的文本个数
	int							m_cntShowPerItem;//一项中显示的文本个数
	struct textpara
	{
		RECT					rct;
		COLORREF				color;
		UINT					format;
		LOGFONT					font;
	} m_textpara[4];

	struct pictpara
	{
		RECT rct;
		LPDIRECTDRAWSURFACE		pImage;
	} m_pictpara[3];

	int							m_cntButtPerItem;
	struct buttpara
	{
		RECT rct;
		LPDIRECTDRAWSURFACE		pImageUp;
		LPDIRECTDRAWSURFACE		pImageDown;
		LPDIRECTDRAWSURFACE		pImageRemind;
		LPDIRECTDRAWSURFACE		pImageDisabled;
	} m_buttpara[3];

	struct progpara
	{
		int						pix;//相对位置
		int						len;//before长度
		POINT					pointMove;
		LPDIRECTDRAWSURFACE		pImageBack;
		LPDIRECTDRAWSURFACE		pImageBefore;
	} m_progpara;

	void						DrawItemButt();
	void						DrawItemText();
	void						DrawItemPict();
	void						DrawProg();
	bool						IsItemShow(int index);
	bool						GetPointRct(RECT *rct,POINT point);
	void						OffsetFirstItemPix(int offpix);
};
class CHandWrite:public CSkin
{
public:
	CHandWrite					();
	bool						LoadHandWrite(int x,int y ,int dx,int dy,TCHAR *ResPath,TCHAR *ResFile);
	bool						LoadHandWrite(int x,int y ,int dx,int dy,HINSTANCE hInstance,TCHAR *ResFile);
	void						SetHandWriteRct(UINT id,int x,int y ,int dx,int dy);
	void						SetPenPara(COLORREF Color,int font);
	void						SetOutputType(UINT Type);
	bool						IsInHandWriteRct(POINT point);
	void						LButtonDown(POINT point);
	void						MouseMove(POINT point);
	void						LButtonUp(POINT point);
	void						Draw();
	int							GetResult(TCHAR *result);

	static void					InitHandwrite();
	static void					UninHandwrite();
	static void					OnTimer(HWND hwnd,UINT message,UINT iTimerID,DWORD dwTime);

protected:
	LPDIRECTDRAWSURFACE			m_pImageUp;
	UINT						m_uID;
	RECT						m_rctBack;
	RECT						m_rctBefore;
	COLORREF					m_Color;
	int							m_Font;
	POINT						m_PrePt;
};
class CText:public CSkin
{
	public:
		CText						();
		void						LoadText(int x,int y ,int dx,int dy,UINT TextFormat=DT_CENTER|DT_VCENTER|DT_WORD_ELLIPSIS,COLORREF TextColor=RGB(255,255,255),DOUBLE TextYard=1.4);
		void						SetTextFormat(UINT Format);
		void						SetTextColor(COLORREF Color);
		void						SetTextYard(double Yard);
		void						AddChar(TCHAR tch,UINT iMaxlen=MAX_PATH,bool bReDraw=true);
		void						DelChar(bool bReDraw=true);
		bool						SetWindowsText(const TCHAR *WindowsText,bool bReDraw=true);
		TCHAR						*GetWindowsText();
		void						Draw();
		HFONT						GetFont() {return m_font;}		
		void						GetCharMetrics(TCHAR ch, INT32 *charWidth);
		void						GetTextMetric(TEXTMETRIC *tm);
	protected:
		TCHAR						*m_Caption;
		COLORREF					m_Color;
		UINT						m_uFormat;
		UINT						m_uMaxlen;
		DOUBLE						m_dYard;
		HFONT						m_font;
};
class CButt:public CText
{
	public:
		CButt						();
		bool						LoadButton(UINT ID,int x,int y,int dx ,int dy,int offsety,TCHAR *ResPath,TCHAR *ResFileUp,TCHAR *ResFileDown,TCHAR *ResFileDisabled,TCHAR *ResFileRemind);
		bool						LoadButton(UINT ID,int x,int y ,int dx,int dy,int offsety,HINSTANCE hUiDLL,TCHAR *ResFileUp,TCHAR *ResFileDown,TCHAR *ResFileDisabled,TCHAR *ResFileRemind);
		void						LButtonDown(POINT point,RECT moveRct);
		void						MouseMove(POINT point,EMoveDirection direction,POINT lastMovePoint,RECT moveRect);
		void						LButtonUp(POINT point,RECT moveRct);
		void						SetState(EButtonState State,bool bReDraw=true);
		EButtonState				GetState();
		EButtonFlag					GetFlag() {return m_eFlag;}
		void						Draw();

		static void					OnTimer1(HWND hwnd,UINT message,UINT iTimerID,DWORD dwTime);
		static void					OnTimer2(HWND hwnd,UINT message,UINT iTimerID,DWORD dwTime);
		
	protected:
		UINT						m_uID;
		EButtonState				m_eState;
		EButtonFlag					m_eFlag;

		LPDIRECTDRAWSURFACE			m_pImageUp;
		LPDIRECTDRAWSURFACE			m_pImageDown;
		LPDIRECTDRAWSURFACE			m_pImageDisabled;
		LPDIRECTDRAWSURFACE			m_pImageRemind;
};
class CSwitch:public CButt
{
	public:
		CSwitch						();
		void						LButtonUp(POINT point,RECT moveRct);
		void						Draw();
		void						SetSwitchState(ESwitchState State,bool bReDraw=true);
		ESwitchState				GetSwitchState();

	protected:
		ESwitchState				m_sState;
};
class CPict:public CSkin
{
	public:
		CPict						();
		bool						LoadPicture(int x,int y,int dx ,int dy,int ddx,TCHAR *ResPath,TCHAR *ResFile);
		bool						LoadPicture(int x,int y,int dx,int dy,int ddx,HINSTANCE hUiDLL,TCHAR *ResFile);
		void						Draw();
		void						SetIndex(int nIndex,bool bReDraw=true);

	protected:
		int							m_index;
		LPDIRECTDRAWSURFACE			m_pImageUp;
};
class CProg:public CSkin
{
	public:
		CProg						();
		bool						LoadProgress(UINT ID,int x,int y,int dx ,int dy,int dxmark,int dymark,TCHAR *ResPath,TCHAR *ResFileBack,TCHAR *ResFileFore,TCHAR *ResFileMark);
		bool						LoadProgress(UINT ID,int x,int y,int dx ,int dy,int dxmark,int dymark,HINSTANCE hUiDLL,TCHAR *ResFileBack,TCHAR *ResFileFore,TCHAR *ResFileMark);
		void						Draw();
		void						SetPos(int nPos,bool bReDraw=true);
		int							GetPos();
		void						MouseMove(POINT point);
		
	private:
		UINT						m_uID;
		RECT						m_rcFore;
		int							m_nPosition;
		int							m_nHeigthMark;
		int							m_nWidthMark;
		LPDIRECTDRAWSURFACE			m_pImageBack;
		LPDIRECTDRAWSURFACE			m_pImageBefore;
		LPDIRECTDRAWSURFACE			m_pImageMark;
};
class CPhoto:public CSkin
{
public:
	CPhoto						();
	~CPhoto						();
	bool						LoadPhoto(int x,int y,int dx ,int dy);
	bool						LoadPicture(TCHAR *File);
	void						Draw();
	void						Rotate(float angle);
	void						Zoom(double zoom);

protected:
	RECT						m_rcPict;
	TCHAR						*m_Caption;
	int							m_uMaxlen;
	float						m_angle;
	double						m_zoom;
	LPDIRECTDRAWSURFACE         m_pImage;//原始
	LPDIRECTDRAWSURFACE         m_pImageChange;//变化后
	bool						SetCurrentFile(TCHAR *WindowsText);
	HRESULT						DDCopyBitmap(LPDIRECTDRAWSURFACE pdds, HBITMAP hbm, int x, int y,int dx, int dy);
	void						ZoomRect(int *destW,int *destH,LONG srcW,LONG srcH,RECT *rct);
};
class CGrap:public CSkin
{
	public:
		CGrap						();
		~CGrap						();
		void						SetProp(EGRAPLIST prop);
		EGRAPLIST					GetProp();
		void						SetIndex(int index);
		bool						LoadGraph(EMoveDirection moveDirection);
		void						Draw();
		void						LButtonUp(EMoveDirection moveDirection);
		void						UpdateState(EMoveDirection moveDirection);
		void						Rotate(float angle);

	protected:
		EGRAPLIST					m_prop;
		int							m_index;
		bool						m_bFullScreen;
		float						m_angle;
		bool						m_bUpdate;
		LPDIRECTDRAWSURFACE         m_pImage;//原始
		LPDIRECTDRAWSURFACE         m_pImageChange;//变化后

};
#define BUFFER_MAX 70
typedef enum _EDIT_DIRECTION
{
	EDIT_MOVEUP,
	EDIT_MOVEDOWN,
	EDIT_MOVELEFT,
	EDIT_MOVERIGHT
}EDIT_DIRECTION;
class CEdit
{
public:
	//constructor & destructor
					 CEdit(UINT Options=0);
	virtual			 ~CEdit();
	void			SetText(TCHAR *Text);
	TCHAR *			GetText();
	INT32			GetTextLength() {return m_length;}
    bool			LButtonDown(POINT point);
	void			MouseMove(POINT point, POINT lastPoint);
	void			Draw();
	void			LoadEdit(int x,int y,int dx,int dy,bool MultiLine=false,COLORREF EditColor = RGB(255,255,255),COLORREF TextColor = RGB(0,0,0),DOUBLE TextYard=1.8, UINT BufMax=BUFFER_MAX);
	bool			LoadBackground(int x,int y,int dx,int dy,TCHAR *ResPath,TCHAR *ResFile);
	bool			LoadBackground(int x,int y,int dx,int dy,HINSTANCE hUiDLL,TCHAR *ResFile);
	void			AddChar(TCHAR tch,bool bReDraw=true);
	void			DeleteChar(bool bReDraw=true);
	void			SetTextColor(COLORREF Color);
	void			SetTextYard(double Yard);
	void			ShowWindowCursor(bool show=true);
	void			HideWindowCursor();
private:
	bool			IsShowMax();
	void			GetShowLength(INT32 *len, INT32 *pos);
	void			SetShowBuf();
	INT32			GetEditWidth(INT32 position, EDIT_DIRECTION direction);
	void			GetLineRow(INT32 position, INT32 *line, INT32 *row);
private:
 	// internal objects
 	CText		*m_text;
	RECT		m_rcBackground;
	RECT		m_rcEdit;
	UINT		m_options;
	TCHAR		*m_buffer;
	TCHAR		*m_buffer_show;
	INT32		 m_length_show;
	INT32		 m_position_show;
	INT32		 m_cursor; //same as length, buf[m_cursor-1],buf[m_cursor]
	INT32		 m_length;
	INT32		 m_length_old;
	INT32		 m_length_max;
	bool		 m_multiline;
	INT32		 m_line_count;
	bool		 m_cursor_created;
	bool		 m_font_changed;
	COLORREF	 m_edit_color;
	LPDIRECTDRAWSURFACE	m_pImageBackground;
	
};

#define IDT_TIMER_SCOLL	211
class CBindList
{
public:
						CBindList();
						~CBindList();
	void				LoadBindList(int x,int y,int dx, int dy, int cnt);
	void				SetBindButt(int *butt1, int cnt1,int *butt2,int cnt2,bool overlap);
	void				MouseMove(POINT point, POINT lastPoint);
	void				ResetBindButt();
	void				MaskScrollMsg();
	void				ShowBindList(int index,UINT8 show);
	void				SetScrollEnable(bool value);
private:
	RECT				*m_list;
	INT32				m_list_cnt;
	UINT8				*m_list_show;
	INT32				m_list_index;
	INT32				*m_butt1;
	INT32				m_butt1_cnt;
	int					*m_butt2;
	INT32				m_butt2_cnt; //butt2 can not show with butt1 at the same time
	bool				m_butt2_show;//default butt1
	bool				m_butt_overlap;
	bool				m_scroll_timeout;
	bool				m_scrollEnable;

};
class CPage:public CText
{
	public:
		CPage						();
		bool						LoadPage(UINT ID,int x,int y,int dx,int dy,TCHAR *ResPath,TCHAR *ResFile);
		bool						LoadPage(UINT ID,int x,int y,int dx,int dy,HINSTANCE hUiDLL,TCHAR *ResFile);
		RECT						GetMoveRect();
		void						SetChildText(int textindex);
		void						SetChildEdit(int editindex);
		void						SetChildButton(int buttonindex);
		void						SetChildSwitch(int switchindex);
		void						SetChildPict(int pictureindex);
		void						SetChildProg(int progindex);
		void						SetChildPhoto(int photoindex);
		void						SetChildList(int listindex);
		void						SetChildHandWrite(int handwriteindex);
		void						SetChildGrapList(CList *ListGrap);
		void						SetChildBindList(int bindlistIndex);
		void						SetCurrentGrap(int flag);
		void						RotateCurrentGrap(float angle);
		void						Draw();
		void						LButtonDown(POINT point);
		void						MouseMove(POINT point);
		void						LButtonUp(POINT point);
		void						UpdatePage(RECT *RefRct);
		void						FlipPageRefreshMid();
		void						FlipPageRefreshTop();
		void						FlipPageRefreshMix();
		void						FlipPageSlideRig();
		void						FlipPageSlideTop();
		void						FlipPageSlideMid();
		void						FlipPageTransTop();
		void						SetMoveRect(RECT rctMove);
		void						SetMoveDirection(EMoveDirection direction);
		void						SetMoveStop(int direction);
		void						SetCallback(FUNCCALLBACK enter, FUNCCALLBACK exit);
		int							GetCurrentEdit() {return m_currentEdit;}
		void						SetCurrentEdit(int index) {m_currentEdit=index;}

	protected:
		struct NODEBUTT				{ int id; NODEBUTT *next; } *buttHead;
		struct NODESWITCH			{ int id; NODESWITCH *next; } *switchHead;
		struct NODETEXT				{ int id; NODETEXT *next; } *textHead;
		struct NODEPICT				{ int id; NODEPICT *next; } *pictHead;
		struct NODEPROG				{ int id; NODEPROG *next; } *progHead;
		struct NODEPHOTO			{ int id; NODEPHOTO *next; } *photoHead;
		struct NODELISTBOX			{ int id; NODELISTBOX *next; } *listboxHead;
		struct NODEHANDWRITE		{ int id; NODEHANDWRITE *next; } *handwriteHead;
		struct NODEEDIT				{ int id; NODEEDIT *next; } *editHead;
		struct NODEBINDLIST			{ int id; NODEBINDLIST *next;} *bindlistHead;
		UINT						m_uID;
		LPDIRECTDRAWSURFACE         m_pDDS;//背景
		LPDIRECTDRAWSURFACE         m_pDDSPage;//所有
		int							m_flipCnt;
		POINT						m_lastDownPoint;
		POINT						m_lastMovePoint;
		POINT						m_lastMovePoint2;
		POINT						m_lastMovePoint3;
		RECT						m_moveRect;
		EMoveDirection				m_moveDirection;
		int							m_stopDirection;
		bool						m_bHaveGrap;
		int							m_currentEdit;
public:
		FUNCCALLBACK			m_func_page_enter;
		FUNCCALLBACK			m_func_page_exit;
};
//导出://
extern CStack						g_stackPage;
extern LPDIRECTDRAWSURFACE			g_pDDSPrimary;
bool								Skin_InitSkin(HWND hWnd,CPage *objPage,CText *objText,CButt *objButt,CSwitch *objSwitch,CPict *objPict,CProg *objProg,CListBox *objListBox,CHandWrite *objHandWrite, CEdit *objEdit,CPhoto *objPhoto,CBindList *objBindList);
void								Skin_DestroySkin();
void								Skin_OpenPage(int thispage,EOPENMODE mode=OPEN);
void								Skin_ClosePage(EOPENMODE mode=OPEN);
void								Skin_ClosePageTo(int toPage);
int									Skin_GetPage();
//====//
#endif