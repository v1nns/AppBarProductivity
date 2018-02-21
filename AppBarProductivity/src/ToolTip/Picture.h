// Picture.h: interface for the CPicture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PICTURE_H__6098A4C3_D6D5_4711_BC7B_1595F459B480__INCLUDED_)
#define AFX_PICTURE_H__6098A4C3_D6D5_4711_BC7B_1595F459B480__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPicture  
{
public:
	CPicture();
	virtual ~CPicture();

	bool			Load(LPCTSTR sResourceType, LPCTSTR sResource);
	bool			Load(LPCTSTR sResourceType, UINT nResource);
	bool			Load(LPCTSTR sFileName);
	bool			Draw(CDC* pDC);
	bool			Draw(CDC* pDC, CPoint Pos);
	bool			Draw(CDC* pDC, CPoint Pos, CSize Size);
	bool			Draw(CDC* pDC, double nSizeRatio);
	bool			Draw(CDC* pDC, CPoint Pos, double nSizeRatio);
	bool			Draw(CDC* pDC, int x, int y, int cx, int cy);
	CSize			GetSize(CDC* pDC);
private:
	static	bool	GetResource(UINT nResource, LPCTSTR lpType, BYTE* pResource, int& nBufSize);
	static	bool	GetResource(LPCTSTR lpName, LPCTSTR lpType, BYTE* pResource, int& nBufSize);
	void			UnLoad();
	bool			LoadFromBuffer(BYTE* pBuff, int nSize);
	IPicture*		m_pPicture;
	enum
	{
		HIMETRIC_INCH	= 2540
	};
};

#endif // !defined(AFX_PICTURE_H__6098A4C3_D6D5_4711_BC7B_1595F459B480__INCLUDED_)
