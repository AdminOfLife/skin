
#include "skins.h"
#include <InitGuid.h>
#include <imaging.h>

CStack						g_stackPage;

static HWND					g_hWnd;
static CText				*g_Text;
static CButt				*g_Butt;
static CSwitch				*g_Switch;
static CPict				*g_Pict;
static CProg				*g_Prog;
static CPhoto				*g_Photo;
static CListBox				*g_ListBox;
static CHandWrite			*g_HandWrite;
static CPage				*g_Page;
static CList				*g_listGrap;
static CEdit				*g_Edit;
static CGrap				m_grap[3];
static CBindList			*g_BindList;

static LPDIRECTDRAW			g_pDD;
static DDSURFACEDESC		g_ddsd;
static LPDIRECTDRAWSURFACE  g_pDDSTmp1;
static LPDIRECTDRAWSURFACE  g_pDDSTmp2;

#ifdef MODE_FULLSCREEN
static LPDIRECTDRAWSURFACE	g_pDDSBack;
#endif
LPDIRECTDRAWSURFACE			g_pDDSPrimary;

static void MidRect(RECT *RctDest,RECT Rct1,RECT Rct2)
{
	RctDest->left=(Rct1.left+Rct2.left)/2;
	RctDest->top=(Rct1.top+Rct2.top)/2;
	RctDest->right=(Rct1.right+Rct2.right)/2;
	RctDest->bottom=(Rct1.bottom+Rct2.bottom)/2;
}
static void MidRect2(RECT *RctDest,RECT Rct1,RECT Rct2)
{
	RctDest->left=(Rct1.left+Rct2.left)/2;
	RctDest->top=(Rct1.top+Rct2.top)/2;
	RctDest->right=(Rct1.right+Rct2.right)/2;
	RctDest->bottom=(Rct1.bottom+Rct2.bottom)/2;

}
static HBITMAP LoadImageFromFile(TCHAR * pFileImage)
{ 
	HBITMAP hResult = NULL;
	IImagingFactory *pImgFactory = NULL;
	IImage *pImageBmp = NULL; 
	if (SUCCEEDED(CoCreateInstance (CLSID_ImagingFactory, NULL,CLSCTX_INPROC_SERVER, IID_IImagingFactory, (void **)&pImgFactory)))
	{
		ImageInfo imageInfo;
		HRESULT hr = pImgFactory->CreateImageFromFile(pFileImage, &pImageBmp);
		if (SUCCEEDED(hr)&& SUCCEEDED(pImageBmp->GetImageInfo(&imageInfo)))
		{
			HDC hdc = ::GetDC(g_hWnd);
			HDC dcBitmap=CreateCompatibleDC(hdc);;
			hResult = CreateCompatibleBitmap(hdc, imageInfo.Width, imageInfo.Height);
			SelectObject(dcBitmap, hResult);
			RECT rct={0,0,imageInfo.Width,imageInfo.Height};
			pImageBmp->Draw(dcBitmap,&rct,NULL);  
			DeleteDC(dcBitmap);
			pImageBmp->Release();
		}
		pImgFactory->Release();
	}
	return hResult;
}
static HRESULT DDCopyBitmap(LPDIRECTDRAWSURFACE pdds, HBITMAP hbm, int x, int y,int dx, int dy)
{
	HDC                     hdcImage;
	HDC                     hdc;
	BITMAP                  bm;
	DDSURFACEDESC			ddsd;
	HRESULT                 hr;

	if (hbm == NULL || pdds == NULL) return E_FAIL;
	pdds->Restore();
	hdcImage = CreateCompatibleDC(NULL);
	if (!hdcImage)
		return E_FAIL;
	SelectObject(hdcImage, hbm);

	GetObject(hbm, sizeof(bm), &bm);
	dx = dx == 0 ? bm.bmWidth : dx;
	dy = dy == 0 ? bm.bmHeight : dy;

	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	pdds->GetSurfaceDesc(&ddsd);

	if ((hr = pdds->GetDC(&hdc)) == DD_OK)
	{
		StretchBlt(hdc, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcImage, x, y, dx, dy, SRCCOPY);
		pdds->ReleaseDC(hdc);
	}
	DeleteDC(hdcImage);
	return hr;
}
static HRESULT	PASCAL EnumFunction(LPDIRECTDRAWSURFACE pSurface,LPDDSURFACEDESC lpSurfaceDesc,LPVOID  lpContext)
{
	static bool bCalled=false;
	if (!bCalled) 
	{
		*((LPDIRECTDRAWSURFACE *)lpContext)=pSurface;
		bCalled=true;
		return DDENUMRET_OK;
	}
	else 
	{
		OutputDebugString(L"DDEX1: Enumerated more than surface?");
		pSurface->Release();
		return DDENUMRET_CANCEL;
	}
}
static HRESULT RestoreAll()
{
	HRESULT hRet;
	/*hRet=g_pDDSPrimary->Restore();
	if (hRet==DD_OK)
	{
	hRet=g_Page[0].m_pDDS->Restore();
	if (hRet==DD_OK)
	{
	hRet=g_Page[1].m_pDDS->Restore();
	if (hRet==DD_OK)
	{
	InitSurfaces();
	}
	}
	}*/
	return hRet;
}
bool Skin_InitSkin(HWND hWnd,CPage *objPage,CText *objText,CButt *objButt,CSwitch *objSwitch,CPict *objPict,CProg *objProg,CListBox *objListBox,CHandWrite *objHandWrite, CEdit *objEdit,CPhoto *objPhoto,CBindList *objBindList)
{
	g_hWnd=hWnd;
	g_Page=objPage;
	g_Text=objText;
	g_Butt=objButt;
	g_Switch=objSwitch;
	g_Pict=objPict;
	g_Prog=objProg;
	g_Photo=objPhoto;
	g_ListBox=objListBox;
	g_HandWrite=objHandWrite;
	g_Edit=objEdit; 
	g_BindList=objBindList;
	if(g_HandWrite)
	{
		CHandWrite::InitHandwrite();
	}
	HRESULT hRet;
	hRet=DirectDrawCreate( NULL,&g_pDD,NULL );
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
#ifdef MODE_FULLSCREEN
	hRet=g_pDD->SetCooperativeLevel(hWnd,DDSCL_FULLSCREEN);
#else
	hRet=g_pDD->SetCooperativeLevel(hWnd,DDSCL_NORMAL);
#endif
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
#ifdef MODE_FULLSCREEN
	hRet=g_pDD->SetDisplayMode(SYSTEMW,SYSTEMH,24,0,0 ); 
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
#endif
#ifdef MODE_FULLSCREEN
	g_ddsd.dwSize=sizeof(g_ddsd); 
	g_ddsd.dwFlags=DDSD_CAPS|DDSD_BACKBUFFERCOUNT; 
	g_ddsd.ddsCaps.dwCaps =DDSCAPS_FLIP|DDSCAPS_PRIMARYSURFACE; 
	g_ddsd.dwBackBufferCount=1;
#else
	g_ddsd.dwSize=sizeof(g_ddsd); 
	g_ddsd.dwFlags=DDSD_CAPS; 
	g_ddsd.ddsCaps.dwCaps =DDSCAPS_PRIMARYSURFACE; 
#endif
	hRet=g_pDD->CreateSurface(&g_ddsd,&g_pDDSPrimary,NULL);
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
	//创建剪裁器
	IDirectDrawClipper* lpClipper = NULL;
	hRet = g_pDD->CreateClipper(0, &lpClipper, NULL);
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
	//将剪裁器与窗口关联
	hRet = lpClipper->SetHWnd(0, g_hWnd);
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
	//将剪裁器与主页面关联
	hRet = g_pDDSPrimary->SetClipper(lpClipper);
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
#ifdef MODE_FULLSCREEN
	hRet=g_pDDSPrimary->EnumAttachedSurfaces(&g_pDDSBack,EnumFunction);
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
#endif
	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=SYSTEMH;
	g_ddsd.dwWidth=SYSTEMW;
	hRet=g_pDD->CreateSurface(&g_ddsd,&g_pDDSTmp1,NULL);
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
	hRet=g_pDD->CreateSurface(&g_ddsd,&g_pDDSTmp2,NULL);
	if (hRet != DD_OK) 
	{
		ASSERT(FALSE);
		return hRet;
	}
	return DD_OK;
}
void Skin_DestroySkin()
{
	if( g_pDD!=NULL )
	{
		g_pDD->RestoreDisplayMode();
		if( g_pDDSPrimary!=NULL )
		{
			g_pDDSPrimary->Release();
			g_pDDSPrimary=NULL;
		}
		g_pDD->Release();
		g_pDD=NULL;
	}
	if(g_HandWrite)
	{
		CHandWrite::UninHandwrite();
	}
	DestroyWindow(GetActiveWindow());
}
int Skin_GetPage()
{
	return g_stackPage.GetTop();
}
void Skin_OpenPage(int thispage,EOPENMODE mode)
{
	if(g_stackPage.GetTop() && g_stackPage.GetTop() == thispage) return;

	if(g_stackPage.GetTop() && g_Page[g_stackPage.GetTop()].m_func_page_exit != NULL)
	{
		g_Page[g_stackPage.GetTop()].m_func_page_exit();
	}
	g_stackPage.Push(thispage);
	if(g_Page[g_stackPage.GetTop()].m_func_page_enter != NULL)
	{
		g_Page[g_stackPage.GetTop()].m_func_page_enter();
	}
	g_Page[g_stackPage.GetTop()].Draw();
	switch(mode)
	{
		case REFRESH_V_MID:
			g_Page[g_stackPage.GetTop()].FlipPageRefreshMid();//从中间到两边垂直刷新
			break;
		case REFRESH_V_TOP:
			g_Page[g_stackPage.GetTop()].FlipPageRefreshTop();//从上到下刷新
			break;
		case REFRESH_V_MIX:
			g_Page[g_stackPage.GetTop()].FlipPageRefreshMix();
			break;
		case SLIDE_V_TOP:
			g_Page[g_stackPage.GetTop()].FlipPageSlideTop();//从上到下滑出
			break;
		case SLIDE_H_RIGHT:
			g_Page[g_stackPage.GetTop()].FlipPageSlideRig();//从右到左滑出
			break;
		case SLIDE_V_MID:
			g_Page[g_stackPage.GetTop()].FlipPageSlideMid();//从中间到两边垂直滑出
			break;
		case TRANS_V_TOP:
			g_Page[g_stackPage.GetTop()].FlipPageTransTop();
			break;
		default:
		{
			g_Page[g_stackPage.GetTop()].UpdatePage(NULL);
		}	
	}
}
void Skin_ClosePage(EOPENMODE mode)
{
	if(g_stackPage.GetTop() && g_Page[g_stackPage.GetTop()].m_func_page_exit != NULL)
	{
		g_Page[g_stackPage.GetTop()].m_func_page_exit();
	}
	if(!g_stackPage.Pop()) return;
	if(g_Page[g_stackPage.GetTop()].m_func_page_enter != NULL)
	{
		g_Page[g_stackPage.GetTop()].m_func_page_enter();
	}
	g_Page[g_stackPage.GetTop()].Draw();
	switch(mode)
	{
		case REFRESH_V_MID:
			g_Page[g_stackPage.GetTop()].FlipPageRefreshMid();//从中间到两边垂直刷新
			break;
		case REFRESH_V_TOP:
			g_Page[g_stackPage.GetTop()].FlipPageRefreshTop();//从上到下刷新
			break;
		case REFRESH_V_MIX:
			g_Page[g_stackPage.GetTop()].FlipPageRefreshMix();
			break;
		case SLIDE_V_TOP:
			g_Page[g_stackPage.GetTop()].FlipPageSlideTop();//从上到下滑出
			break;
		case SLIDE_H_RIGHT:
			g_Page[g_stackPage.GetTop()].FlipPageSlideRig();//从右到左滑出
			break;
		case SLIDE_V_MID:
			g_Page[g_stackPage.GetTop()].FlipPageSlideMid();//从中间到两边垂直滑出
			break;
		case TRANS_V_TOP:
			g_Page[g_stackPage.GetTop()].FlipPageTransTop();
			break;
		default:
			{
				g_Page[g_stackPage.GetTop()].UpdatePage(NULL);
			}	
	}
}
void Skin_ClosePageTo(int toPage)
{
	while (g_stackPage.GetTop()&&g_stackPage.GetTop()!=toPage)
	{
		if(g_Page[g_stackPage.GetTop()].m_func_page_exit != NULL)
		{
			g_Page[g_stackPage.GetTop()].m_func_page_exit();
		}

		g_stackPage.Pop();
		if(g_Page[g_stackPage.GetTop()].m_func_page_enter != NULL)
		{
			g_Page[g_stackPage.GetTop()].m_func_page_enter();
		}
	}
	if(!g_stackPage.GetTop())
	{
		Skin_OpenPage(toPage);
	}
	else
	{
		g_Page[g_stackPage.GetTop()].UpdatePage(NULL);
	}
}
CSkin::CSkin()
{
	m_nShow=SW_SHOW;
	SetRectEmpty(&m_rct);
	SetRectEmpty(&m_lastDownRCT);
	m_movable=false;
	m_rctOffset=0;
}

RECT CSkin::GetRect()
{
	return m_rct;
}

bool CSkin::IsClick(POINT point)
{
	if(m_nShow==SW_HIDE) return false;
    return PtInRect(&m_rct,point);
}

bool CSkin::IsWindowVisible()
{
	if(m_nShow!=SW_HIDE)
		return true;
	return false;
}

void CSkin::ShowWindow(int nCmdShow,bool bReDraw)
{
	if(m_nShow==nCmdShow) return;
	m_nShow=nCmdShow;
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
		//g_Page[g_stackPage.GetTop()].UpdatePage(NULL);
	}
}

void CSkin::MoveWindow(int x,int y,int dx,int dy,bool bReDraw)
{
	RECT RectBuf={0};
	CopyRect(&RectBuf,&m_rct);
	SetRect(&m_rct,x,y,x+dx,y+dy);
	g_Page[g_stackPage.GetTop()].Draw();
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
		g_Page[g_stackPage.GetTop()].UpdatePage(&RectBuf);
	}
}

void CSkin::OffSetWindow(int x,int y,bool bReDraw)
{
	RECT RectBuf={0};
	CopyRect(&RectBuf,&m_rct);
	OffsetRect(&m_rct,x,y);
	g_Page[g_stackPage.GetTop()].Draw();
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
		g_Page[g_stackPage.GetTop()].UpdatePage(&RectBuf);
	}
}

void CSkin::LButtonDown()
{
	SetRect(&m_lastDownRCT,m_rct.left,m_rct.top,m_rct.right,m_rct.bottom);
}
void CSkin::MouseMove(POINT point,EMoveDirection direction,POINT lastMovePoint)
{
	if(m_movable)
	{
		if(direction==HORIZONTAL)
		{
			m_rct.right+=point.x-lastMovePoint.x;
			m_rct.left+=point.x-lastMovePoint.x;
		}
		else if(direction==VERTICAL)
		{
			m_rct.bottom+=point.y-lastMovePoint.y;
			m_rct.top+=point.y-lastMovePoint.y;
		}
		else
		{
			m_rct.right+=point.x-lastMovePoint.x;
			m_rct.bottom+=point.y-lastMovePoint.y;
			m_rct.left+=point.x-lastMovePoint.x;
			m_rct.top+=point.y-lastMovePoint.y;
		}
	}
}

void CSkin::SetMovable(bool movable)
{
	m_movable=movable;
}
CStack::CStack()
{
	head=NULL;
}

void CStack::Push(int data)
{
	NODE *p,*s;
	s=(NODE *)new(NODE);
	s->data=data;

	p=head;
	if(head==NULL)
	{
		head=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		head=s;
	}
}

int CStack::GetTop()
{
	if(head!=NULL)
	{
		return head->data;
	}
	else
	{
		return 0;
	}
}

bool CStack::Pop()
{
	NODE *p;
	p=head;
	if(p==NULL) return false;

	head=p->next;
	delete p;
	return true;
}

CList::CList()
{
	m_nItemCount=0; 
	head=NULL;
}

bool CList::InsertItem(int nItem,TCHAR *szItem)
{
	NODE *p,*q,*s;
	s=(NODE *)new(NODE);
	s->data=(TCHAR *)malloc((wcslen(szItem)+1)*sizeof(TCHAR));
	if(s->data==NULL)
	{
		delete s;
		return false;
	}

	memset(s->data,0,(wcslen(szItem)+1)*sizeof(TCHAR));
	wcscpy(s->data,szItem);

	m_nItemCount++;

	p=head;
	if(head==NULL)
	{
		head=s;
		s->next=NULL;
	}
	else if(nItem==0)
	{
		s->next=p;
		head=s;
	}
	else
	{
		int i=0;
		while(i!=nItem&&p->next!=NULL)
		{
			q=p;
			p=p->next;
			i++;
		}

		if(i==nItem)
		{
			q->next=s;
			s->next=p;
		}
		else
		{
			p->next=s;
			s->next=NULL;
		}
	}
	return true;
}

bool CList::ChangeItemText(int nItem,TCHAR *szItem)
{
	NODE *current=head;

	int i=0;
	while(i!=nItem&&current!=NULL)
	{
		current=current->next;
		i++;
	}

	if(i==nItem)
	{
		if(current!=NULL)
		{
			memset(current->data,0,(wcslen(szItem)+1)*sizeof(TCHAR));
			wcscpy(current->data,szItem);
			return true;
		}
	}
	return false;
}
TCHAR *CList::GetItemText(int nItem)
{
	NODE *current=head;

	int i=0;
	while(i!=nItem&&current!=NULL)
	{
		current=current->next;
		i++;
	}

	if(i==nItem)
	{
		if(current!=NULL)
		{
			return current->data;
		}
	}
	return L"";
}

int CList::GetItemIndex(TCHAR *szItem)
{
	NODE *current=head;

	int i=0;
	while(current!=NULL&&wcscmp(current->data,szItem))
	{
		current=current->next;
		i++;
	}
	return i;
}

void CList::DeleteItem(int nItem)
{
	NODE *p,*q;
	p=head;
	if(p==NULL) return;

	if(nItem==0)
	{
		head=p->next;
		free(p->data);
		delete p;
		m_nItemCount--;
	}
	else
	{
		int i=0;
		while(i!=nItem&&p->next!=NULL)
		{
			q=p;
			p=p->next;
			i++;
		}

		if(i==nItem)
		{
			q->next=p->next;
			free(p->data);
			delete p;
			m_nItemCount--;
		}
	}
}

void CList::DeleteAllItems()
{
	while(m_nItemCount>0)
	{
		DeleteItem(0);
	}
}

int CList::GetItemCount()
{
	return m_nItemCount;
}

CListBox::CListBox()
{
	m_gesture=false;
	SetRectEmpty(&m_rct);
	SetRectEmpty(&m_rctList);
	SetRectEmpty(&m_rctDown);
	SetRectEmpty(&m_rctProg);
	SetRectEmpty(&m_textpara[0].rct);
	SetRectEmpty(&m_textpara[1].rct);
	SetRectEmpty(&m_textpara[2].rct);
	SetRectEmpty(&m_pictpara[0].rct);
	SetRectEmpty(&m_pictpara[1].rct);
	SetRectEmpty(&m_pictpara[2].rct);
	memset(&m_textpara[0].font,0,sizeof(LOGFONT));
	memset(&m_textpara[1].font,0,sizeof(LOGFONT));
	memset(&m_textpara[2].font,0,sizeof(LOGFONT));
}

bool CListBox::InsertItem(int nItem,TCHAR *szItem,bool bReDraw)
{
	bool result=CList::InsertItem(nItem,szItem);
	if(bReDraw&&!(m_nItemCount%m_cntTextPerItem))
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
	return result;
}

bool CListBox::ChangeItemText(int nItem,TCHAR *szItem,bool bReDraw)
{
	bool result=CList::ChangeItemText(nItem,szItem);
	if(bReDraw&&!(m_nItemCount%m_cntTextPerItem))
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
	return result;
}
void CListBox::DeleteItem(int nItem,bool bReDraw)
{
	CList::DeleteItem(nItem);
	if(bReDraw&&!(m_nItemCount%m_cntTextPerItem))
	{
		if(m_pixFirstItem+m_nItemCount/m_cntTextPerItem*m_pixPerItem<=m_rctList.bottom-m_rctList.top)
		{
			OffsetFirstItemPix(m_pixPerItem);
			if(m_indexDown>0)
				m_indexDown-=m_cntTextPerItem;
		}
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
}

void CListBox::DeleteAllItems(bool bReDraw)
{
	CList::DeleteAllItems();
	m_pixFirstItem=0;
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
}

bool CListBox::LoadListBox(UINT id,int cnt,int x,int y,int dx,int dy1,TCHAR *ResPath,TCHAR *ResFile)
{
	m_uID=id;
	m_cntShowItem=cnt;
	m_pixPerItem=dy1;
	SetRect(&m_rctList,x,y,x+dx,y+dy1*cnt);
	CopyRect(&m_rct,&m_rctList);

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=SYSTEMH+200;
	g_ddsd.dwWidth=SYSTEMW;
	if(g_pDD->CreateSurface(&g_ddsd,&m_pImageTmp,NULL)!=DD_OK)
		return false;
	if(ResFile!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFile);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(!hbm)
			return false;
		
		DDCopyBitmap(m_pImageTmp,hbm,0,0,dx,dy1*cnt);
		DeleteObject(hbm);
	}
	return true;
}

void CListBox::LoadTextPerItem(int id,int cnt,int x,int y ,int dx,int dy,UINT TextFormat,COLORREF TextColor,DOUBLE TextYard)
{
	if(id+1>m_cntShowPerItem)
		m_cntShowPerItem=id+1;
	m_cntTextPerItem=cnt;

	SetRect(&m_textpara[id].rct,x,y,x+dx,y+dy);
	m_textpara[id].color=TextColor;
	m_textpara[id].format=TextFormat;
	m_textpara[id].font.lfHeight=(long)(-16*TextYard);
	if(TextYard>=1)
	{
		m_textpara[id].font.lfWeight=FW_BOLD ;
	}
	else
	{
		m_textpara[id].font.lfWeight=FW_NORMAL ;
	}
	wcscpy(m_textpara[id].font.lfFaceName,L"Tahoma");

}
bool CListBox::LoadPictPerItem(int id,int x,int y,int dx,int dy,int ddx,TCHAR *ResPath,TCHAR *ResFile)
{
	SetRect(&m_pictpara[id].rct,x,y,x+dx,y+dy);

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwWidth=ddx;
	g_ddsd.dwHeight=dy;

	if(ResFile!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFile);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(!hbm)
			return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pictpara[id].pImage,NULL)!=DD_OK)
			return false;
		DDCopyBitmap(m_pictpara[id].pImage,hbm,0,0,ddx,dy);
		DeleteObject(hbm);
	}
	return true;
}
bool CListBox::LoadButtPerItem(int id,int cnt,int x,int y,int dx,int dy,TCHAR *ResPath,TCHAR *ResFileUp,TCHAR *ResFileDown,TCHAR *ResFileRemind,TCHAR *ResFileDisabled)
{
	SetRect(&m_buttpara[id].rct,x,y,x+dx,y+dy);
	m_cntButtPerItem=cnt;

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwWidth=dx;
	g_ddsd.dwHeight=dy;
	if(ResPath==NULL)
		return false;

	if(ResFileUp!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileUp);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_buttpara[id].pImageUp,NULL)!=DD_OK)
				return false;

			DDCopyBitmap(m_buttpara[id].pImageUp,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileDown!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileDown);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_buttpara[id].pImageDown,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_buttpara[id].pImageDown,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileRemind!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileRemind);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_buttpara[id].pImageRemind,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_buttpara[id].pImageRemind,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileDisabled!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileDisabled);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_buttpara[id].pImageDisabled,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_buttpara[id].pImageDisabled,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	return true;

}
bool CListBox::LoadProg(int dx,TCHAR *ResPath,TCHAR *ResFileBack,TCHAR *ResFileFore)
{
	SetRect(&m_rctProg,m_rctList.right,m_rctList.top,m_rctList.right+dx,m_rctList.bottom);
	UnionRect(&m_rct,&m_rctList,&m_rctProg);


	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=m_rctList.bottom-m_rctList.top;
	g_ddsd.dwWidth=dx;

	if(ResFileBack!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileBack);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_progpara.pImageBack,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_progpara.pImageBack,hbm,0,0,dx,m_rctList.bottom-m_rctList.top);
			DeleteObject(hbm);
		}
	}
	if(ResFileFore!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileFore);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_progpara.pImageBefore,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_progpara.pImageBefore,hbm,0,0,dx,m_rctList.bottom-m_rctList.top);
			DeleteObject(hbm);
		}
	}
	return true;
}
void CListBox::LButtonDown(POINT point)
{
	//list
	if(PtInRect(&m_rctList,point))
	{
		if(point.y-m_rctList.top-m_pixFirstItem<0)
		{
			int tmpid=((point.y-m_rctList.top-m_pixFirstItem)/m_pixPerItem-1)*m_cntTextPerItem;
			if(tmpid>=0&&tmpid<m_nItemCount)
				m_indexDown=tmpid;
		}
		else
		{
			int tmpid=(point.y-m_rctList.top-m_pixFirstItem)/m_pixPerItem*m_cntTextPerItem;
			if(tmpid>=0&&tmpid<m_nItemCount)
				m_indexDown=tmpid;
			else m_indexDown=-1;
		}
		//butt
		for(int i=0;i<m_cntButtPerItem;i++)
		{
			RECT rct={0};
			CopyRect(&rct,&m_buttpara[i].rct);
			OffsetRect(&rct,m_rctList.left,m_rctList.top+(point.y-m_rctList.top)/m_pixPerItem*m_pixPerItem);
			if(PtInRect(&rct,point))
			{
				m_indexDownId=i;
			}
		}

		GetPointRct(&m_rctDown,point);
		m_pointLastMoveList2=point;
		m_pointLastMoveList=point;
		m_pointMoveList=point;
	}
	//progress
	else if(PtInRect(&m_rctProg,point))
	{
		OffsetFirstItemPix(0-(point.y-(m_rctProg.top+m_progpara.pix+m_progpara.len/2))*(m_pixPerItem*m_nItemCount/m_cntTextPerItem)/(m_rctProg.bottom-m_rctProg.top));
		m_progpara.pointMove=point;
	}
}
void CListBox::MouseMove(POINT point)
{
	if(!m_gesture) return;
	//list
	if(PtInRect(&m_rctList,point))
	{
		if(PtInRect(&m_rctDown,point))
		{
		}
		else
		{
			SetRectEmpty(&m_rctDown);
			OffsetFirstItemPix(point.y-m_pointMoveList.y);
			m_pointLastMoveList2=m_pointLastMoveList;
			m_pointLastMoveList=m_pointMoveList;
			m_pointMoveList=point;
			g_Page[g_stackPage.GetTop()].Draw();
			g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
		}
	}
	//progress
	else if(PtInRect(&m_rctProg,point))
	{
		OffsetFirstItemPix(0-(point.y-m_progpara.pointMove.y)*(m_pixPerItem*m_nItemCount/m_cntTextPerItem)/(m_rctProg.bottom-m_rctProg.top));
		m_progpara.pointMove=point;
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
	else
	{
		m_pointLastMoveList2=m_pointLastMoveList;
		m_pointLastMoveList=m_pointMoveList;
		m_pointMoveList=point;
	}
}

void CListBox::LButtonUp(POINT point)
{
	if(PtInRect(&m_rctList,point))
	{
		if(point.y<m_pointLastMoveList2.y-25||point.y>m_pointLastMoveList2.y+25)
		{
			if(m_gesture)
			{
				float flag=1.5;
				if(point.y<m_pointLastMoveList2.y)
				{
					flag=-1.5;
				}
				for(int i=0;i<5;i++)
				{
					OffsetFirstItemPix(flag*m_pixPerItem);
					g_Page[g_stackPage.GetTop()].Draw();
					g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
				}
			}
		}
		else if(!IsRectEmpty(&m_rctDown))
		{
			//butt
			for(int i=0;i<m_cntButtPerItem;i++)
			{
				RECT rct={0};
				CopyRect(&rct,&m_buttpara[i].rct);
				OffsetRect(&rct,m_rctList.left,m_rctList.top+(point.y-m_rctList.top)/m_pixPerItem*m_pixPerItem);
				if(PtInRect(&rct,point))
				{
					if(m_indexDownId==i)
					{
						PostMessage(g_hWnd,WM_CLICKLISTBUTTON,m_uID<<16|m_indexDownId,m_indexDown);
					}
				}
			}
		}
	}
	SetRectEmpty(&m_rctDown);
	m_indexDownId=-1;
}
void CListBox::NextPage()
{
	OffsetFirstItemPix(m_rctList.top-m_rctList.bottom);
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
}
void CListBox::LastPage()
{
	OffsetFirstItemPix(m_rctList.bottom-m_rctList.top);
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
}
void CListBox::SetToTop()
{
	m_pixFirstItem=0;
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
}
void CListBox::SetToBottom()
{
	m_pixFirstItem=m_rctList.bottom-m_rctList.top-m_nItemCount/m_cntTextPerItem*m_pixPerItem;
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
}
int CListBox::GetCurrentIndex()
{
	return m_indexDown;
}
int CListBox::GetTopIndex()
{
	return (0-m_pixFirstItem)/m_pixPerItem*m_cntTextPerItem;
}
void CListBox::SetCurrentIndex(int index,bool bReDraw)
{
	m_indexDown=index;
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
}
void CListBox::SetGesture(bool gesture)
{
	m_gesture=gesture; 
}
void CListBox::Draw()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	/*if(m_pImage)
	m_pImageTmp->Blt(&m_rct,m_pImage,NULL,DDBLT_ROP,&ddbltfx);
	else
	m_pImageTmp->Blt(&m_rct,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
	*/
	//draw to m_pImageTmp
	DrawItemButt();
	DrawItemPict();
	DrawItemText();
	DrawProg();

	RECT rcts={0,0,SYSTEMW,SYSTEMH};
	RECT rct;
	IntersectRect(&rct,&rcts,&m_rct);
	//g_pDDSTmp1->Blt(&rct,m_pImageTmp,&rct,DDBLT_ROP,&ddbltfx);
	g_pDDSTmp2->Blt(&rct,m_pImageTmp,&rct,DDBLT_ROP,&ddbltfx);
}
void CListBox::DrawItemText()
{
	int index=(0-m_pixFirstItem)/m_pixPerItem*m_cntTextPerItem;
	if(index<0) index=0;
	for(int j=0;j<m_cntShowPerItem;j++)
	{
		for(int i=index+j;i<m_nItemCount&&i<index+(m_cntShowItem+1)*m_cntTextPerItem;i+=m_cntTextPerItem)
		{
			HDC hdc;
			HFONT font=CreateFontIndirect(&m_textpara[j].font);
			RECT rct;
			CopyRect(&rct,&m_textpara[j].rct);
			OffsetRect(&rct,m_rctList.left,m_rctList.top+m_pixFirstItem+i/m_cntTextPerItem*m_pixPerItem);
			if (m_pImageTmp->GetDC(&hdc)==DD_OK)
			{
				SetBkMode(hdc,TRANSPARENT);
				::SetTextColor(hdc,m_textpara[j].color);
				SelectObject(hdc,font);
				DrawText( hdc,GetItemText(i),-1,&rct,m_textpara[j].format);
				DeleteObject(font);
				m_pImageTmp->ReleaseDC(hdc);
			}
		}

	}
}
void CListBox::DrawItemPict()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	int index=(0-m_pixFirstItem)/m_pixPerItem*m_cntTextPerItem;
	if(index<0) index=0;
	for(int j=0;j<m_cntTextPerItem-m_cntShowPerItem;j++)
	{
		if(m_pictpara[j].pImage==NULL)
			continue;
		for(int i=index+m_cntShowPerItem+j;i<m_nItemCount&&i<index+(m_cntShowItem+1)*m_cntTextPerItem;i+=m_cntTextPerItem)
		{
			int flag=_wtoi(GetItemText(i));
			while (true)
			{
				RECT rct1={0};
				RECT rct2={0};
				CopyRect(&rct1,&m_pictpara[j].rct);
				OffsetRect(&rct1,m_rctList.left,m_rctList.top+m_pixFirstItem+i/m_cntTextPerItem*m_pixPerItem);
				SetRect(&rct2,flag*(m_pictpara[j].rct.right-m_pictpara[j].rct.left),0,(flag+1)*(m_pictpara[j].rct.right-m_pictpara[j].rct.left),m_pictpara[j].rct.bottom-m_pictpara[j].rct.top);	
				hRet=m_pImageTmp->Blt(&rct1,m_pictpara[j].pImage,&rct2,DDBLT_ROP,&ddbltfx);
				if (hRet==DD_OK)
					break;
				if (hRet==DDERR_SURFACELOST)
				{
					hRet=RestoreAll();
					if (hRet!=DD_OK)
						break;
				}
				if (hRet!=DDERR_WASSTILLDRAWING)
					break;
			}
		}
	}
}
void CListBox::DrawItemButt()
{
	if(!m_cntButtPerItem)
	{
		return;
	}
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	if(!IsItemShow(m_indexDown))
		m_indexDown=-1;

	int dy=m_pixFirstItem%m_pixPerItem;
	if(dy>0) dy-=m_pixPerItem;
	for(int i=dy;i<m_rctList.bottom-m_rctList.top;i+=m_pixPerItem)
	{
		RECT rct;
		SetRect(&rct,m_rctList.left,m_rctList.top+i,m_rctList.right,m_rctList.top+i+m_pixPerItem);
		while (true)
		{
			if(m_indexDown==(i-m_pixFirstItem)/m_pixPerItem*m_cntTextPerItem)
			{
				//butt
				for(int j=0;j<m_cntButtPerItem;j++)
				{
					if(j>0&&(i-m_pixFirstItem)/m_pixPerItem*m_cntTextPerItem>=m_nItemCount)
					{
						continue;
					}
					RECT rcts={0};
					CopyRect(&rcts,&m_buttpara[j].rct);
					OffsetRect(&rcts,rct.left,rct.top);
					if((j==m_indexDownId||j==0)&&m_buttpara[j].pImageDown)
						hRet=m_pImageTmp->Blt(&rcts,m_buttpara[j].pImageDown,NULL,DDBLT_ROP,&ddbltfx);
					else if(m_buttpara[j].pImageUp)
						hRet=m_pImageTmp->Blt(&rcts,m_buttpara[j].pImageUp,NULL,DDBLT_ROP,&ddbltfx);
				}
			}
			else
			{
				for(int j=0;j<m_cntButtPerItem;j++)
				{
					if(j>0&&(i-m_pixFirstItem)/m_pixPerItem*m_cntTextPerItem>=m_nItemCount)
					{
						continue;
					}
					if(m_buttpara[j].pImageUp)
					{
						RECT rcts={0};
						CopyRect(&rcts,&m_buttpara[j].rct);
						OffsetRect(&rcts,rct.left,rct.top);
						hRet=m_pImageTmp->Blt(&rcts,m_buttpara[j].pImageUp,NULL,DDBLT_ROP,&ddbltfx);
					}
				}
			}

			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
	}
}
void CListBox::DrawProg()
{
	if(m_progpara.pImageBack==NULL&&m_progpara.pImageBefore==NULL) return;

	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	while (true)
	{
		if(m_progpara.pImageBack)
		{
			hRet=m_pImageTmp->Blt(&m_rctProg,m_progpara.pImageBack,NULL,DDBLT_ROP,&ddbltfx);
		}
		if(m_progpara.pImageBefore)
		{
			if(m_nItemCount<=m_cntShowItem*m_cntTextPerItem)
			{
				m_progpara.len=m_rctProg.bottom-m_rctProg.top;
				m_progpara.pix=0;
			}
			else
			{
				m_progpara.len=(m_rctProg.bottom-m_rctProg.top)*(m_rctProg.bottom-m_rctProg.top)/(m_nItemCount*m_pixPerItem/m_cntTextPerItem);
				m_progpara.pix=(0-m_pixFirstItem)*(m_rctProg.bottom-m_rctProg.top)/(m_nItemCount*m_pixPerItem/m_cntTextPerItem);
			}
			RECT rct={0};
			RECT rct1={0};
			SetRect(&rct,m_rctProg.left,m_rctProg.top+m_progpara.pix,m_rctProg.right,m_rctProg.top+m_progpara.pix+m_progpara.len);
			CopyRect(&rct1,&rct);
			OffsetRect(&rct1,0-m_rctProg.left,0-m_rctProg.top);
			hRet=m_pImageTmp->Blt(&rct,m_progpara.pImageBefore,&rct1,DDBLT_ROP,&ddbltfx);
		}
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
}
bool CListBox::IsItemShow(int index)
{
	if(m_indexDown>=m_nItemCount)
		return false;
	int pix=index/m_cntTextPerItem*m_pixPerItem;
	if(pix+m_pixPerItem>0-m_pixFirstItem&&pix<0-m_pixFirstItem+m_rctList.bottom-m_rctList.top)
		return true;
	return false;
}
bool CListBox::GetPointRct(RECT *rct,POINT point)
{
	for(int y=m_pixFirstItem;y<=m_rctList.bottom-m_rctList.top;y+=m_pixPerItem)
	{
		if(y>point.y-m_rctList.top)
		{
			SetRect(rct,m_rctList.left,y-m_pixPerItem+m_rctList.top,m_rctList.right,y+m_rctList.top);
			return true;
		}
	}
	return false;
}
void CListBox::OffsetFirstItemPix(int offpix)
{
	m_pixFirstItem+=offpix;
	if(m_pixFirstItem+m_nItemCount/m_cntTextPerItem*m_pixPerItem<=m_rctList.bottom-m_rctList.top/*&&point.y<=m_pointMoveList.y*/)
	{
		m_pixFirstItem=m_rctList.bottom-m_rctList.top-m_nItemCount/m_cntTextPerItem*m_pixPerItem;
	}
	if(m_pixFirstItem>0)
	{
		m_pixFirstItem=0;
	}
}

static HINSTANCE			hRecogLib;
static HWRInitialize		HWRInitialize1;
static HWRRelease			HWRRelease1;
static sub					sub1;
static segment_one_stroke	segment_one_stroke1;
static start_recognition	start_recognition1;
static unsigned char*		WorkBuf;
static point_type			RecordPt[2048];
static int							m_cntPt=0;
static TCHAR						m_HandWriteSlect[8];
static int							m_HandWriteResult=0;
static UINT							m_InputType=CHINESE_TYPE|ASCII_TYPE;
static UINT							m_ShapeMode=0x11;
CHandWrite::CHandWrite()
{
	SetRectEmpty(&m_rctBack);
	SetRectEmpty(&m_rctBefore);
	m_nShow=SW_SHOW;
	m_Color=RGB(0,0,0);
	m_Font=5;
}
void CHandWrite::InitHandwrite()
{
	hRecogLib = LoadLibrary(TEXT("handwrite.dll"));
	if (hRecogLib == NULL)
	{
		MessageBox(g_hWnd,_T("找不到文件handwrite.dll"), NULL,MB_OK);
		return;
	}
	HWRInitialize1 = (int (WINAPI *)(unsigned char *, unsigned char *, unsigned char *, unsigned char *))GetProcAddress(hRecogLib, L"HWRInitialize");
	HWRRelease1 = (int (WINAPI *)(unsigned char *))GetProcAddress(hRecogLib, TEXT("HWRRelease"));
	sub1 = (struct RecogResultStruct * (WINAPI *)(unsigned char *, short, short, struct point_type *))GetProcAddress(hRecogLib, TEXT("SUB"));
	start_recognition1 = (struct SegmentResultStruct * (WINAPI *)(unsigned char *, short, short))GetProcAddress(hRecogLib, TEXT("SR"));
	segment_one_stroke1 = (void (WINAPI *)(unsigned char *, short, short, struct point_type *))GetProcAddress(hRecogLib, TEXT("SOS"));
	WorkBuf = new unsigned char[80 * 1024];
	memset(WorkBuf,0,80 * 1024*sizeof(unsigned char));
	HWRInitialize1(WorkBuf,NULL,NULL,NULL);
}
void CHandWrite::UninHandwrite()
{
	free(WorkBuf);
	FreeLibrary(hRecogLib);
}
bool CHandWrite::LoadHandWrite(int x,int y ,int dx,int dy,TCHAR *ResPath,TCHAR *ResFile)
{
	SetRect(&m_rctBack,x,y,x+dx,y+dy);
	SetRect(&m_rct,x,y,x+dx,y+dy);
	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwWidth=dx;
	g_ddsd.dwHeight=dy;
	if(ResFile!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFile);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(!hbm)
			return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImageUp,NULL)!=DD_OK)
			return false;
		DDCopyBitmap(m_pImageUp,hbm,0,0,dx,dy);
		DeleteObject(hbm);
	}
	return true;
}
bool CHandWrite::LoadHandWrite(int x,int y ,int dx,int dy,HINSTANCE hInstance,TCHAR *ResFile)
{
	SetRect(&m_rctBack,x,y,x+dx,y+dy);
	SetRect(&m_rct,x,y,x+dx,y+dy);
	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwWidth=dx;
	g_ddsd.dwHeight=dy;
	if(ResFile!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hInstance,ResFile);
		if(!hbm)
			return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImageUp,NULL)!=DD_OK)
			return false;
		DDCopyBitmap(m_pImageUp,hbm,0,0,dx,dy);
		DeleteObject(hbm);
	}
	return true;
}
void CHandWrite::SetHandWriteRct(UINT id,int x,int y ,int dx,int dy)
{
	m_uID=id;
	SetRect(&m_rctBefore,x,y,x+dx,y+dy);
}
void CHandWrite::SetPenPara(COLORREF Color,int font)
{
	m_Color=Color;
	m_Font=font;
}
void CHandWrite::SetOutputType(UINT Type)
{
	m_InputType=Type;
}

bool CHandWrite::IsInHandWriteRct(POINT point)
{
	if(m_nShow==SW_HIDE) 
		return false;
	if(PtInRect(&m_rctBefore,point)||m_cntPt>0)
		return true;
	return false;
}
void CHandWrite::LButtonDown(POINT point)
{
	if(PtInRect(&m_rctBefore,point))//在手写区
	{
		if(m_cntPt==0)
		{
			memset(RecordPt,0,2048*sizeof(point_type));
			memset(WorkBuf,0,80 * 1024*sizeof(unsigned char));
			HWRInitialize1(WorkBuf,NULL,NULL,NULL);
		}
		KillTimer(g_hWnd,m_uID+1);
		m_PrePt=point;
		RecordPt[m_cntPt].x=(short)point.x;
		RecordPt[m_cntPt].y=(short)point.y;
		m_cntPt++;
	}
}
void CHandWrite::MouseMove(POINT point)
{
	if(PtInRect(&m_rctBefore,point))
	{
		HDC hdc;
		if(g_pDDSPrimary->GetDC(&hdc)==DD_OK)
		{
			HPEN hNewPen = CreatePen(PS_SOLID,m_Font,m_Color);
			HPEN hOldPen = (HPEN)SelectObject(hdc, hNewPen);
			if(m_cntPt==0)
			{
				MoveToEx(hdc,point.x,point.y,NULL);
			}
			else
			{
				MoveToEx(hdc,m_PrePt.x,m_PrePt.y,NULL);
			}
			m_PrePt=point;
			LineTo(hdc,point.x,point.y);
			DeleteObject(hNewPen);
			g_pDDSPrimary->ReleaseDC(hdc);
		}
		if(m_cntPt<1000)
		{
			RecordPt[m_cntPt].x=(short)point.x;
			RecordPt[m_cntPt].y=(short)point.y;
			m_cntPt++;
		}
	}
}
void CHandWrite::LButtonUp(POINT point)
{
	if(PtInRect(&m_rctBefore,point))
	{
		SetTimer(g_hWnd,m_uID+1,500,OnTimer);
		RecordPt[m_cntPt].x=-1;
		RecordPt[m_cntPt].y=-1;
		m_cntPt++;
	}
	else
	{
		if(m_cntPt>0)
		{
			SetTimer(g_hWnd,m_uID+1,500,OnTimer);
			RecordPt[m_cntPt].x=-1;
			RecordPt[m_cntPt].y=-1;
			m_cntPt++;
		}
	}
}
void CHandWrite::Draw()
{
	if(m_nShow==SW_HIDE) return;
	if(m_pImageUp==NULL) return;

	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	while (true)
	{
		hRet=g_pDDSTmp1->Blt(&m_rctBack,m_pImageUp,NULL,DDBLT_ROP,&ddbltfx);
		hRet=g_pDDSTmp2->Blt(&m_rctBack,m_pImageUp,NULL,DDBLT_ROP,&ddbltfx);

		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}

}
int CHandWrite::GetResult(TCHAR *result)
{
	for(int i=0;i<m_HandWriteResult;i++)
	{
		result[i]=m_HandWriteSlect[i];
	}
	return m_HandWriteResult;
}
void CALLBACK CHandWrite::OnTimer(HWND hwnd,UINT message,UINT iTimerID,DWORD dwTime)
{
	KillTimer(g_hWnd,iTimerID);
	RecordPt[m_cntPt].x=-2;
	RecordPt[m_cntPt].y=-2;
	m_cntPt=0;

	struct RecogResultStruct far *ptr_RRS = NULL;
	ptr_RRS = (*sub1)(WorkBuf, m_InputType, m_ShapeMode, RecordPt); 

	int i=0;
	while(i<ptr_RRS->counter&&i<8)
	{
		m_HandWriteSlect[i]=ptr_RRS->string[i];
		i++;
	}
	m_HandWriteResult=i;
	PostMessage(g_hWnd,WM_HANDWRITERESULT,iTimerID-1,0);
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(NULL);
}
CText::CText()
{
	m_uFormat=DT_CENTER|DT_WORD_ELLIPSIS;
	m_Color=RGB(221,221,221);
	m_uMaxlen=0;
	m_dYard=1.4;
	m_Caption=NULL;
	m_font = NULL;
	m_rctOffset=0;
}

void CText::LoadText(int x,int y,int dx,int dy,UINT TextFormat,COLORREF TextColor,DOUBLE TextYard)
{
	SetRect(&m_rct,x,y,x+dx,y+dy);
	m_uFormat=TextFormat;
	m_Color=TextColor;
	m_dYard=TextYard;
}

void CText::SetTextFormat(UINT Format)
{
	m_uFormat=Format;
}

void CText::SetTextColor(COLORREF Color)
{
	m_Color=Color;
}

void CText::SetTextYard(double Yard)
{
	m_dYard=Yard;
}

void CText::Draw()
{
	if(m_nShow==SW_HIDE) return;

	if(m_Caption==NULL) return;
	LOGFONT logfont;
	memset(&logfont,0,sizeof(LOGFONT));
	logfont.lfHeight=(long)(-16*m_dYard);
	if(m_dYard>=1)
	{
		logfont.lfWeight=FW_BOLD ;
	}
	else
	{
		logfont.lfWeight=FW_NORMAL ;
	}
	wcscpy(logfont.lfFaceName,L"Tahoma");
	m_font=CreateFontIndirect( &logfont);

	HDC hdc;
	if(m_movable)
	{
		if (g_pDDSTmp1->GetDC(&hdc)==DD_OK)
		{
			SetBkMode(hdc,TRANSPARENT);
			::SetTextColor(hdc,m_Color);
			SelectObject(hdc,m_font);
			RECT rct={0};
			CopyRect(&rct,&m_rct);
			OffsetRect(&rct,0,m_rctOffset);
			DrawText( hdc,m_Caption,-1,&rct,m_uFormat );
			DeleteObject(m_font);
			g_pDDSTmp1->ReleaseDC(hdc);
		}
	}
	else
	{
		if (g_pDDSTmp1->GetDC(&hdc)==DD_OK)
		{
			SetBkMode(hdc,TRANSPARENT);
			::SetTextColor(hdc,m_Color);
			SelectObject(hdc,m_font);
			RECT rct={0};
			CopyRect(&rct,&m_rct);
			OffsetRect(&rct,0,m_rctOffset);
			DrawText( hdc,m_Caption,-1,&rct,m_uFormat );
			DeleteObject(m_font);
			g_pDDSTmp1->ReleaseDC(hdc);
		}
		if (g_pDDSTmp2->GetDC(&hdc)==DD_OK)
		{
			SetBkMode(hdc,TRANSPARENT);
			::SetTextColor(hdc,m_Color);
			SelectObject(hdc,m_font);
			RECT rct={0};
			CopyRect(&rct,&m_rct);
			OffsetRect(&rct,0,m_rctOffset);
			DrawText( hdc,m_Caption,-1,&rct,m_uFormat );
			DeleteObject(m_font);
			g_pDDSTmp2->ReleaseDC(hdc);
		}
	}
}

void CText::AddChar(TCHAR tch,UINT iMaxlen,bool bReDraw)
{
	UINT len=0;
	if(m_Caption!=NULL)
	{
		len=wcslen(m_Caption);
	}

	if(len<m_uMaxlen)
	{
		m_Caption[len]=tch;
		m_Caption[len+1]='\0';
	}
	else if(len<iMaxlen)
	{
		TCHAR *wzBuffer=(TCHAR *)malloc((iMaxlen+1)*sizeof(TCHAR));
		if(wzBuffer==NULL) return;
		m_uMaxlen=iMaxlen;
		memset(wzBuffer,0,(iMaxlen+1)*sizeof(TCHAR));
		if(m_Caption!=NULL)
		{
			wcscpy(wzBuffer,m_Caption);
			free(m_Caption);
		}
		m_Caption=wzBuffer;

		len=wcslen(m_Caption);
		m_Caption[len]=tch;
		m_Caption[len+1]='\0';
	}
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
}

void CText::DelChar(bool bReDraw)
{
	if(m_Caption==NULL)
		return;

	UINT len=wcslen(m_Caption);
	if(len>0)
	{
		m_Caption[len-1]='\0';
		if(bReDraw)
		{
			g_Page[g_stackPage.GetTop()].Draw();
			g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
		}
	}
}

bool CText::SetWindowsText(const TCHAR *WindowsText,bool bReDraw)
{
	bool bRet=true;
	if(m_Caption==NULL)
	{
		m_uMaxlen=wcslen(WindowsText);
		m_Caption=(TCHAR *)malloc((m_uMaxlen+1)*sizeof(TCHAR));
		if(m_Caption==NULL)
		{
			m_uMaxlen=0;
			bRet=false;
		}
		else
		{
			memset(m_Caption,0,(m_uMaxlen+1)*sizeof(TCHAR));
			wcscpy(m_Caption,WindowsText);
		}
	}
	else
	{
		if(wcslen(WindowsText)<=m_uMaxlen)
		{
			memset(m_Caption,0,m_uMaxlen);
			wcscpy(m_Caption,WindowsText);
		}
		else
		{
			free(m_Caption);
			m_uMaxlen=wcslen(WindowsText);
			m_Caption=(TCHAR *)malloc((m_uMaxlen+1)*sizeof(TCHAR));
			if(m_Caption==NULL)
			{
				m_uMaxlen=0;
				bRet=false;
			}
			else
			{
				memset(m_Caption,0,(m_uMaxlen+1)*sizeof(TCHAR));
				wcscpy(m_Caption,WindowsText);
			}
		}
	}
	if(bReDraw)	
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
	return bRet;
}

TCHAR *CText::GetWindowsText()
{
	if(m_Caption==NULL) return L"";
	return m_Caption;
}

void CText::GetCharMetrics(TCHAR ch, INT32 *charWidth)
{
	ABC abc;
	HDC hdc;
	HFONT font= GetFont();
	if(font == NULL)
	{
		LOGFONT logfont;
		memset(&logfont,0,sizeof(LOGFONT));
		logfont.lfHeight=(long)(-16*m_dYard);
		if(m_dYard>=1)
		{
			logfont.lfWeight=FW_BOLD ;
		}
		else
		{
			logfont.lfWeight=FW_NORMAL ;
		}
		wcscpy(logfont.lfFaceName,L"Tahoma");
		font=CreateFontIndirect( &logfont);
	}
	if(g_pDDSTmp2->GetDC(&hdc)==DD_OK)
	{
		SelectObject(hdc,font);
		GetCharABCWidths(hdc, ch, ch, &abc); 
		*charWidth= abc.abcA+abc.abcB+abc.abcC;
		g_pDDSTmp2->ReleaseDC(hdc);
	}
}
void CText::GetTextMetric(TEXTMETRIC *tm)
{
	HDC hdc;
	HFONT font= GetFont();
	if(font == NULL)
	{
		LOGFONT logfont;
		memset(&logfont,0,sizeof(LOGFONT));
		logfont.lfHeight=(long)(-16*m_dYard);
		if(m_dYard>=1)
		{
			logfont.lfWeight=FW_BOLD ;
		}
		else
		{
			logfont.lfWeight=FW_NORMAL ;
		}
		wcscpy(logfont.lfFaceName,L"Tahoma");
		font=CreateFontIndirect( &logfont);
	}
	if(g_pDDSTmp2->GetDC(&hdc)==DD_OK)
	{
		SelectObject(hdc,font);
		GetTextMetrics(hdc, tm); 
		g_pDDSTmp2->ReleaseDC(hdc);
	}
}
static UINT					m_HoldDownId;
static UINT					m_stopHoldDown;
void CALLBACK CButt::OnTimer2(HWND hwnd,UINT message,UINT iTimerID,DWORD dwTime)
{
	PostMessage(g_hWnd,WM_HOLDDOWNBUTTON,m_HoldDownId,0);
	if(m_stopHoldDown)
	{
		KillTimer(hwnd,iTimerID);
	}
}
void CALLBACK CButt::OnTimer1(HWND hwnd,UINT message,UINT iTimerID,DWORD dwTime)
{
	KillTimer(hwnd,iTimerID);
	if(!m_stopHoldDown)
	{
		SetTimer(NULL,NULL,100,OnTimer2);
	}
}
CButt::CButt()
{
	m_uID=NULL;
	m_eState=UP;
	m_eFlag = BUTTFLAG_NONE;
	m_pImageUp=NULL;
	m_pImageDown=NULL;
	m_pImageDisabled=NULL;
}

bool CButt::LoadButton(UINT ID,int x,int y ,int dx,int dy,int offsety,TCHAR *ResPath,TCHAR *ResFileUp,TCHAR *ResFileDown,TCHAR *ResFileDisabled,TCHAR *ResFileRemind)
{
	m_uID=ID;
	SetRect(&m_rct,x,y,x+dx,y+dy);
	m_rctOffset=offsety;

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=dy;
	g_ddsd.dwWidth=dx;

	if(ResFileUp!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileUp);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageUp,NULL)!=DD_OK)
				return false;

			DDCopyBitmap(m_pImageUp,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileDown!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileDown);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageDown,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageDown,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileDisabled!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileDisabled);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageDisabled,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageDisabled,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileRemind!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileRemind);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageRemind,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageRemind,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	return true;
}

bool CButt::LoadButton(UINT ID,int x,int y ,int dx,int dy,int offsety,HINSTANCE hUiDLL,TCHAR *ResFileUp,TCHAR *ResFileDown,TCHAR *ResFileDisabled,TCHAR *ResFileRemind)
{
	m_uID=ID;
	SetRect(&m_rct,x,y,x+dx,y+dy);
	m_rctOffset=offsety;

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=dy;
	g_ddsd.dwWidth=dx;

	if(ResFileUp!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFileUp);
		if (hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageUp,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageUp,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileDown!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFileDown);
		if (hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageDown,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageDown,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileDisabled!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFileDisabled);
		if (hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageDisabled,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageDisabled,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileRemind!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFileRemind);
		if (hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageRemind,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageRemind,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	return true;
}

void CButt::LButtonDown(POINT point,RECT moveRct)
{
	CSkin::LButtonDown();
	if(m_nShow==SW_HIDE) return;
	if (PtInRect(&m_rct,point))
	{
		if (m_eState==UP||m_eState==REMIND)
		{
			if(m_movable)
			{
				if(PtInRect(&moveRct,point))
				{
					m_eState=DOWN;
				}
			}
			else
			{
				m_eState=DOWN;
			}
		}
	}
	if(m_eState==DOWN)
	{
		SetTimer(NULL,NULL,500,OnTimer1);
		m_HoldDownId=m_uID;
		m_stopHoldDown=0;
	}
}

void CButt::MouseMove(POINT point,EMoveDirection direction,POINT lastMovePoint,RECT moveRect)
{
	RECT rct;
	bool flag1=IntersectRect(&rct,&m_rct,&moveRect);
	CSkin::MouseMove(point,direction,lastMovePoint);
	bool flag2=IntersectRect(&rct,&m_rct,&moveRect);
	if(flag1&&!flag2)
	{
		if(direction==HORIZONTAL)
		PostMessage(g_hWnd,WM_BUTTONMOVEOUT,m_uID,m_rct.left-moveRect.left);
		else
		PostMessage(g_hWnd,WM_BUTTONMOVEOUT,m_uID,m_rct.top-moveRect.top);
	}
	else if(!flag1&&flag2)
	{
		if(direction==HORIZONTAL)
		PostMessage(g_hWnd,WM_BUTTONMOVEIN,m_uID,m_rct.left-moveRect.left);
		else
		PostMessage(g_hWnd,WM_BUTTONMOVEIN,m_uID,m_rct.top-moveRect.top);
	}
}

void CButt::LButtonUp(POINT point,RECT moveRct)
{
	m_stopHoldDown=1;
	if(m_nShow==SW_HIDE) return;
	if (m_eState==DOWN)
	{
		m_eState=UP;
		if (PtInRect(&m_lastDownRCT,point))
		{
			if(m_movable)
			{
				if(PtInRect(&moveRct,point))
				{
					if(m_rct.left!=m_lastDownRCT.left)
					{
						if(m_rct.left-m_lastDownRCT.left>-VIBRATEPIX&&m_rct.left-m_lastDownRCT.left<VIBRATEPIX)
							PostMessage(g_hWnd,WM_CLICKBUTTON,m_uID,0);
					}
					else if(m_rct.top!=m_lastDownRCT.top)
					{
						if(m_rct.top-m_lastDownRCT.top>-VIBRATEPIX&&m_rct.top-m_lastDownRCT.top<VIBRATEPIX)
						PostMessage(g_hWnd,WM_CLICKBUTTON,m_uID,0);
					}
					else
					{
						PostMessage(g_hWnd,WM_CLICKBUTTON,m_uID,0);
					}
					
				}
			}
			else
			{
				PostMessage(g_hWnd,WM_CLICKBUTTON,m_uID,0);
			}
		}
	}
}

EButtonState CButt::GetState()
{
	return m_eState;
}

void CButt::SetState(EButtonState State,bool bReDraw)
{
	if(m_eState!=State)
	{
		if(State == REMIND) m_eFlag = BUTTFLAG_HIGHLIGHT;
		else		m_eFlag = BUTTFLAG_NONE;
		m_eState=State;
		if(bReDraw)
		{
			g_Page[g_stackPage.GetTop()].Draw();
			g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
		}
	}
}

void CButt::Draw()
{
	if(m_nShow==SW_HIDE) return;

	LPDIRECTDRAWSURFACE	pImage=NULL;
	if (UP==m_eState)
	{
		pImage=m_pImageUp;
	}
	else if (DOWN==m_eState)
	{
		pImage=m_pImageDown;
	}
	else if (DISABLED==m_eState)
	{
		pImage=m_pImageDisabled;
	}
	else if (REMIND==m_eState)
	{
		pImage=m_pImageRemind;
	}
	if(pImage==NULL)
	{
		// Draw Text
		CText::Draw();
		return;
	}

	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	while (true)
	{
		if(m_movable)
		{
			/*RECT InterRct1;
			IntersectRect(&InterRct1,&m_rct,&moveRct);
			RECT InterRct2;
			SetRect(&InterRct2,InterRct1.left-m_rct.left,InterRct1.top-m_rct.top,InterRct1.right-m_rct.left,InterRct1.bottom-m_rct.top);
			hRet=g_pDDSTmp1->Blt(&InterRct1,pImage,&InterRct2,DDBLT_ROP,&ddbltfx);*/
			hRet=g_pDDSTmp1->Blt(&m_rct,pImage,NULL,DDBLT_ROP,&ddbltfx);
		}
		else
		{
			hRet=g_pDDSTmp1->Blt(&m_rct,pImage,NULL,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSTmp2->Blt(&m_rct,pImage,NULL,DDBLT_ROP,&ddbltfx);
		}
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
	// Draw Text
	CText::Draw();
}

CSwitch::CSwitch()
{
	m_sState=OFF;
}
void CSwitch::LButtonUp(POINT point,RECT moveRct)
{
	m_stopHoldDown=1;
	if(m_nShow==SW_HIDE) return;
	if (m_eState==DOWN)
	{
		m_eState=UP;
		if (PtInRect(&m_lastDownRCT,point))
		{
			if(m_movable)
			{
				if(PtInRect(&moveRct,point))
				{
					if(m_rct.left!=m_lastDownRCT.left)
					{
						if(m_rct.left-m_lastDownRCT.left>-VIBRATEPIX&&m_rct.left-m_lastDownRCT.left<VIBRATEPIX)
						{
							if(m_sState==ON) m_sState=OFF;
							else m_sState=ON;
							PostMessage(g_hWnd,WM_SWITCHCHANGE,m_uID,m_sState);
						}
						
					}
					else if(m_rct.top!=m_lastDownRCT.top)
					{
						if(m_rct.top-m_lastDownRCT.top>-VIBRATEPIX&&m_rct.top-m_lastDownRCT.top<VIBRATEPIX)
						{
							if(m_sState==ON) m_sState=OFF;
							else m_sState=ON;
							PostMessage(g_hWnd,WM_SWITCHCHANGE,m_uID,m_sState);
						}
					}
					else
					{
						if(m_sState==ON) m_sState=OFF;
						else m_sState=ON;
						PostMessage(g_hWnd,WM_SWITCHCHANGE,m_uID,m_sState);
					}

				}
			}
			else
			{
				if(m_sState==ON) m_sState=OFF;
				else m_sState=ON;
				PostMessage(g_hWnd,WM_SWITCHCHANGE,m_uID,m_sState);
			}
		}
	}

}
void CSwitch::Draw()
{
	if(m_nShow==SW_HIDE) return;

	LPDIRECTDRAWSURFACE	pImage=NULL;
	if (UP==m_eState)
	{
		if(m_sState==OFF)
			pImage=m_pImageUp;
		else
			pImage=m_pImageDown;
	}
	else if (DOWN==m_eState)
	{
		if(m_sState==OFF)
			pImage=m_pImageDown;
		else
			pImage=m_pImageUp;
	}
	else if (DISABLED==m_eState)
	{
		pImage=m_pImageDisabled;
	}
	else if (REMIND==m_eState)
	{
		pImage=m_pImageRemind;
	}
	if(pImage==NULL)
	{
		// Draw Text
		CText::Draw();
		return;
	}

	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	while (true)
	{
		if(m_movable)
		{
			/*RECT InterRct1;
			IntersectRect(&InterRct1,&m_rct,&moveRct);
			RECT InterRct2;
			SetRect(&InterRct2,InterRct1.left-m_rct.left,InterRct1.top-m_rct.top,InterRct1.right-m_rct.left,InterRct1.bottom-m_rct.top);
			hRet=g_pDDSTmp1->Blt(&InterRct1,pImage,&InterRct2,DDBLT_ROP,&ddbltfx);*/
			hRet=g_pDDSTmp1->Blt(&m_rct,pImage,NULL,DDBLT_ROP,&ddbltfx);
		}
		else
		{
			hRet=g_pDDSTmp1->Blt(&m_rct,pImage,NULL,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSTmp2->Blt(&m_rct,pImage,NULL,DDBLT_ROP,&ddbltfx);
		}
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
	// Draw Text
	CText::Draw();
}

void CSwitch::SetSwitchState(ESwitchState State,bool bReDraw)
{
	if(m_sState!=State)
	{
		m_sState=State;
		if(bReDraw)
		{
			g_Page[g_stackPage.GetTop()].Draw();
			g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
		}
	}
}
ESwitchState CSwitch::GetSwitchState()
{
	return m_sState;
}
CPict::CPict()
{
	m_index=0;
	m_pImageUp=NULL;
}

bool CPict::LoadPicture(int x,int y,int dx,int dy,int ddx,TCHAR *ResPath,TCHAR *ResFile)
{
	SetRect(&m_rct,x,y,x+dx,y+dy);

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwWidth=ddx;
	g_ddsd.dwHeight=dy;

	if(ResFile!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFile);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(!hbm)
			return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImageUp,NULL)!=DD_OK)
			return false;
		DDCopyBitmap(m_pImageUp,hbm,0,0,ddx,dy);
		DeleteObject(hbm);
	}
	return true;
}

bool CPict::LoadPicture(int x,int y,int dx,int dy,int ddx,HINSTANCE hUiDLL,TCHAR *ResFile)
{
	SetRect(&m_rct,x,y,x+dx,y+dy);

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwWidth=ddx;
	g_ddsd.dwHeight=dy;

	if(ResFile!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFile);
		if (hbm==NULL)
			return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImageUp,NULL)!=DD_OK)
			return false;
		DDCopyBitmap(m_pImageUp,hbm,0,0,ddx,dy);
		DeleteObject(hbm);
	}
	return true;
}

void CPict::SetIndex(int nIndex,bool bReDraw)
{
	m_index=nIndex;
	if(bReDraw)
	{	
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	}
}

void CPict::Draw()
{
	if(m_nShow==SW_HIDE) return;
	if(m_pImageUp==NULL) return;

	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	while (true)
	{
		RECT rct={0};
		SetRect(&rct,m_index*(m_rct.right-m_rct.left),0,(m_index+1)*(m_rct.right-m_rct.left),m_rct.bottom-m_rct.top);
		if(m_movable)
		{
			/*RECT InterRct1;
			IntersectRect(&InterRct1,&m_rct,&moveRct);
			RECT InterRct2;
			SetRect(&InterRct2,rct.left+(InterRct1.left-m_rct.left),rct.top+(InterRct1.top-m_rct.top),rct.right+(InterRct1.right-m_rct.right),rct.bottom+(InterRct1.bottom-m_rct.bottom));
			hRet=g_pDDSTmp1->Blt(&InterRct1,m_pImageUp,&InterRct2,DDBLT_ROP,&ddbltfx);*/
			hRet=g_pDDSTmp1->Blt(&m_rct,m_pImageUp,&rct,DDBLT_ROP,&ddbltfx);
		}
		else
		{
			hRet=g_pDDSTmp1->Blt(&m_rct,m_pImageUp,&rct,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSTmp2->Blt(&m_rct,m_pImageUp,&rct,DDBLT_ROP,&ddbltfx);
		}
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
}

CProg::CProg()
{
	m_nPosition=0;
	m_nWidthMark=0;
	m_nHeigthMark=0;
	SetRectEmpty(&m_rcFore);
	m_pImageBack=NULL;
	m_pImageBack=NULL;
	m_pImageMark=NULL;
}
bool CProg::LoadProgress(UINT ID,int x,int y,int dx ,int dy,int dxmark,int dymark,TCHAR *ResPath,TCHAR *ResFileBack,TCHAR *ResFileFore,TCHAR *ResFileMark)
{
	m_uID=ID;
	SetRect(&m_rct,x,y,x+dx,y+dy);
	m_nHeigthMark=dymark;
	m_nWidthMark=dxmark;

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=dy;
	g_ddsd.dwWidth=dx;

	if(ResFileBack!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileBack);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageBack,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageBack,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileFore!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileFore);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageBefore,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageBefore,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileMark!=NULL)
	{
		g_ddsd.dwHeight=dymark;
		g_ddsd.dwWidth=dxmark;

		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFileMark);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if(hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageMark,NULL)!=DD_OK)
				return false;
			DDCopyBitmap(m_pImageMark,hbm,0,0,dxmark,dymark);
			DeleteObject(hbm);
		}
	}
	return true;
}
bool CProg::LoadProgress(UINT ID,int x,int y,int dx ,int dy,int dxmark,int dymark,HINSTANCE hUiDLL,TCHAR *ResFileBack,TCHAR *ResFileFore,TCHAR *ResFileMark)
{
	m_uID=ID;
	SetRect(&m_rct,x,y,x+dx,y+dy);
	m_nHeigthMark=dymark;
	m_nWidthMark=dxmark;

	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=dy;
	g_ddsd.dwWidth=dx;

	if(ResFileBack!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFileBack);
		if (hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageBack,NULL)!=DD_OK)
				return false;

			DDCopyBitmap(m_pImageBack,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
		
	}
	if(ResFileFore!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFileFore);
		if (hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageBefore,NULL)!=DD_OK)
				return false;

			DDCopyBitmap(m_pImageBefore,hbm,0,0,dx,dy);
			DeleteObject(hbm);
		}
	}
	if(ResFileMark!=NULL)
	{
		g_ddsd.dwHeight=dymark;
		g_ddsd.dwWidth=dxmark;
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFileMark);
		if (hbm)
		{
			if(g_pDD->CreateSurface(&g_ddsd,&m_pImageMark,NULL)!=DD_OK)
				return false;

			DDCopyBitmap(m_pImageMark,hbm,0,0,dxmark,dymark);
			DeleteObject(hbm);
		}
	}
	return true;
}
void CProg::SetPos(int nPos,bool bReDraw)
{
	m_nPosition=nPos;
	if(m_nPosition>100) 
	{
		m_nPosition=100;
	}
	else if(m_nPosition<0)
	{
		m_nPosition=0;
	}
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].Draw();
		RECT rct={0};
		SetRect(&rct,m_rct.left-m_nWidthMark/2,m_rct.top-m_nHeigthMark/2,m_rct.right+m_nWidthMark/2,m_rct.bottom+m_nHeigthMark/2);
		g_Page[g_stackPage.GetTop()].UpdatePage(&rct);
	}
	//PostMessage(g_hWnd,WM_PROGPOSCHANGE,m_nPosition,0);
}

int CProg::GetPos()
{
	return m_nPosition;
}

void CProg::MouseMove(POINT point)
{
	if(m_nShow==SW_HIDE) return;
	if(!PtInRect(&m_rct,point)) return;
	if(m_rct.right==m_rct.left) return;
	m_nPosition=(point.x-m_rct.left)*100/(m_rct.right-m_rct.left);
	g_Page[g_stackPage.GetTop()].Draw();
	RECT rct={0};
	SetRect(&rct,m_rct.left-m_nWidthMark/2,m_rct.top-m_nHeigthMark/2,m_rct.right+m_nWidthMark/2,m_rct.bottom+m_nHeigthMark/2);
	g_Page[g_stackPage.GetTop()].UpdatePage(&rct);
	PostMessage(g_hWnd,WM_PROGPOSCHANGE,m_uID,m_nPosition);
}
void CProg::Draw()
{
	if(m_nShow==SW_HIDE) return;
	if(m_pImageBefore==NULL&&m_pImageBack==NULL) return;

	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	while (true)
	{
		if(m_pImageBack!=NULL)
		{
			hRet=g_pDDSTmp2->Blt(&m_rct,m_pImageBack,NULL,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSTmp1->Blt(&m_rct,m_pImageBack,NULL,DDBLT_ROP,&ddbltfx);
		}
		if(m_pImageBefore!=NULL)
		{
			RECT rct={0};
			SetRect(&rct,0,0,m_nPosition*(m_rct.right-m_rct.left)/100,m_rct.bottom-m_rct.top);
			RECT rct2={0};
			SetRect(&rct2,m_rct.left,m_rct.top,m_rct.left+m_nPosition*(m_rct.right-m_rct.left)/100,m_rct.bottom);
			hRet=g_pDDSTmp2->Blt(&rct2,m_pImageBefore,&rct,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSTmp1->Blt(&rct2,m_pImageBefore,&rct,DDBLT_ROP,&ddbltfx);
		}
		if(m_pImageMark!=NULL)
		{
			int posx=m_rct.left+m_nPosition*(m_rct.right-m_rct.left)/100;
			int posy=(m_rct.top+m_rct.bottom)/2;
			RECT rct={0};
			SetRect(&rct,posx-m_nWidthMark/2,posy-m_nHeigthMark/2,posx+m_nWidthMark/2,posy+m_nHeigthMark/2);
			hRet=g_pDDSTmp2->Blt(&rct,m_pImageMark,NULL,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSTmp1->Blt(&rct,m_pImageMark,NULL,DDBLT_ROP,&ddbltfx);
		}
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
}



CPhoto::CPhoto()
{
	m_pImage=NULL;
	m_pImageChange=NULL;
	m_angle=0;
	m_zoom=1;
	SetRectEmpty(&m_rcPict);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}
CPhoto::~CPhoto()
{
	CoUninitialize();
}
bool CPhoto::LoadPhoto(int x,int y,int dx ,int dy)
{
	SetRect(&m_rct,x,y,x+dx,y+dy);
	if(!m_pImage||!m_pImageChange)
	{
		g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
		g_ddsd.dwWidth=dx;
		g_ddsd.dwHeight=dy;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImage,NULL)!=DD_OK) return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImageChange,NULL)!=DD_OK) return false;
	}
}
bool CPhoto::LoadPicture(TCHAR *File)
{
	m_angle=0;
	SetCurrentFile(File);

	HBITMAP hbm=LoadImageFromFile(File);
	if (hbm==NULL) return false;
	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);
	int w,h;
	ZoomRect(&w,&h,bm.bmWidth,bm.bmHeight,&m_rct);
	SetRect(&m_rcPict,((m_rct.right-m_rct.left)-w)/2,((m_rct.bottom-m_rct.top)-h)/2,((m_rct.right-m_rct.left)+w)/2,((m_rct.bottom-m_rct.top)+h)/2);
	OffsetRect(&m_rcPict,m_rct.left,m_rct.top);

	DDCopyBitmap(m_pImage,hbm,0,0,bm.bmWidth,bm.bmHeight);
	DDCopyBitmap(m_pImageChange,hbm,0,0,bm.bmWidth,bm.bmHeight);
	DeleteObject(hbm);
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
	return true;
}
void CPhoto::Draw()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	while (true)
	{
		RECT rct={0};
		SetRect(&rct,0,0,m_rcPict.right-m_rcPict.left,m_rcPict.bottom-m_rcPict.top);
		hRet=g_pDDSTmp1->Blt(&m_rcPict,m_pImageChange,&rct,DDBLT_ROP,&ddbltfx);
		hRet=g_pDDSTmp2->Blt(&m_rcPict,m_pImageChange,&rct,DDBLT_ROP,&ddbltfx);
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
}
void CPhoto::Rotate(float angle)
{
	m_angle+=angle;
	HBITMAP hbm = NULL;
	IBasicBitmapOps	*oper;
	IImagingFactory *pImgFactory = NULL;
	IImage *pImageBmp = NULL; 
	IBitmapImage *rotebitmap = NULL; 
	if (SUCCEEDED(CoCreateInstance (CLSID_ImagingFactory, NULL,CLSCTX_INPROC_SERVER, IID_IImagingFactory, (void **)&pImgFactory)))
	{
		if(SUCCEEDED(pImgFactory->CreateImageFromFile(m_Caption, &pImageBmp)))
			if(SUCCEEDED(pImgFactory->CreateBitmapFromImage(pImageBmp, 0, 0, PixelFormatDontCare, InterpolationHintDefault, &rotebitmap)))
				if(SUCCEEDED(rotebitmap->QueryInterface(IID_IBasicBitmapOps, (void **) &oper)))
				{
					rotebitmap->Release();
					if(SUCCEEDED(oper->Rotate(m_angle, InterpolationHintDefault, &rotebitmap)))
					{
						if(SUCCEEDED(rotebitmap->QueryInterface(IID_IImage,(void **) &pImageBmp)))
						{
							ImageInfo imageInfo;
							if (SUCCEEDED(pImageBmp->GetImageInfo(&imageInfo)))
							{
								HDC hdc = ::GetDC(g_hWnd);
								HDC dcBitmap=CreateCompatibleDC(hdc);;
								hbm = CreateCompatibleBitmap(hdc, imageInfo.Width, imageInfo.Height);
								SelectObject(dcBitmap, hbm);
								RECT rct={0,0,imageInfo.Width,imageInfo.Height};
								pImageBmp->Draw(dcBitmap,&rct,NULL);  
								DeleteDC(dcBitmap);
								pImageBmp->Release();
							}
						}
					}
					oper->Release();
				}
				rotebitmap->Release();
				pImgFactory->Release();
	}


	if (hbm==NULL) return;
	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);
	int w,h;
	ZoomRect(&w,&h,bm.bmWidth,bm.bmHeight,&m_rct);
	SetRect(&m_rcPict,((m_rct.right-m_rct.left)-w)/2,((m_rct.bottom-m_rct.top)-h)/2,((m_rct.right-m_rct.left)+w)/2,((m_rct.bottom-m_rct.top)+h)/2);
	OffsetRect(&m_rcPict,m_rct.left,m_rct.top);
	DDCopyBitmap(m_pImageChange,hbm,0,0,bm.bmWidth,bm.bmHeight);
	DeleteObject(hbm);

	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
}
void CPhoto::Zoom(double zoom)
{
	m_zoom*=zoom;
	HBITMAP hbm=LoadImageFromFile(m_Caption);
	if (hbm==NULL) return;
	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);
	int w,h;
	RECT rct;
	RECT rct1;
	ZoomRect(&w,&h,bm.bmWidth,bm.bmHeight,&m_rct);
	SetRect(&rct,((m_rct.right-m_rct.left)-w)/2,((m_rct.bottom-m_rct.top)-h)/2,((m_rct.right-m_rct.left)+w)/2,((m_rct.bottom-m_rct.top)+h)/2);
	OffsetRect(&rct,m_rct.left,m_rct.top);
	SetRect(&rct1,(rct.left+rct.right)/2-(rct.right-rct.left)/2*m_zoom,(rct.top+rct.bottom)/2-(rct.bottom-rct.top)/2*m_zoom,(rct.left+rct.right)/2+(rct.right-rct.left)/2*m_zoom,(rct.top+rct.bottom)/2+(rct.bottom-rct.top)/2*m_zoom);
	IntersectRect(&m_rcPict,&rct1,&m_rct);
	//rct一开始显示的矩形，rct1放大后看到的矩形，m_rcPict放大后看到的矩形。
	//成比例				可能不成比例
	//rct1缩放多少比例相对原大小，m_rcPict就缩小多少比例，缩小后的矩形进行拷贝。

	int width,hight;
	width=bm.bmWidth*(m_rcPict.right-m_rcPict.left)/(rct1.right-rct1.left);
	hight=bm.bmHeight*(m_rcPict.bottom-m_rcPict.top)/(rct1.bottom-rct1.top);

	//DDCopyBitmap(m_pImage,hbm,0,0,bm.bmWidth,bm.bmHeight);
	DDCopyBitmap(m_pImageChange,hbm,bm.bmWidth/2-width/2,bm.bmHeight/2-hight/2,width,hight);
	DeleteObject(hbm);
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);

	
}
bool CPhoto::SetCurrentFile(TCHAR *WindowsText)
{
	bool bRet=true;
	if(m_Caption==NULL)
	{
		m_uMaxlen=wcslen(WindowsText);
		m_Caption=(TCHAR *)malloc((m_uMaxlen+1)*sizeof(TCHAR));
		if(m_Caption==NULL)
		{
			m_uMaxlen=0;
			bRet=false;
		}
		else
		{
			memset(m_Caption,0,(m_uMaxlen+1)*sizeof(TCHAR));
			wcscpy(m_Caption,WindowsText);
		}
	}
	else
	{
		if(wcslen(WindowsText)<=m_uMaxlen)
		{
			memset(m_Caption,0,m_uMaxlen);
			wcscpy(m_Caption,WindowsText);
		}
		else
		{
			free(m_Caption);
			m_uMaxlen=wcslen(WindowsText);
			m_Caption=(TCHAR *)malloc((m_uMaxlen+1)*sizeof(TCHAR));
			if(m_Caption==NULL)
			{
				m_uMaxlen=0;
				bRet=false;
			}
			else
			{
				memset(m_Caption,0,(m_uMaxlen+1)*sizeof(TCHAR));
				wcscpy(m_Caption,WindowsText);
			}
		}
	}
	return bRet;
}

void CPhoto::ZoomRect(int *destW,int *destH,LONG srcW,LONG srcH,RECT *rct)
{
	if(srcW>(m_rct.right-m_rct.left)&&srcH>(m_rct.bottom-m_rct.top))
	{
		if((float)srcW/(m_rct.right-m_rct.left)>(float)srcH/(m_rct.bottom-m_rct.top))
		{
			*destW=(m_rct.right-m_rct.left);
			*destH=(float)srcH*(m_rct.right-m_rct.left)/srcW;
		}
		else
		{
			*destH=(m_rct.bottom-m_rct.top);
			*destW=(float)srcW*(m_rct.bottom-m_rct.top)/srcH;
		}
	}
	else if(srcW>(m_rct.right-m_rct.left))
	{
		*destW=(m_rct.right-m_rct.left);
		*destH=(float)srcH*(m_rct.right-m_rct.left)/srcW;
	}
	else if(srcH>(m_rct.bottom-m_rct.top))
	{
		*destH=(m_rct.bottom-m_rct.top);
		*destW=(float)srcW*(m_rct.bottom-m_rct.top)/srcH;
	}
	else
	{
		*destW=srcW;
		*destH=srcH;
	}

};
HRESULT CPhoto::DDCopyBitmap(LPDIRECTDRAWSURFACE pdds, HBITMAP hbm, int x, int y,int dx, int dy)
{
	HDC                     hdcImage;
	HDC                     hdc;
	BITMAP                  bm;
	DDSURFACEDESC			ddsd;
	HRESULT                 hr;

	if (hbm == NULL || pdds == NULL) return E_FAIL;
	pdds->Restore();
	hdcImage = CreateCompatibleDC(NULL);
	if (!hdcImage)
		return E_FAIL;
	SelectObject(hdcImage, hbm);

	GetObject(hbm, sizeof(bm), &bm);
	dx = dx == 0 ? bm.bmWidth : dx;
	dy = dy == 0 ? bm.bmHeight : dy;

	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	pdds->GetSurfaceDesc(&ddsd);

	if ((hr = pdds->GetDC(&hdc)) == DD_OK)
	{
		StretchBlt(hdc, 0, 0, (m_rcPict.right-m_rcPict.left), (m_rcPict.bottom-m_rcPict.top), hdcImage, x, y, dx, dy, SRCCOPY);
		pdds->ReleaseDC(hdc);
	}
	DeleteDC(hdcImage);
	return hr;
}
///////////////////////
//class CEdit
//constructor & destructor
CEdit::CEdit(UINT Options)
{
	m_options = Options; 
	m_length = 0;
	m_cursor = 0;
	m_length_show = 0;
	m_position_show = 0;
	m_line_count = 0;
	m_text = new CText();
	m_cursor_created = false;
	m_font_changed = false;
	m_pImageBackground = NULL;
}
CEdit::~CEdit()
{
	//HideCaret(g_hWnd);
	if(m_text != NULL)
	{
		delete m_text;
	}
	if(m_buffer != NULL)
	{
		delete [] m_buffer;
	}
	if(m_buffer_show != NULL)
	{
		delete [] m_buffer_show;
	}
		
}

bool CEdit::IsShowMax()
{
	if(m_length  <= 0) return false;
	INT32 stringWidth = 0;
	INT32 curPos = 0;
	INT32 width = 0;
	while((stringWidth <= (m_rcEdit.right - m_rcEdit.left)) && (curPos < m_length))
	{
		m_text->GetCharMetrics(m_buffer[curPos],&width);
		stringWidth+=width;
		curPos++;
	}
	if(stringWidth > (m_rcEdit.right - m_rcEdit.left)) return true;
	else return false;
}

INT32 CEdit::GetEditWidth(INT32 position, EDIT_DIRECTION direction)
{
	INT32	stringWidth = 0;
	INT32	width = 0;
	INT32	len = 0;
	INT32	count;
	count = position;
	if(direction==EDIT_MOVEDOWN)
	{
		while((stringWidth <= (m_rcEdit.right - m_rcEdit.left)) && count >0 )
		{
			m_text->GetCharMetrics(m_buffer[count--],&width);
			stringWidth += width;
			len++;
		}
	}
	else if(direction==EDIT_MOVEUP)
	{
		while((stringWidth <= (m_rcEdit.right - m_rcEdit.left)) && count < m_length)
		{
			m_text->GetCharMetrics(m_buffer[count++],&width);
			stringWidth += width;
			len++;
		}
	}
	if(stringWidth <= (m_rcEdit.right - m_rcEdit.left))
	{
		return len;
	}
	else
	{
		return (len-1);
	}
}
//pos can be as index of array
//showLength need minus 1
void CEdit::GetShowLength(INT32 *len, INT32 *pos)
{
	if(len == NULL || pos == NULL)	return;
	INT32 span;
	INT32 showLength=0;
	INT32 stringWidthPre = 0;
	INT32 stringWidthLast = 0;
	INT32 count=0;
	INT32 countRight=0;
	INT32 width = 0;
	INT32 lenRight=0;
	bool showMax=IsShowMax();
	if(!showMax)
	{
		*pos=0;
		*len=m_length;
	}
	else
	{
		span= m_length_show-m_cursor;
		countRight = m_length_old-m_position_show-m_length_show;
		lenRight = (m_position_show+m_length_show >m_length)?m_length:(m_position_show+m_length_show);
		for(INT32 i = m_cursor+m_position_show;i < lenRight; i++)
		{
			m_text->GetCharMetrics(m_buffer[i],&width);
			stringWidthLast += width;
			showLength++;
		}
		count = m_cursor+m_position_show-1;
		while(stringWidthPre <= (m_rcEdit.right - m_rcEdit.left - stringWidthLast))
		{
			m_text->GetCharMetrics(m_buffer[count--],&width);
			stringWidthPre += width;
			showLength++;
		}
		*pos = m_length - showLength + 1-countRight;
		if(*pos <= 0) *pos = 0;
		*len = showLength-1;
	}

}
void CEdit::GetLineRow(INT32 position, INT32 *line, INT32 *row)
{
	if( line==NULL || row == NULL || position > m_length)		return;
	if(m_multiline)
	{
		INT32 i = 0;
		INT32 lineLen = 0;
		INT32 lines = 0;
		if(position == 0)
		{
			*line = 1;
			*row = 0;
			return;
		}
		while(i < position)
		{

			lineLen = GetEditWidth(i, EDIT_MOVEUP);
			lines++;
			i += lineLen;
		}
		*line = lines;
		if(i > position)
		{
			*row = position + lineLen - i;
		}
		else
		{
			*row = lineLen;
		}
	}
}

void CEdit::SetShowBuf()
{
	if(m_multiline)
	{
		INT32 pos = 0;
		INT32 posShow=0;
		INT32 lineLen;
		m_line_count = 1;
		while(pos < m_length)
		{

			lineLen = GetEditWidth(pos, EDIT_MOVEUP);
			_tcsncpy(m_buffer_show+posShow, m_buffer+pos, lineLen);
			pos += lineLen;
			posShow+=lineLen;
			if(pos < m_length)
			{
				m_buffer_show[posShow++] = _T('\n');
				m_line_count++;
			}
		}
		m_length_show = m_length+m_line_count-1;
	}
	else
	{
		bool showMax;
		showMax = IsShowMax();
		if(!showMax)
		{
			_tcscpy(m_buffer_show, m_buffer);
			m_length_show = m_length;
			m_position_show = 0;
		}
		else
		{
			GetShowLength(&m_length_show, &m_position_show);
			_tcsncpy(m_buffer_show, m_buffer+m_position_show, m_length_show);
		}
	}
	m_buffer_show[m_length_show] = _T('\0');
}
void CEdit::SetText(TCHAR *Text)
{
	if(Text == NULL) return;
	bool showMax;
	UINT32 len = _tcslen(Text);
	m_length_old = m_length;
	m_length = (len < m_length_max)?len:m_length_max;
	_tcsncpy(m_buffer, Text, m_length);	
	m_buffer[m_length] =  _T('\0');
	if(m_multiline)
	{
		m_cursor = m_length-1;
	}
	else
	{
		showMax = IsShowMax();
		if(showMax)
		{
			m_cursor = GetEditWidth(m_length-1, EDIT_MOVEDOWN);

		}
		else
		{
			m_cursor = m_length;
		}
	}
	SetShowBuf();
	m_text->SetWindowsText(m_buffer_show);
	ShowWindowCursor(false);

}

TCHAR* CEdit::GetText()
{
	if(m_buffer == NULL)	return _T('\0');
	return m_buffer;
}

void CEdit::HideWindowCursor() {HideCaret(g_hWnd);}
void CEdit::ShowWindowCursor(bool show)
{
	INT32 width=0;
	INT32 height = 0;
	INT32 stringWidth=0;
	TEXTMETRIC tm;
	m_text->GetTextMetric(&tm);
	if(!m_cursor_created || m_font_changed)
	{
		CreateCaret(g_hWnd,(HBITMAP) NULL, 0, tm.tmHeight);
		m_cursor_created = true;
		m_font_changed = false;
	}
	if(m_multiline)
	{
		INT32 lines = 0;
		INT32 rows = 0;
		m_length_show= m_length+m_line_count-1;
		if(m_cursor > m_length_show)		m_cursor=m_length_show;
		GetLineRow(m_cursor, &lines, &rows);
		for(INT32 i=m_cursor-rows; i<m_cursor;i++)
		{
			m_text->GetCharMetrics(m_buffer[i],&width);
			stringWidth+=width;
			if(stringWidth>(m_rcEdit.right-m_rcEdit.left+5))
			{
				stringWidth-=width;
				break;
			}
		}
		height = (lines-1)*tm.tmHeight;
	}
	else
	{
		if(m_cursor > m_length_show) m_cursor = m_length_show;
		for(INT32 i=m_position_show; i< m_cursor+m_position_show;i++)
		{
			m_text->GetCharMetrics(m_buffer[i],&width);
			stringWidth+=width;
			if(stringWidth>(m_rcEdit.right-m_rcEdit.left+5))
			{
				stringWidth-=width;
				break;
			}

		}
	}
	if(m_multiline)
	{
		RECT rect;
		GetWindowRect(g_hWnd,&rect);
		SetCaretPos(m_rcEdit.left+stringWidth, m_rcEdit.top+height-rect.top);
	}
	else
	{
		RECT rect;
		GetWindowRect(g_hWnd,&rect);
		height = (m_rcEdit.bottom - m_rcEdit.top)/2 - tm.tmHeight/2-rect.top;
		SetCaretPos(m_rcEdit.left+stringWidth, m_rcEdit.top+height);
	}

	if(show)	ShowCaret(g_hWnd);
}

void CEdit::SetTextColor(COLORREF Color)
{
	m_text->SetTextColor(Color);
}

void CEdit::SetTextYard(double Yard)
{
	m_text->SetTextYard(Yard);
	m_font_changed = true;
}
bool CEdit::LoadBackground(int x,int y,int dx,int dy,TCHAR *ResPath,TCHAR *ResFile)
{
	if(ResPath == NULL || ResFile == NULL )	return false;
	SetRect(&m_rcBackground, x, y, x+dx, y+dy);
	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=dy;
	g_ddsd.dwWidth=dx;

	TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
	wcscpy(Res,ResPath);
	wcscat(Res,ResFile);
	HBITMAP hbm=SHLoadDIBitmap(Res);
	if(hbm)
	{
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImageBackground,NULL)!=DD_OK)
			return false;

		DDCopyBitmap(m_pImageBackground,hbm,0,0,dx,dy);
		DeleteObject(hbm);
	}
	return true;
}

bool CEdit::LoadBackground(int x,int y,int dx,int dy,HINSTANCE hUiDLL,TCHAR *ResFile)
{
	if(ResFile == NULL)	return false;
	SetRect(&m_rcBackground, x, y, x+dx, y+dy);
	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwWidth=dx;
	g_ddsd.dwHeight=dy;


	HBITMAP hbm=LoadBitmap(hUiDLL,ResFile);
	if (hbm==NULL)	return false;
	if(g_pDD->CreateSurface(&g_ddsd,&m_pImageBackground,NULL)!=DD_OK)	return false;
	DDCopyBitmap(m_pImageBackground,hbm,0,0,dx,dy);
	DeleteObject(hbm);
	return true;
}

void CEdit::Draw()
{
	HDC hdc;
	if(m_pImageBackground == NULL)
	{

		if(m_edit_color != (COLORREF)(-1) && g_pDDSTmp2->GetDC(&hdc)==DD_OK)
		{
		
			SelectObject(hdc,GetStockObject(WHITE_PEN));
			HBRUSH hbrush, hbrushOld;
			hbrush = CreateSolidBrush(m_edit_color);
			//SelectObject(hdc, GetStockObject(WHITE_BRUSH)); 
			hbrushOld = (HBRUSH)SelectObject(hdc, hbrush);
			Rectangle(hdc, m_rcEdit.left, m_rcEdit.top, m_rcEdit.right, m_rcEdit.bottom);
			SelectObject(hdc, hbrushOld);
			DeleteObject(hbrush);		
			g_pDDSTmp2->ReleaseDC(hdc);

		}
	}
	else
	{
		HRESULT hRet;
		DDBLTFX ddbltfx;
		memset(&ddbltfx,0,sizeof(ddbltfx));
		ddbltfx.dwSize=sizeof(ddbltfx);
		ddbltfx.dwROP=SRCCOPY;
		g_pDDSTmp2->Blt(&m_rcBackground,m_pImageBackground,NULL,DDBLT_ROP,&ddbltfx);
	}
	m_text->Draw();
}
void CEdit::AddChar(TCHAR tch,bool bReDraw)
{
	if(m_length >= m_length_max ) return;

	INT32	curSpan = m_length-m_cursor-m_position_show;
	TCHAR *buf = new TCHAR[m_length+2];
	INT32 editWidth=0;
	bool	showMax;
	if(m_length == 0)
	{
		buf[0]=tch;
		buf[1]=_T('\0');
	}
	else
	{
		if((m_cursor+m_position_show)!= m_length)
		{
			_tcscpy(buf,m_buffer);
			buf[m_cursor+m_position_show]=tch;
			_tcscpy(buf+m_cursor+m_position_show+1,m_buffer+m_cursor+m_position_show);
		}
		else
		{
			_tcscpy(buf,m_buffer);
			buf[m_cursor+m_position_show]=tch;
			buf[m_cursor+m_position_show+1]=_T('\0');
		}

	}
	m_length_old=m_length;
	m_length++;
	m_cursor++;
	_tcscpy(m_buffer,buf);

	SetShowBuf();
	if(!m_multiline)
	{
		showMax = IsShowMax();
		if(showMax)
		{
			m_cursor = m_length-curSpan-m_position_show;
			editWidth = GetEditWidth(m_length-1, EDIT_MOVEDOWN);
			if(m_cursor > editWidth)
			{
				m_cursor = editWidth;
			}
		}
		else
		{

		}
	}
	HideWindowCursor();
	m_text->SetWindowsText(m_buffer_show);
	ShowWindowCursor();
	delete [] buf;
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rcEdit);
	}
}
void CEdit::DeleteChar(bool bReDraw)
{
	if(m_length<=0 || m_cursor<=0) return;
	bool	showMax;
	INT32	editWidth;
	INT32	curSpan = m_length-m_cursor-m_position_show;
	m_length_old=m_length;
	m_length--;
	m_cursor--;
	if(m_cursor+m_position_show==m_length)
	{
		m_buffer[m_cursor+m_position_show] =  _T('\0');
	}
	else
	{
		TCHAR *buf = new TCHAR[m_length+1];
		_tcscpy(buf,m_buffer);
		_tcscpy(buf+m_cursor+m_position_show,m_buffer+m_cursor+m_position_show+1);
		_tcscpy(m_buffer,buf);
		delete [] buf;
	}
	SetShowBuf();
	if(!m_multiline)
	{
		showMax = IsShowMax();
		if(showMax)
		{
			//m_cursor++;//should not change for overshow
			m_cursor = m_length-curSpan-m_position_show;
			editWidth = GetEditWidth(m_length-1, EDIT_MOVEDOWN);
			if(m_cursor > editWidth)
			{
				m_cursor = editWidth;
			}
		}
		else
		{
			m_cursor = m_length-curSpan-m_position_show;
		}
	}
	HideWindowCursor();
	m_text->SetWindowsText(m_buffer_show);
	ShowWindowCursor();

	
	if(bReDraw)
	{
		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(&m_rcEdit);
	}
}

void CEdit::LoadEdit(int x,int y,int dx,int dy,bool MultiLine,COLORREF EditColor, COLORREF TextColor,DOUBLE TextYard,UINT32 BufMax)
{
	m_multiline = MultiLine;
	if(BufMax == 0)
	{
		m_length_max = BUFFER_MAX;
	}
	else
	{
		m_length_max = BufMax;
	}	
	m_edit_color = EditColor;
	m_buffer = new TCHAR[m_length_max+20];
	m_buffer_show = new TCHAR[m_length_max+20];
	memset(m_buffer, 0, sizeof(TCHAR)*(m_length_max+20));
	memset(m_buffer_show, 0, sizeof(TCHAR)*(m_length_max+20));
	SetRect(&m_rcEdit, x, y, x+dx, y+dy);
	if(m_multiline)
	{
		m_text->LoadText(x,y,dx,dy,DT_LEFT,TextColor,TextYard);
	}
	else
	{
		m_text->LoadText(x,y,dx,dy,DT_LEFT | DT_VCENTER,TextColor,TextYard);
	}
}

bool  CEdit::LButtonDown(POINT point)
{
	bool ret = true;
	INT32 width;
	INT32 stringWidth=0;
	INT32 curPos=m_position_show;
	if(!PtInRect(&m_rcEdit,point)) return false;
	if(!m_multiline)
	{
		while(stringWidth <= (point.x-m_rcEdit.left) && curPos < m_length)
		{
			m_text->GetCharMetrics(m_buffer[curPos],&width);
			stringWidth+=width;
			curPos++;
		}
		if(curPos==m_position_show)  m_cursor=0;
		else if((curPos==(m_position_show+1)) &&(stringWidth > (point.x-m_rcEdit.left)))
		{
			m_cursor=0;
		}
		else
		{

			if(curPos >= m_length && (point.x-m_rcEdit.left) >= stringWidth) 
			{
				m_cursor = m_length_show;
			}
			else if(curPos >= m_length && (point.x-m_rcEdit.left) < stringWidth)
			{
				m_cursor = m_length_show-1;
			}
			else
			{
				m_cursor = curPos-m_position_show-1;
			}

		}
	}
	else
	{
		TEXTMETRIC tm; 
		m_text->GetTextMetric(&tm);
		INT32 lines = 1;
		INT32 count=0;
		INT32 pos=0;
		INT32 posStart=0;
		INT32 height = tm.tmHeight;
		while(height <= point.y-m_rcEdit.top)
		{
			height += tm.tmHeight;
			lines++;
		}
		if(lines > m_line_count) lines = m_line_count;

		if(lines > 1)
		{
			for(INT32 i=0; i< m_length_show;i++)
			{
				if(m_buffer_show[i]==_T('\n'))
				{
					count++;
				}
				if(count == lines-1)
				{
					pos = i;
					break;
				}
			}
			pos -= (lines-1);
			pos++;//first char of the focus line
		}
		else
		{
			pos = 0;
		}
		posStart= pos;
		while(stringWidth < point.x - m_rcEdit.left && pos < m_length)
		{
			m_text->GetCharMetrics(m_buffer[pos++],&width);
			stringWidth+=width;
		}
		//m_cursor = pos;
		if(pos==posStart)  m_cursor=posStart;
		else if((pos==(posStart+1)) &&(stringWidth > (point.x-m_rcEdit.left)))
		{
			m_cursor=posStart;
		}
		else
		{

			if(pos >= m_length && (point.x-m_rcEdit.left) >= stringWidth) 
			{
				m_cursor = m_length;
			}
			else if(pos >= m_length && (point.x-m_rcEdit.left) < stringWidth)
			{
				m_cursor = m_length-1;
			}
			else
			{
				m_cursor = pos-1;
			}
		}
	}
	HideWindowCursor();
	Draw();
	ShowWindowCursor();
	return ret;
}

void CEdit::MouseMove(POINT point, POINT lastPoint)
{
	if(m_multiline)	return;
	if(!PtInRect(&m_rcEdit,point)) return;
	bool showMax = IsShowMax();
	INT32 span=3;
	if(!showMax) return;

	if(point.x > lastPoint.x)
	{
		//Right
		if(m_position_show+m_length_show < m_length)
		{
			m_position_show=(m_position_show+span < m_length)?(m_position_show+span):(m_length-1);
		}
		else
		{
			return;
		}
	}
	else if(point.x < lastPoint.x)
	{
		//left

		if(m_position_show > 0)
		{
			m_position_show= ((m_position_show-span)>0)?(m_position_show-span):0;

		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}
	m_length_show = GetEditWidth(m_position_show, EDIT_MOVEUP);
	_tcsncpy(m_buffer_show, m_buffer+m_position_show, m_length_show);
	m_buffer_show[m_length_show] = _T('\0');
	HideWindowCursor();
	m_text->SetWindowsText(m_buffer_show);
	ShowWindowCursor();
	//g_Page[g_stackPage.GetTop()].Draw();

}


CBindList::CBindList()
{
	m_list = NULL;
	m_list_cnt = 0;
	m_butt1 = NULL;
	m_butt2 = NULL;
	m_butt1_cnt = 0;
	m_butt2_cnt = 0;
	m_butt2_show = false;
	m_butt_overlap = false;
	m_list_index = 0;
	m_scroll_timeout = true;
	m_list_show = NULL;
	m_scrollEnable = true;
}
CBindList::~CBindList()
{

	if(m_list != NULL)
	{
		delete [] m_list;
		m_list = NULL;
	}
	if(m_butt1 != NULL)
	{
		delete [] m_butt1;
		m_butt1 = NULL;
	}
	if(m_butt2 != NULL)
	{
		delete [] m_butt2;
		m_butt2 = NULL;
	}
	if(m_list_show != NULL)
	{
		delete [] m_list_show;
		m_list_show = NULL;
	}
}
void CBindList::LoadBindList(int x, int y, int dx, int dy, int cnt)
{
	if(cnt <=0)	return;
	m_list_cnt = cnt;
	m_list = new RECT[m_list_cnt];
	if(m_list == NULL)	return;
	for(int i = 0;i<m_list_cnt;i++)
	{
		SetRect(&m_list[i],x,y+dy*i,x+dx,y+dy*(i+1));
	}
	m_list_show = new UINT8[m_list_cnt];
	if(m_list_show == NULL)	return;
	for(int i = 0;i<m_list_cnt;i++)
	{	
		m_list_show[i] = 1;
	}

}
void CBindList::SetBindButt(int *butt1, int cnt1, int *butt2, int cnt2,bool overlap)
{
	if(butt1 == NULL || butt2 == NULL)	return;
	if(cnt1 <= 0 || cnt2 <= 0)	return;
	m_butt1_cnt = cnt1;
	m_butt2_cnt = cnt2;
	m_butt1 = new int[m_butt1_cnt];
	m_butt2 = new int[m_butt2_cnt];
	memcpy(m_butt1,butt1,m_butt1_cnt*sizeof(int));
	memcpy(m_butt2,butt2,m_butt2_cnt*sizeof(int));
	m_butt_overlap = overlap;

}
void CBindList::ResetBindButt()
{
	if(m_list_cnt == 0 || m_butt1_cnt ==0 || m_butt2_cnt==0)	return;
	int buttId;
	for(int i =m_list_index*(m_butt2_cnt/m_list_cnt);i< (m_list_index+1)*(m_butt2_cnt/m_list_cnt);i++)
	{
		buttId = m_butt2[i];
		g_Butt[buttId].ShowWindow(SW_HIDE);
	}
	if(m_butt_overlap)
	{
		for(int i =m_list_index*(m_butt1_cnt/m_list_cnt);i< (m_list_index+1)*(m_butt1_cnt/m_list_cnt);i++)
		{
			buttId = m_butt1[i];
			g_Butt[buttId].ShowWindow(SW_SHOW);
		}
	}
	m_butt2_show = false;
}
void CBindList::MaskScrollMsg()
{
	m_scroll_timeout = true;
}

void CBindList::SetScrollEnable(bool value)
{
	m_scrollEnable = value;
}
void CBindList::ShowBindList(int index,UINT8 show)
{
	if(index >= m_list_cnt)	return;
	if(m_list_show != NULL)
	{
		m_list_show[index] = show;
	}
}
void CBindList::MouseMove(POINT point, POINT lastPoint)
{
	if(m_list_cnt == 0 || m_butt1_cnt ==0 || m_butt2_cnt==0)	return;
	if(!m_scroll_timeout || !m_scrollEnable)	return;
	RECT rect;
	int buttId;
	SetRect(&rect,m_list[0].left,m_list[0].top,m_list[0].right,m_list[0].top+(m_list[0].bottom-m_list[0].top)*m_list_cnt);
	if(!PtInRect(&rect,point))	return;
	UINT8 show=0;
	int i=0;
	for(i =0;i<m_list_cnt;i++)
	{
		if(PtInRect(&m_list[i],point) && PtInRect(&m_list[i],lastPoint))
		{
			m_list_index = i;
			show = m_list_show[i];
			if(m_list_show[i] == 0x0) return;   //not show then return
			break;
		}
	}
	if(i==m_list_cnt) return;
	m_scroll_timeout = false;
	if(!m_butt2_show)
	{
		if(m_butt_overlap)
		{
			for(int i =m_list_index*(m_butt1_cnt/m_list_cnt);i< (m_list_index+1)*(m_butt1_cnt/m_list_cnt);i++)
			{
				buttId = m_butt1[i];
				g_Butt[buttId].ShowWindow(SW_HIDE);
			}
		}
		for(int i =m_list_index*(m_butt2_cnt/m_list_cnt);i< (m_list_index+1)*(m_butt2_cnt/m_list_cnt);i++)
		{
			buttId = m_butt2[i];
			g_Butt[buttId].ShowWindow(SW_SHOW);
		}
		m_butt2_show = true;
	}
	else
	{
		for(int i =m_list_index*(m_butt2_cnt/m_list_cnt);i< (m_list_index+1)*(m_butt2_cnt/m_list_cnt);i++)
		{
			buttId = m_butt2[i];
			g_Butt[buttId].ShowWindow(SW_HIDE);
		}
		if(m_butt_overlap)
		{
			for(int i =m_list_index*(m_butt1_cnt/m_list_cnt);i< (m_list_index+1)*(m_butt1_cnt/m_list_cnt);i++)
			{
				buttId = m_butt1[i];
				g_Butt[buttId].ShowWindow(SW_SHOW);
			}
		}
		m_butt2_show = false;
	}
	SetTimer(g_hWnd,IDT_TIMER_SCOLL,1000,NULL);

}
CPage::CPage()
{
	pictHead=NULL;
	progHead=NULL;
	textHead=NULL;
	buttHead=NULL;
	switchHead=NULL;
	m_uID=NULL;
	m_bHaveGrap=false;
	m_flipCnt=1;
	m_lastMovePoint2.x=0;
	m_lastMovePoint2.y=0;
	m_lastMovePoint3.x=0;
	m_lastMovePoint3.y=0;
	m_lastMovePoint.x=0;
	m_lastMovePoint.y=0;
	m_moveDirection=VERTICAL;
	m_stopDirection=NONESTOP;
	SetRect(&m_rct,0,0,SYSTEMW,SYSTEMH);
	SetRectEmpty(&m_moveRect);
	m_func_page_enter = NULL;
	m_func_page_exit = NULL;
	m_currentEdit = 0;
}
void CPage::SetCallback(FUNCCALLBACK enter, FUNCCALLBACK exit)
{
	m_func_page_enter =enter;
	m_func_page_exit = exit;

}

bool CPage::LoadPage(UINT ID,int x,int y,int dx,int dy,TCHAR *ResPath,TCHAR *ResFile)
{
	m_uID=ID;
	SetRect(&m_rct,x,y,x+dx,y+dy);
	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=dy;
	g_ddsd.dwWidth=dx;
	if(g_pDD->CreateSurface(&g_ddsd,&m_pDDSPage,NULL)!=DD_OK)
		return false;

	if(ResFile!=NULL)
	{
		TCHAR Res[MAX_PATH*sizeof(TCHAR)]={0};
		wcscpy(Res,ResPath);
		wcscat(Res,ResFile);
		HBITMAP hbm=SHLoadDIBitmap(Res);
		if (hbm==NULL)
			return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pDDS,NULL)!=DD_OK)
			return false;
		DDCopyBitmap(m_pDDS,hbm,0,0,dx,dy);
		DeleteObject(hbm);
	}
	return true;
}

bool CPage::LoadPage(UINT ID,int x,int y,int dx,int dy,HINSTANCE hUiDLL,TCHAR *ResFile)
{
	m_uID=ID;
	SetRect(&m_rct,x,y,x+dx,y+dy);
	g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
	g_ddsd.dwHeight=dy;
	g_ddsd.dwWidth=dx;
	if(g_pDD->CreateSurface(&g_ddsd,&m_pDDSPage,NULL)!=DD_OK)
		return false;

	if(ResFile!=NULL)
	{
		HBITMAP hbm=LoadBitmap(hUiDLL,ResFile);
		if (hbm==NULL)
			return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pDDS,NULL)!=DD_OK)
			return false;
		
		DDCopyBitmap(m_pDDS,hbm,0,0,dx,dy);
		DeleteObject(hbm);
	}
	return true;
}
RECT CPage::GetMoveRect()
{
	return m_moveRect;
}

void CPage::SetChildText(int textindex)
{
	NODETEXT *p,*s;
	s=(NODETEXT *)new(NODETEXT);
	s->id=textindex;

	p=textHead;
	if(textHead==NULL)
	{
		textHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		textHead=s;
	}
}

void CPage::SetChildButton(int buttonindex)
{
	NODEBUTT *p,*s;
	s=(NODEBUTT *)new(NODEBUTT);
	s->id=buttonindex;

	p=buttHead;
	if(buttHead==NULL)
	{
		buttHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		buttHead=s;
	}
}
void CPage::SetChildSwitch(int switchindex)
{
	NODESWITCH *p,*s;
	s=(NODESWITCH *)new(NODESWITCH);
	s->id=switchindex;

	p=switchHead;
	if(switchHead==NULL)
	{
		switchHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		switchHead=s;
	}
}
void CPage::SetChildPict(int pictureindex)
{
	NODEPICT *p,*s;
	s=(NODEPICT *)new(NODEPICT);
	s->id=pictureindex;

	p=pictHead;
	if(pictHead==NULL)
	{
		pictHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		pictHead=s;
	}
}

void CPage::SetChildProg(int progressindex)
{
	NODEPROG *p,*s;
	s=(NODEPROG *)new(NODEPROG);
	s->id=progressindex;

	p=progHead;
	if(progHead==NULL)
	{
		progHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		progHead=s;
	}
}
void CPage::SetChildPhoto(int photoindex)
{
	NODEPHOTO *p,*s;
	s=(NODEPHOTO *)new(NODEPHOTO);
	s->id=photoindex;

	p=photoHead;
	if(photoHead==NULL)
	{
		photoHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		photoHead=s;
	}
}
void CPage::SetChildList(int listindex)
{
	NODELISTBOX *p,*s;
	s=(NODELISTBOX *)new(NODELISTBOX);
	s->id=listindex;

	p=listboxHead;
	if(listboxHead==NULL)
	{
		listboxHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		listboxHead=s;
	}
}
void CPage::SetChildHandWrite(int handwriteindex)
{
	NODEHANDWRITE *p,*s;
	s=(NODEHANDWRITE *)new(NODEHANDWRITE);
	s->id=handwriteindex;

	p=handwriteHead;
	if(handwriteHead==NULL)
	{
		handwriteHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		handwriteHead=s;
	}
}
void CPage::SetChildEdit(int editIndex)
{
	NODEEDIT *p,*s;
	s=(NODEEDIT *)new(NODEEDIT);
	s->id=editIndex;

	p=editHead;
	if(editHead==NULL)
	{
		editHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		editHead=s;
	}
}
void CPage::SetChildBindList(int bindlistIndex)
{
	NODEBINDLIST *p,*s;
	s=(NODEBINDLIST *)new(NODEBINDLIST);
	s->id=bindlistIndex;

	p=bindlistHead;
	if(bindlistHead==NULL)
	{
		bindlistHead=s;
		s->next=NULL;
	}
	else
	{
		s->next=p;
		bindlistHead=s;
	}
}
void CPage::SetChildGrapList(CList *listGrap)
{
	m_bHaveGrap=true;
	g_listGrap=listGrap;
	m_grap[0].SetProp(GRAP_PREV);
	m_grap[1].SetProp(GRAP_MID);
	m_grap[2].SetProp(GRAP_NEXT);
}
void CPage::SetCurrentGrap(int flag)
{
	if(!g_listGrap->GetItemCount()) return;
	m_grap[0].SetIndex(flag-1);
	m_grap[1].SetIndex(flag);
	m_grap[2].SetIndex(flag+1);
	m_grap[1].LoadGraph(m_moveDirection);
	Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(NULL);
	m_grap[2].LoadGraph(m_moveDirection);
	m_grap[0].LoadGraph(m_moveDirection);
	Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(NULL);
}
void CPage::RotateCurrentGrap(float angle)
{
	if(!m_bHaveGrap) return;
	for(int i=0;i<3;i++)
	{
		if(m_grap[i].GetProp()==GRAP_MID)
		{
			m_grap[i].Rotate(angle);
		}
	}
}
void CPage::Draw()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwFillColor=0; 
	ddbltfx.dwROP=SRCCOPY;
	// Draw Back
	if(m_pDDS!=NULL)
	{
		g_pDDSTmp1->Blt(&m_rct,m_pDDS,NULL,DDBLT_ROP,&ddbltfx);
		g_pDDSTmp2->Blt(&m_rct,m_pDDS,NULL,DDBLT_ROP,&ddbltfx);
	}
	else
	{
		g_pDDSTmp1->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
		g_pDDSTmp2->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
	}
	// Draw Grap
	if(m_bHaveGrap)
	{
		m_grap[0].Draw();
		m_grap[1].Draw();
		m_grap[2].Draw();
	}
	// Draw Photo
	NODEPHOTO *currentPhoto=photoHead;
	while(currentPhoto!=NULL)
	{
		g_Photo[currentPhoto->id].Draw();
		currentPhoto=currentPhoto->next;
	}
	// Draw Pict
	NODEPICT *currentPict=pictHead;
	while(currentPict!=NULL)
	{
		g_Pict[currentPict->id].Draw();
		currentPict=currentPict->next;
	}
	// Draw Prog
	NODEPROG *currentProg=progHead;
	while(currentProg!=NULL)
	{
		g_Prog[currentProg->id].Draw();
		currentProg=currentProg->next;
	}
	
	// Draw List
	NODELISTBOX *currentListBox=listboxHead;
	while(currentListBox!=NULL)
	{
		g_ListBox[currentListBox->id].Draw();
		currentListBox=currentListBox->next;
	}
	// Draw Butt
	NODEBUTT *currentButt=buttHead;
	while(currentButt!=NULL)
	{
		g_Butt[currentButt->id].Draw();
		currentButt=currentButt->next;
	}
	// Draw Switch
	NODESWITCH *currentSwitch=switchHead;
	while(currentSwitch!=NULL)
	{
		g_Switch[currentSwitch->id].Draw();
		currentSwitch=currentSwitch->next;
	}
	// Draw Text
	NODETEXT *currentText=textHead;
	while(currentText!=NULL)
	{
		g_Text[currentText->id].Draw();
		currentText=currentText->next;
	}
	//Draw Edit
	NODEEDIT *currentEdit=editHead;
	while(currentEdit!=NULL)
	{
		g_Edit[currentEdit->id].Draw();
		currentEdit=currentEdit->next;
	}
	// Draw HandWrite
	NODEHANDWRITE *currentHandWrite=handwriteHead;
	while(currentHandWrite!=NULL)
	{
		g_HandWrite[currentHandWrite->id].Draw();
		currentHandWrite=currentHandWrite->next;
	}
	// Draw Title Text
	CText::Draw();
	m_pDDSPage->Blt(NULL,g_pDDSTmp2,&m_rct,DDBLT_ROP,&ddbltfx);
	RECT rct;
	SetRect(&rct,m_moveRect.left-m_rct.left,m_moveRect.top-m_rct.top,m_moveRect.right-m_rct.left,m_moveRect.bottom-m_rct.top);
	m_pDDSPage->Blt(&rct,g_pDDSTmp1,&m_moveRect,DDBLT_ROP,&ddbltfx);
}

void CPage::UpdatePage(RECT *RefRct)
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	while (true)
	{
#ifdef MODE_FULLSCREEN
		hRet=g_pDDSBack->Blt(&m_rct,m_pDDSPage,NULL,DDBLT_ROP,&ddbltfx);
		//hRet=g_pDDSPrimary->Blt(&m_rct,m_pDDSPage,NULL,DDBLT_ROP,&ddbltfx);
#else
		if(RefRct==NULL)
		{
			hRet=g_pDDSPrimary->Blt(&m_rct,m_pDDSPage,NULL,DDBLT_ROP,&ddbltfx);//整页刷新
		}
		else
		{
			RECT RRct={0};
			IntersectRect(&RRct,RefRct,&m_rct);
			RECT rct={RRct.left-m_rct.left,RRct.top-m_rct.top,RRct.right-m_rct.left,RRct.bottom-m_rct.top};
			hRet=g_pDDSPrimary->Blt(&RRct,m_pDDSPage,&rct,DDBLT_ROP,&ddbltfx);//局部刷新
		}
		
#endif
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
#ifdef MODE_FULLSCREEN
	while (true)
	{
		hRet=g_pDDSPrimary->Flip(g_pDDSBack,0);
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
#endif
}
void CPage::FlipPageRefreshMid()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	//
#ifdef MODE_FULLSCREEN
	g_pDDSBack->Blt(NULL,g_pDDSPrimary,NULL,DDBLT_ROP,&ddbltfx);
#endif
	//
	for(int i=(m_rct.bottom-m_rct.top)/2/m_flipCnt/4;i<(m_rct.bottom-m_rct.top)/m_flipCnt/2;i=i+((m_rct.bottom-m_rct.top)/2/m_flipCnt-i+4)/4)
	{
		RECT rct1={0,(m_rct.bottom-m_rct.top)/2-(i+1)*m_flipCnt,m_rct.right-m_rct.left,(m_rct.bottom-m_rct.top)/2+(i+1)*m_flipCnt};
		RECT rct1s={rct1.left+m_rct.left,rct1.top+m_rct.top,rct1.right+m_rct.left,rct1.bottom+m_rct.top};
		while (true)
		{
#ifdef MODE_FULLSCREEN
			hRet=g_pDDSBack->Blt(&rct1s,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
#else
			hRet=g_pDDSPrimary->Blt(&rct1s,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
#endif
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#ifdef MODE_FULLSCREEN
		while (true)
		{
			hRet=g_pDDSPrimary->Flip(g_pDDSBack,0);
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
		if((m_rct.bottom-m_rct.top)/2/m_flipCnt-i<=0)
		{
			break;
		}
#endif
		Sleep(10);
	}
}

void CPage::FlipPageRefreshTop()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	//
#ifdef MODE_FULLSCREEN
	g_pDDSBack->Blt(NULL,g_pDDSPrimary,NULL,DDBLT_ROP,&ddbltfx);
#endif
	//
	for(int i=(m_rct.bottom-m_rct.top)/m_flipCnt/4;i<(m_rct.bottom-m_rct.top)/m_flipCnt;i=i+((m_rct.bottom-m_rct.top)/m_flipCnt-i+4)/4)
	{
		RECT rct1={0,0,m_rct.right-m_rct.left,(i+1)*m_flipCnt};
		RECT rct2={m_rct.left,m_rct.top,m_rct.right,m_rct.top+(i+1)*m_flipCnt};
		while (true)
		{
#ifdef MODE_FULLSCREEN
			hRet=g_pDDSBack->Blt(&rct2,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
#else
			hRet=g_pDDSPrimary->Blt(&rct2,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
#endif
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#ifdef MODE_FULLSCREEN
		while (true)
		{
			hRet=g_pDDSPrimary->Flip(g_pDDSBack,0);
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#endif
		if((m_rct.bottom-m_rct.top)/m_flipCnt-i<=0)
		{
			break;
		}
		Sleep(10);
	}
}
void CPage::FlipPageRefreshMix()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	//
#ifdef MODE_FULLSCREEN
	g_pDDSBack->Blt(NULL,g_pDDSPrimary,NULL,DDBLT_ROP,&ddbltfx);
#endif
	//
	for(int i=0;i<=(m_rct.bottom-m_rct.top)/m_flipCnt/8;i++)
	{
		RECT rct1={0,0,m_rct.right-m_rct.left,(i+1)*m_flipCnt};
		RECT rct2={0,(m_rct.bottom-m_rct.top)/8,m_rct.right-m_rct.left,(m_rct.bottom-m_rct.top)/8+(i+1)*m_flipCnt};
		RECT rct3={0,2*(m_rct.bottom-m_rct.top)/8,m_rct.right-m_rct.left,2*(m_rct.bottom-m_rct.top)/8+(i+1)*m_flipCnt};
		RECT rct4={0,3*(m_rct.bottom-m_rct.top)/8,m_rct.right-m_rct.left,3*(m_rct.bottom-m_rct.top)/8+(i+1)*m_flipCnt};
		RECT rct5={0,4*(m_rct.bottom-m_rct.top)/8,m_rct.right-m_rct.left,4*(m_rct.bottom-m_rct.top)/8+(i+1)*m_flipCnt};
		RECT rct6={0,5*(m_rct.bottom-m_rct.top)/8,m_rct.right-m_rct.left,5*(m_rct.bottom-m_rct.top)/8+(i+1)*m_flipCnt};
		RECT rct7={0,6*(m_rct.bottom-m_rct.top)/8,m_rct.right-m_rct.left,6*(m_rct.bottom-m_rct.top)/8+(i+1)*m_flipCnt};
		RECT rct8={0,7*(m_rct.bottom-m_rct.top)/8,m_rct.right-m_rct.left,7*(m_rct.bottom-m_rct.top)/8+(i+1)*m_flipCnt};
		if(7*(m_rct.bottom-m_rct.top)/8+(i+1)*m_flipCnt>SYSTEMH)
		{
			SetRect(&rct8,rct8.left,rct8.top,rct8.right,SYSTEMH);
		}
		RECT rct1s={rct1.left+m_rct.left,rct1.top+m_rct.top,rct1.right+m_rct.left,rct1.bottom+m_rct.top};
		RECT rct2s={rct2.left+m_rct.left,rct2.top+m_rct.top,rct2.right+m_rct.left,rct2.bottom+m_rct.top};
		RECT rct3s={rct3.left+m_rct.left,rct3.top+m_rct.top,rct3.right+m_rct.left,rct3.bottom+m_rct.top};
		RECT rct4s={rct4.left+m_rct.left,rct4.top+m_rct.top,rct4.right+m_rct.left,rct4.bottom+m_rct.top};
		RECT rct5s={rct5.left+m_rct.left,rct5.top+m_rct.top,rct5.right+m_rct.left,rct5.bottom+m_rct.top};
		RECT rct6s={rct6.left+m_rct.left,rct6.top+m_rct.top,rct6.right+m_rct.left,rct6.bottom+m_rct.top};
		RECT rct7s={rct7.left+m_rct.left,rct7.top+m_rct.top,rct7.right+m_rct.left,rct7.bottom+m_rct.top};
		RECT rct8s={rct8.left+m_rct.left,rct8.top+m_rct.top,rct8.right+m_rct.left,rct8.bottom+m_rct.top};
		while (true)
		{
#ifdef MODE_FULLSCREEN
			hRet=g_pDDSBack->Blt(&rct1s,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSBack->Blt(&rct2s,m_pDDSPage,&rct2,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSBack->Blt(&rct3s,m_pDDSPage,&rct3,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSBack->Blt(&rct4s,m_pDDSPage,&rct4,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSBack->Blt(&rct5s,m_pDDSPage,&rct5,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSBack->Blt(&rct6s,m_pDDSPage,&rct6,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSBack->Blt(&rct7s,m_pDDSPage,&rct7,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSBack->Blt(&rct8s,m_pDDSPage,&rct8,DDBLT_ROP,&ddbltfx);
#else
			hRet=g_pDDSPrimary->Blt(&rct1s,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSPrimary->Blt(&rct2s,m_pDDSPage,&rct2,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSPrimary->Blt(&rct3s,m_pDDSPage,&rct3,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSPrimary->Blt(&rct4s,m_pDDSPage,&rct4,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSPrimary->Blt(&rct5s,m_pDDSPage,&rct5,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSPrimary->Blt(&rct6s,m_pDDSPage,&rct6,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSPrimary->Blt(&rct7s,m_pDDSPage,&rct7,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSPrimary->Blt(&rct8s,m_pDDSPage,&rct8,DDBLT_ROP,&ddbltfx);
#endif
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#ifdef MODE_FULLSCREEN
		while (true)
		{
			hRet=g_pDDSPrimary->Flip(g_pDDSBack,0);
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#endif
		if((m_rct.bottom-m_rct.top)/m_flipCnt-i<=0)
		{
			break;
		}
		//Sleep(10);
	}
}
void CPage::FlipPageSlideTop()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	//
#ifdef MODE_FULLSCREEN
	g_pDDSBack->Blt(NULL,g_pDDSPrimary,NULL,DDBLT_ROP,&ddbltfx);
#endif
	//
	for(int i=(m_rct.bottom-m_rct.top)/m_flipCnt/4;i<(m_rct.bottom-m_rct.top)/m_flipCnt;i=i+((m_rct.bottom-m_rct.top)/m_flipCnt-i+4)/4)
	{
		RECT rct1={0,m_rct.bottom-m_rct.top-(i+1)*m_flipCnt,m_rct.right-m_rct.left,m_rct.bottom-m_rct.top};
		RECT rct2={m_rct.left,m_rct.top,m_rct.right,m_rct.top+(i+1)*m_flipCnt};
		while (true)
		{
#ifdef MODE_FULLSCREEN
			hRet=g_pDDSBack->Blt(&rct2,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
#else
			hRet=g_pDDSPrimary->Blt(&rct2,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
#endif
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#ifdef MODE_FULLSCREEN
		while (true)
		{
			hRet=g_pDDSPrimary->Flip(g_pDDSBack,0);
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#endif
		if((m_rct.bottom-m_rct.top)/m_flipCnt-i<=0)
		{
			break;
		}
		Sleep(10);
	}
}

void CPage::FlipPageSlideRig()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	//
#ifdef MODE_FULLSCREEN
	g_pDDSBack->Blt(NULL,g_pDDSPrimary,NULL,DDBLT_ROP,&ddbltfx);
#endif
	//
	for(int i=(m_rct.right-m_rct.left)/m_flipCnt/4;i<(m_rct.right-m_rct.left)/m_flipCnt;i=i+((m_rct.right-m_rct.left)/m_flipCnt-i+4)/4)
	{
		RECT rct1={0,0,m_rct.left+(i+1)*m_flipCnt-m_rct.left,m_rct.bottom-m_rct.top};
		RECT rct2={m_rct.right-(i+1)*m_flipCnt,m_rct.top,m_rct.right,m_rct.bottom};
		while (true)
		{
#ifdef MODE_FULLSCREEN
			hRet=g_pDDSBack->Blt(&rct2,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
#else
			hRet=g_pDDSPrimary->Blt(&rct2,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
#endif
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#ifdef MODE_FULLSCREEN
		while (true)
		{
			hRet=g_pDDSPrimary->Flip(g_pDDSBack,0);
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#endif
		if((m_rct.right-m_rct.left)/m_flipCnt-i<=0)
		{
			break;
		}
		Sleep(10);
	}
}
void CPage::FlipPageSlideMid()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	//
#ifdef MODE_FULLSCREEN
	g_pDDSBack->Blt(NULL,g_pDDSPrimary,NULL,DDBLT_ROP,&ddbltfx);
#endif
	//
	for(int i=(m_rct.bottom-m_rct.top)/2/m_flipCnt/4;i<(m_rct.bottom-m_rct.top)/m_flipCnt/2;i=i+((m_rct.bottom-m_rct.top)/2/m_flipCnt-i+4)/4)
	{
		RECT rct1={0,0,m_rct.right-m_rct.left,(i+1)*m_flipCnt};
		RECT rct2={0,m_rct.bottom-m_rct.top-(i+1)*m_flipCnt,m_rct.right-m_rct.left,m_rct.bottom-m_rct.top};
		RECT rcts1={m_rct.left,(m_rct.top+m_rct.bottom)/2-(i+1)*m_flipCnt,m_rct.right,(m_rct.top+m_rct.bottom)/2};
		RECT rcts2={m_rct.left,(m_rct.top+m_rct.bottom)/2,m_rct.right,(m_rct.top+m_rct.bottom)/2+(i+1)*m_flipCnt};
		while (true)
		{
#ifdef MODE_FULLSCREEN
			hRet=g_pDDSBack->Blt(&rcts1,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSBack->Blt(&rcts2,m_pDDSPage,&rct2,DDBLT_ROP,&ddbltfx);
#else
			hRet=g_pDDSPrimary->Blt(&rcts1,m_pDDSPage,&rct1,DDBLT_ROP,&ddbltfx);
			hRet=g_pDDSPrimary->Blt(&rcts2,m_pDDSPage,&rct2,DDBLT_ROP,&ddbltfx);
#endif
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#ifdef MODE_FULLSCREEN
		while (true)
		{
			hRet=g_pDDSPrimary->Flip(g_pDDSBack,0);
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#endif
		if((m_rct.bottom-m_rct.top)/2/m_flipCnt-i<=0)
		{
			break;
		}
		Sleep(10);
	}
}

void CPage::FlipPageTransTop()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;
	//
#ifdef MODE_FULLSCREEN
	g_pDDSBack->Blt(NULL,g_pDDSPrimary,NULL,DDBLT_ROP,&ddbltfx);
#endif
	//
	for(int i=(m_rct.bottom-m_rct.top)/m_flipCnt/4;i<(m_rct.bottom-m_rct.top)/m_flipCnt;i=i+((m_rct.bottom-m_rct.top)/m_flipCnt-i+4)/4)
	{
		RECT rct0={0,0,m_rct.right-m_rct.left,(i+1)*m_flipCnt};
		RECT rct1={rct0.left+m_rct.left,rct0.top+m_rct.top,rct0.right+m_rct.left,rct0.bottom+m_rct.top};
		while (true)
		{
#ifdef MODE_FULLSCREEN
			hRet=g_pDDSBack->Blt(&rct1,m_pDDSPage,NULL,DDBLT_ROP,&ddbltfx);
#else
			hRet=g_pDDSPrimary->Blt(&rct1,m_pDDSPage,NULL,DDBLT_ROP,&ddbltfx);
#endif
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#ifdef MODE_FULLSCREEN
		while (true)
		{
			hRet=g_pDDSPrimary->Flip(g_pDDSBack,0);
			if (hRet==DD_OK)
				break;
			if (hRet==DDERR_SURFACELOST)
			{
				hRet=RestoreAll();
				if (hRet!=DD_OK)
					break;
			}
			if (hRet!=DDERR_WASSTILLDRAWING)
				break;
		}
#endif
		if((m_rct.bottom-m_rct.top)/m_flipCnt-i<=0)
		{
			break;
		}
		Sleep(10);
	}
}
void CPage::LButtonDown(POINT point)
{
	bool res;
	//HandWrite
	NODEHANDWRITE *currentHandWrite=handwriteHead;
	while(currentHandWrite!=NULL)
	{
		
		if(g_HandWrite[currentHandWrite->id].IsInHandWriteRct(point))
		{
			g_HandWrite[currentHandWrite->id].LButtonDown(point);
			return;
		}
		currentHandWrite=currentHandWrite->next;
	}
	//Butt
	NODEBUTT *currentButt=buttHead;
	while(currentButt!=NULL)
	{
		g_Butt[currentButt->id].LButtonDown(point,m_moveRect);
		currentButt=currentButt->next;
	}
	//SWitch
	NODESWITCH *currentSwitch=switchHead;
	while(currentSwitch!=NULL)
	{
		g_Switch[currentSwitch->id].LButtonDown(point,m_moveRect);
		currentSwitch=currentSwitch->next;
	}
	//Edit
	NODEEDIT *currentEdit=editHead;
	while(currentEdit!=NULL)
	{
		res = g_Edit[currentEdit->id].LButtonDown(point);
		if(res)
		{
			m_currentEdit = currentEdit->id;
			break;
		}
		currentEdit=currentEdit->next;
	}
	//List
	NODELISTBOX *currentListBox=listboxHead;
	while(currentListBox!=NULL)
	{
		g_ListBox[currentListBox->id].LButtonDown(point);
		currentListBox=currentListBox->next;
	}
	//Grap
	if(m_bHaveGrap)
	{
		m_grap[0].LButtonDown();
		m_grap[1].LButtonDown();
		m_grap[2].LButtonDown();
	}
	//
	m_lastMovePoint3.x=point.x;
	m_lastMovePoint3.y=point.y;
	m_lastMovePoint2.x=point.x;
	m_lastMovePoint2.y=point.y;
	m_lastMovePoint.x=point.x;
	m_lastMovePoint.y=point.y;
	m_lastDownPoint.x=point.x;
	m_lastDownPoint.y=point.y;
	m_lastMovePoint.x=point.x;
	m_lastMovePoint.y=point.y;
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
}

void CPage::MouseMove(POINT point)
{
	//HandWrite
	NODEHANDWRITE *currentHandWrite=handwriteHead;
	while(currentHandWrite!=NULL)
	{
		if(g_HandWrite[currentHandWrite->id].IsInHandWriteRct(point))
		{
			g_HandWrite[currentHandWrite->id].MouseMove(point);
			return;
		}
		currentHandWrite=currentHandWrite->next;
	}
	//Prog
	NODEPROG *currentProg=progHead;
	while(currentProg!=NULL)
	{
		g_Prog[currentProg->id].MouseMove(point);
		currentProg=currentProg->next;
	}
	//Edit
	NODEEDIT *currentEdit=editHead;
	while(currentEdit!=NULL)
	{
		g_Edit[currentEdit->id].MouseMove(point, m_lastMovePoint);
		currentEdit=currentEdit->next;
	}
	//BindList
	NODEBINDLIST *currentBindList=bindlistHead;
	while(currentBindList!=NULL)
	{
		g_BindList[currentBindList->id].MouseMove(point, m_lastMovePoint);
		currentBindList=currentBindList->next;
	}
	//List
	NODELISTBOX *currentListBox=listboxHead;
	while(currentListBox!=NULL)
	{
		g_ListBox[currentListBox->id].MouseMove(point);
		currentListBox=currentListBox->next;
	}

	if(!PtInRect(&m_moveRect,point)) 
	{
		m_lastMovePoint.x=point.x;
		m_lastMovePoint.y=point.y;
		return;
	}
	//
	if(m_moveDirection==HORIZONTAL)
	{
		if(m_stopDirection==(LEFTSTOP|RIGHTSTOP))
		{
			m_lastMovePoint.x=point.x;
			m_lastMovePoint.y=point.y;
			return;
		}
		if(m_stopDirection==LEFTSTOP&&point.x<=m_lastMovePoint.x)
		{
				m_lastMovePoint.x=point.x;
				m_lastMovePoint.y=point.y;
				return;
		}
		if(m_stopDirection==RIGHTSTOP&&point.x>=m_lastMovePoint.x)
		{
				m_lastMovePoint.x=point.x;
				m_lastMovePoint.y=point.y;
				return;
		}
		if(point.x>m_lastDownPoint.x-VIBRATEPIX&&point.x<m_lastDownPoint.x+VIBRATEPIX)
		{
			m_lastMovePoint.x=point.x;
			m_lastMovePoint.y=point.y;
			return;
		}
	}
	else
	{
		if(m_stopDirection==(UPSTOP|DOWNSTOP))
		{
			m_lastMovePoint.x=point.x;
			m_lastMovePoint.y=point.y;
			return;
		}
		if(m_stopDirection==UPSTOP&&point.y<=m_lastMovePoint.y)
		{
				m_lastMovePoint.x=point.x;
				m_lastMovePoint.y=point.y;
				return;
		}
		if(m_stopDirection==DOWNSTOP&&point.y>=m_lastMovePoint.y)
		{
				m_lastMovePoint.x=point.x;
				m_lastMovePoint.y=point.y;
				return;
		}
		if(point.y>m_lastDownPoint.y-VIBRATEPIX&&point.y<m_lastDownPoint.y+VIBRATEPIX)
		{
			m_lastMovePoint.x=point.x;
			m_lastMovePoint.y=point.y;
			return;
		}
	}
	//Pict
	NODEPICT *currentPict=pictHead;
	while(currentPict!=NULL)
	{
		g_Pict[currentPict->id].MouseMove(point,m_moveDirection,m_lastMovePoint);
		currentPict=currentPict->next;
	}
	//Butt
	NODEBUTT *currentButt=buttHead;
	while(currentButt!=NULL)
	{
		g_Butt[currentButt->id].MouseMove(point,m_moveDirection,m_lastMovePoint,m_moveRect);
		currentButt=currentButt->next;
	}
	//SWitch
	NODESWITCH *currentSwitch=switchHead;
	while(currentSwitch!=NULL)
	{
		g_Switch[currentSwitch->id].MouseMove(point,m_moveDirection,m_lastMovePoint,m_moveRect);
		currentSwitch=currentSwitch->next;
	}
	//Text
	NODETEXT *currentText=textHead;
	while(currentText!=NULL)
	{
		g_Text[currentText->id].MouseMove(point,m_moveDirection,m_lastMovePoint);
		currentText=currentText->next;
	}
	//Grap
	if(m_bHaveGrap)
	{
		m_grap[0].MouseMove(point,m_moveDirection,m_lastMovePoint);
		m_grap[1].MouseMove(point,m_moveDirection,m_lastMovePoint);
		m_grap[2].MouseMove(point,m_moveDirection,m_lastMovePoint);
	}

	m_lastMovePoint3.x=m_lastMovePoint2.x;
	m_lastMovePoint3.y=m_lastMovePoint2.y;
	m_lastMovePoint2.x=m_lastMovePoint.x;
	m_lastMovePoint2.y=m_lastMovePoint.y;
	m_lastMovePoint.x=point.x;
	m_lastMovePoint.y=point.y;
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);
}

void CPage::LButtonUp(POINT point)
{
	//HandWrite
	NODEHANDWRITE *currentHandWrite=handwriteHead;
	while(currentHandWrite!=NULL)
	{
		if(g_HandWrite[currentHandWrite->id].IsInHandWriteRct(point))
		{
			g_HandWrite[currentHandWrite->id].LButtonUp(point);
			return;
		}
		currentHandWrite=currentHandWrite->next;
	}
	//Butt
	NODEBUTT *currentButt=buttHead;
	while(currentButt!=NULL)
	{
		g_Butt[currentButt->id].LButtonUp(point,m_moveRect);
		currentButt=currentButt->next;
	}
	//SWitch
	NODESWITCH *currentSwitch=switchHead;
	while(currentSwitch!=NULL)
	{
		g_Switch[currentSwitch->id].LButtonUp(point,m_moveRect);
		currentSwitch=currentSwitch->next;
	}
	//List
	NODELISTBOX *currentListBox=listboxHead;
	while(currentListBox!=NULL)
	{
		g_ListBox[currentListBox->id].LButtonUp(point);
		currentListBox=currentListBox->next;
	}
	//Grap
	if(m_bHaveGrap)
	{
		m_grap[0].LButtonUp(m_moveDirection);
		m_grap[1].LButtonUp(m_moveDirection);
		m_grap[2].LButtonUp(m_moveDirection);
	}
	//
	//m_lastMovePoint.x=point.x;
	//m_lastMovePoint.y=point.y;
	g_Page[g_stackPage.GetTop()].Draw();
	g_Page[g_stackPage.GetTop()].UpdatePage(&m_rct);

	PostMessage(g_hWnd,WM_CLICKPAGE,m_uID,0);

	//Grap
	if(m_bHaveGrap)
	{
		m_grap[0].UpdateState(m_moveDirection);
		m_grap[1].UpdateState(m_moveDirection);
		m_grap[2].UpdateState(m_moveDirection);
	}
	else
	{
		if(PtInRect(&m_moveRect,m_lastDownPoint))
		{
			if(m_moveDirection==HORIZONTAL)
			{
				if(point.x>m_lastMovePoint3.x+FASTMOVEPIX)//右
				{
					POINT pt;
					pt.x=m_moveRect.left;
					pt.y=m_moveRect.top;
					LButtonDown(pt);
					pt.x=m_moveRect.left+1;
					pt.y=m_moveRect.top+1;
					MouseMove(pt);
					pt.x=m_moveRect.right-(m_moveRect.right-m_moveRect.left)/2;
					MouseMove(pt);
					pt.x=m_moveRect.right-(m_moveRect.right-m_moveRect.left)/10;
					MouseMove(pt);
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					PostMessage(g_hWnd,WM_FASTMOVE,m_uID,FASTMOVE_RIGHT);
				}
				else if(point.x<m_lastMovePoint3.x-FASTMOVEPIX)
				{
					POINT pt;
					pt.x=m_moveRect.right;
					pt.y=m_moveRect.top;
					LButtonDown(pt);
					pt.x=m_moveRect.right-1;
					pt.y=m_moveRect.top+1;
					MouseMove(pt);
					pt.x=m_moveRect.left+(m_moveRect.right-m_moveRect.left)/2;
					MouseMove(pt);
					pt.x=m_moveRect.left+(m_moveRect.right-m_moveRect.left)/10;
					MouseMove(pt);
					pt.x=m_moveRect.left+1;
					MouseMove(pt);
					PostMessage(g_hWnd,WM_FASTMOVE,m_uID,FASTMOVE_LEFT);
				}
			}
			else
			{
				if(point.y>m_lastMovePoint3.y+FASTMOVEPIX)//下
				{
					POINT pt;
					pt.y=m_moveRect.top;
					pt.x=m_moveRect.right;
					LButtonDown(pt);
					pt.y=m_moveRect.top+1;
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					pt.y=m_moveRect.bottom-(m_moveRect.bottom-m_moveRect.top)/2;
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					pt.y=m_moveRect.bottom-(m_moveRect.bottom-m_moveRect.top)/10;
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					pt.y=m_moveRect.bottom+1;
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					PostMessage(g_hWnd,WM_FASTMOVE,m_uID,FASTMOVE_DOWN);
				}
				else if(point.y<m_lastMovePoint3.y-FASTMOVEPIX)
				{
					POINT pt;
					pt.y=m_moveRect.bottom;
					pt.x=m_moveRect.right;
					LButtonDown(pt);
					pt.y=m_moveRect.bottom-1;
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					pt.y=m_moveRect.top+(m_moveRect.bottom-m_moveRect.top)/2;
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					pt.y=m_moveRect.top+(m_moveRect.bottom-m_moveRect.top)/10;
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					pt.y=m_moveRect.top+1;
					pt.x=m_moveRect.right-1;
					MouseMove(pt);
					PostMessage(g_hWnd,WM_FASTMOVE,m_uID,FASTMOVE_UP);
				}
			}
		}
	}
}

void CPage::SetMoveRect(RECT rctMove)
{
	IntersectRect(&m_moveRect,&m_rct,&rctMove);
}
void CPage::SetMoveDirection(EMoveDirection direction)
{
	m_moveDirection=direction;
}
void CPage::SetMoveStop(int direction)
{
	m_stopDirection=direction;
}
CGrap::CGrap()
{
	m_bFullScreen=false;
	m_pImage=NULL;
	m_pImageChange=NULL;
	m_movable=true;
	m_index=0;
	m_angle=0;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}
CGrap::~CGrap()
{
	g_listGrap=NULL;
	CoUninitialize();
}
void CGrap::SetProp(EGRAPLIST prop)
{
	m_prop=prop;
}
EGRAPLIST CGrap::GetProp()
{
	return m_prop;
}
void CGrap::SetIndex(int index)
{
	m_index=index;
	if(index<0)
	{
		while(m_index<0)
		{
			m_index=g_listGrap->GetItemCount()+m_index;
		}
	}
	else if(index>g_listGrap->GetItemCount()-1)
	{
		while(m_index>g_listGrap->GetItemCount()-1)
		{
			m_index=m_index-g_listGrap->GetItemCount();
		}
	}
}
bool CGrap::LoadGraph(EMoveDirection moveDirection)
{
	m_angle=0;
	if(!g_listGrap->GetItemCount()) return false;

	if(!m_pImage||!m_pImageChange)
	{
		g_ddsd.dwFlags=DDSD_HEIGHT|DDSD_WIDTH;
		g_ddsd.dwWidth=SYSTEMW;
		g_ddsd.dwHeight=SYSTEMH;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImage,NULL)!=DD_OK) return false;
		if(g_pDD->CreateSurface(&g_ddsd,&m_pImageChange,NULL)!=DD_OK) return false;
	}

	//HBITMAP hbm=SHLoadDIBitmap(g_listGrap->GetItemText(m_index));//bmp
	HBITMAP hbm=LoadImageFromFile(g_listGrap->GetItemText(m_index));//bmp jpg等
	if (hbm==NULL) return false;
	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);

	int w,h;
	if(bm.bmWidth>SYSTEMW&&bm.bmHeight>SYSTEMH)
	{
		if((float)bm.bmWidth/SYSTEMW>(float)bm.bmHeight/SYSTEMH)
		{
			w=SYSTEMW;
			h=(float)bm.bmHeight*SYSTEMW/bm.bmWidth;
		}
		else
		{
			h=SYSTEMH;
			w=(float)bm.bmWidth*SYSTEMH/bm.bmHeight;
		}
	}
	else if(bm.bmWidth>SYSTEMW)
	{
		w=SYSTEMW;
		h=(float)bm.bmHeight*SYSTEMW/bm.bmWidth;
	}
	else if(bm.bmHeight>SYSTEMH)
	{
		h=SYSTEMH;
		w=(float)bm.bmWidth*SYSTEMH/bm.bmHeight;
	}
	else
	{
		w=bm.bmWidth;
		h=bm.bmHeight;
	}
	switch(m_prop)
	{
		case GRAP_PREV:
			if(moveDirection==HORIZONTAL)
			SetRect(&m_rct,-w,(SYSTEMH-h)/2,0,(SYSTEMH+h)/2);
			else
			SetRect(&m_rct,(SYSTEMW-w)/2,-h,(SYSTEMW+w)/2,0);
		break;
		case GRAP_MID:
			SetRect(&m_rct,(SYSTEMW-w)/2,(SYSTEMH-h)/2,(SYSTEMW+w)/2,(SYSTEMH+h)/2);
		break;
		case GRAP_NEXT:
			if(moveDirection==HORIZONTAL)
			SetRect(&m_rct,SYSTEMW,(SYSTEMH-h)/2,SYSTEMW+w,(SYSTEMH+h)/2);
			else
			SetRect(&m_rct,(SYSTEMW-w)/2,SYSTEMH,(SYSTEMW+w)/2,SYSTEMH+h);
		break;
	}
	DDCopyBitmap(m_pImage,hbm,0,0,bm.bmWidth,bm.bmHeight);
	DDCopyBitmap(m_pImageChange,hbm,0,0,bm.bmWidth,bm.bmHeight);
	DeleteObject(hbm);
	return true;
}
void CGrap::Draw()
{
	HRESULT hRet;
	DDBLTFX ddbltfx;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwROP=SRCCOPY;

	while (true)
	{
		RECT rct={0};
		SetRect(&rct,0,0,m_rct.right-m_rct.left,m_rct.bottom-m_rct.top);
		hRet=g_pDDSTmp1->Blt(&m_rct,m_pImageChange,&rct,DDBLT_ROP,&ddbltfx);
		hRet=g_pDDSTmp2->Blt(&m_rct,m_pImageChange,&rct,DDBLT_ROP,&ddbltfx);
		if (hRet==DD_OK)
			break;
		if (hRet==DDERR_SURFACELOST)
		{
			hRet=RestoreAll();
			if (hRet!=DD_OK)
				break;
		}
		if (hRet!=DDERR_WASSTILLDRAWING)
			break;
	}
}
void CGrap::LButtonUp(EMoveDirection moveDirection)
{
	if(moveDirection==HORIZONTAL)
	{
		m_bUpdate=true;
		//if(m_rct.left<m_lastDownRCT.left-VIBRATEPIX)//向左
		if(m_rct.left<m_lastDownRCT.left)//向左
		{
			switch(m_prop)
			{
			case GRAP_PREV:
				m_prop=GRAP_NEXT;
				SetIndex(m_index+3);
			break;
			case GRAP_MID:
				{
					m_prop=GRAP_PREV;
					RECT rct2={-(m_rct.right-m_rct.left),(SYSTEMH-(m_rct.bottom-m_rct.top))/2,0,(SYSTEMH+(m_rct.bottom-m_rct.top))/2};
					CopyRect(&m_rct,&rct2);
				}
			break;
			case GRAP_NEXT:
				{
					m_prop=GRAP_MID;
					RECT rct2={(SYSTEMW-(m_rct.right-m_rct.left))/2,(SYSTEMH-(m_rct.bottom-m_rct.top))/2,(SYSTEMW+(m_rct.right-m_rct.left))/2,(SYSTEMH+(m_rct.bottom-m_rct.top))/2};
					RECT rct1={0};
					MidRect(&rct1,m_rct,rct2);
					MoveWindow(rct1.left,rct1.top,rct1.right-rct1.left,rct1.bottom-rct1.top,true);
					CopyRect(&m_rct,&rct2);
				}
			break;
			}
		}
		//else if(m_rct.left>m_lastDownRCT.left+VIBRATEPIX)//向右
		else if(m_rct.left>m_lastDownRCT.left)//向右
		{
			switch(m_prop)
			{
			case GRAP_PREV:
				{
					m_prop=GRAP_MID;
					RECT rct2={(SYSTEMW-(m_rct.right-m_rct.left))/2,(SYSTEMH-(m_rct.bottom-m_rct.top))/2,(SYSTEMW+(m_rct.right-m_rct.left))/2,(SYSTEMH+(m_rct.bottom-m_rct.top))/2};
					RECT rct1={0};
					MidRect(&rct1,m_rct,rct2);
					MoveWindow(rct1.left,rct1.top,rct1.right-rct1.left,rct1.bottom-rct1.top,true);
					CopyRect(&m_rct,&rct2);
				}
			break;
			case GRAP_MID:
				{
					m_prop=GRAP_NEXT;
					RECT rct2={SYSTEMW,(SYSTEMH-(m_rct.bottom-m_rct.top))/2,SYSTEMW+(m_rct.right-m_rct.left),(SYSTEMH+(m_rct.bottom-m_rct.top))/2};
					CopyRect(&m_rct,&rct2);
				}
			break;
			case GRAP_NEXT:
				{
					m_prop=GRAP_PREV;
					SetIndex(m_index-3);
				}
			break;
			}
		}
		else//单击
		{
			m_bUpdate=false;
		}
	}
	else
	{
		m_bUpdate=true;
		//if(m_rct.top<m_lastDownRCT.top-VIBRATEPIX)//向上
		if(m_rct.top<m_lastDownRCT.top)//向上
		{
			switch(m_prop)
			{
			case GRAP_PREV:
				m_prop=GRAP_NEXT;
				SetIndex(m_index+3);
				break;
			case GRAP_MID:
				{
					m_prop=GRAP_PREV;
					RECT rct2={(SYSTEMW-(m_rct.right-m_rct.left))/2,-(m_rct.bottom-m_rct.top),(SYSTEMW+(m_rct.right-m_rct.left))/2,0};
					CopyRect(&m_rct,&rct2);
				}
				break;
			case GRAP_NEXT:
				{
					m_prop=GRAP_MID;
					RECT rct2={(SYSTEMW-(m_rct.right-m_rct.left))/2,(SYSTEMH-(m_rct.bottom-m_rct.top))/2,(SYSTEMW+(m_rct.right-m_rct.left))/2,(SYSTEMH+(m_rct.bottom-m_rct.top))/2};
					RECT rct1={0};
					MidRect(&rct1,m_rct,rct2);
					MoveWindow(rct1.left,rct1.top,rct1.right-rct1.left,rct1.bottom-rct1.top,true);
					CopyRect(&m_rct,&rct2);
				}
				break;
			}
		}
		//else if(m_rct.top>m_lastDownRCT.top+VIBRATEPIX)//向下
		else if(m_rct.top>m_lastDownRCT.top)//向下
		{
			switch(m_prop)
			{
			case GRAP_PREV:
				{
					m_prop=GRAP_MID;
					RECT rct2={(SYSTEMW-(m_rct.right-m_rct.left))/2,(SYSTEMH-(m_rct.bottom-m_rct.top))/2,(SYSTEMW+(m_rct.right-m_rct.left))/2,(SYSTEMH+(m_rct.bottom-m_rct.top))/2};
					RECT rct1={0};
					MidRect(&rct1,m_rct,rct2);
					MoveWindow(rct1.left,rct1.top,rct1.right-rct1.left,rct1.bottom-rct1.top,true);
					CopyRect(&m_rct,&rct2);
				}
				break;
			case GRAP_MID:
				{
					m_prop=GRAP_NEXT;
					RECT rct2={(SYSTEMW-(m_rct.right-m_rct.left))/2,SYSTEMH,(SYSTEMW+(m_rct.right-m_rct.left))/2,SYSTEMH+(m_rct.bottom-m_rct.top)};
					CopyRect(&m_rct,&rct2);
				}
				break;
			case GRAP_NEXT:
				{
					m_prop=GRAP_PREV;
					SetIndex(m_index-3);
				}
				break;
			}
		}
		else//单击
		{
			m_bUpdate=false;
		}
	}
}
void CGrap::UpdateState(EMoveDirection moveDirection)
{
	if(!m_bUpdate) return;
	
	if(moveDirection==HORIZONTAL)
	{
		if(m_prop==GRAP_PREV&&m_rct.left>0)
		{
			LoadGraph(moveDirection);
		}
		if(m_prop==GRAP_NEXT&&m_rct.right<0)
		{
			LoadGraph(moveDirection);
		}
	}
	else
	{
		if(m_prop==GRAP_PREV&&m_rct.top>0)
		{
			LoadGraph(moveDirection);
		}
		if(m_prop==GRAP_NEXT&&m_rct.bottom<0)
		{
			LoadGraph(moveDirection);
		}
	}
}
void CGrap::Rotate(float angle)
{
	m_angle+=angle;
	HBITMAP hbm = NULL;
	IBasicBitmapOps	*oper;
	IImagingFactory *pImgFactory = NULL;
	IImage *pImageBmp = NULL; 
	IBitmapImage *rotebitmap = NULL; 
	if (SUCCEEDED(CoCreateInstance (CLSID_ImagingFactory, NULL,CLSCTX_INPROC_SERVER, IID_IImagingFactory, (void **)&pImgFactory)))
	{
		if(SUCCEEDED(pImgFactory->CreateImageFromFile(g_listGrap->GetItemText(m_index), &pImageBmp)))
		if(SUCCEEDED(pImgFactory->CreateBitmapFromImage(pImageBmp, 0, 0, PixelFormatDontCare, InterpolationHintDefault, &rotebitmap)))
		if(SUCCEEDED(rotebitmap->QueryInterface(IID_IBasicBitmapOps, (void **) &oper)))
		{
			rotebitmap->Release();
			if(SUCCEEDED(oper->Rotate(m_angle, InterpolationHintDefault, &rotebitmap)))
			{
				if(SUCCEEDED(rotebitmap->QueryInterface(IID_IImage,(void **) &pImageBmp)))
				{
					ImageInfo imageInfo;
					if (SUCCEEDED(pImageBmp->GetImageInfo(&imageInfo)))
					{
						HDC hdc = ::GetDC(g_hWnd);
						HDC dcBitmap=CreateCompatibleDC(hdc);;
						hbm = CreateCompatibleBitmap(hdc, imageInfo.Width, imageInfo.Height);
						SelectObject(dcBitmap, hbm);
						RECT rct={0,0,imageInfo.Width,imageInfo.Height};
						pImageBmp->Draw(dcBitmap,&rct,NULL);  
						DeleteDC(dcBitmap);
						pImageBmp->Release();
					}
				}
			}
			oper->Release();
		}
		rotebitmap->Release();
		pImgFactory->Release();
	}


		if (hbm==NULL) return;
		BITMAP bm;
		GetObject(hbm, sizeof(bm), &bm);
		int w,h;
		if(bm.bmWidth>SYSTEMW&&bm.bmHeight>SYSTEMH)
		{
			if((float)bm.bmWidth/SYSTEMW>(float)bm.bmHeight/SYSTEMH)
			{
				w=SYSTEMW;
				h=(float)bm.bmHeight*SYSTEMW/bm.bmWidth;
			}
			else
			{
				h=SYSTEMH;
				w=(float)bm.bmWidth*SYSTEMH/bm.bmHeight;
			}
		}
		else if(bm.bmWidth>SYSTEMW)
		{
			w=SYSTEMW;
			h=(float)bm.bmHeight*SYSTEMW/bm.bmWidth;
		}
		else if(bm.bmHeight>SYSTEMH)
		{
			h=SYSTEMH;
			w=(float)bm.bmWidth*SYSTEMH/bm.bmHeight;
		}
		else
		{
			w=bm.bmWidth;
			h=bm.bmHeight;
		}
		SetRect(&m_rct,(SYSTEMW-w)/2,(SYSTEMH-h)/2,(SYSTEMW+w)/2,(SYSTEMH+h)/2);
		DDCopyBitmap(m_pImageChange,hbm,0,0,bm.bmWidth,bm.bmHeight);
		DeleteObject(hbm);

		g_Page[g_stackPage.GetTop()].Draw();
		g_Page[g_stackPage.GetTop()].UpdatePage(NULL);
}