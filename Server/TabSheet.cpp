#include "pch.h"
#include "TabSheet.h"

BEGIN_MESSAGE_MAP(CTabSheet, CTabCtrl)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

CTabSheet::CTabSheet() :m_nPageNum(0), m_nCurrentPage(0)
{
	// �ṹ�������ʼ��0
	memset(&m_tsPage, NULL, sizeof(TAB_SHEET_PAGE));
}

void CTabSheet::OnLButtonDown(UINT nFlags, CPoint point)
{
	CTabCtrl::OnLButtonDown(nFlags, point);

	int selectedPage = GetCurFocus();
	if (m_nCurrentPage != selectedPage)
	{
		// ѡ��ı�
		// ���õ�ǰѡ�
		m_tsPage[m_nCurrentPage].pDialog->ShowWindow(SW_HIDE);

		// ��ʾѡ��ѡ�
		m_nCurrentPage = selectedPage;
		m_tsPage[m_nCurrentPage].pDialog->ShowWindow(SW_SHOW);
	}
}

BOOL CTabSheet::AddPage(LPCTSTR title, CDialog* pDialog, UINT ID, bool bEnable)
{
	if (m_nPageNum == nMaxPage)
	{
		return FALSE;
	}

	m_tsPage[m_nPageNum].pDialog = pDialog;
	m_tsPage[m_nPageNum].nIDD = ID;
	m_tsPage[m_nPageNum].Title = title;
	m_nPageNum++;

	return TRUE;
}

BOOL CTabSheet::Show()
{
	if (m_nPageNum == 0)
	{
		// û��ѡ�
		return FALSE;
	}

	for (int i = 0; i < m_nPageNum; i++)
	{
		// �����Ի���
		// �ظ������Ի����ֱ�ӱ�
		m_tsPage[i].pDialog->Create(m_tsPage[i].nIDD, this);

		// ����ѡ�
		InsertItem(i, m_tsPage[i].Title);
	}

	// �ǵ�ǰҳ�����ʾ�����Ǿ�����
	for (int i = 0; i < m_nPageNum; i++)
	{
		if (i == m_nCurrentPage)
		{
			m_tsPage[i].pDialog->ShowWindow(SW_SHOW);
		}
		else
		{
			m_tsPage[i].pDialog->ShowWindow(SW_HIDE);
		}
	}

	SetRect();		// ���öԻ���
	return TRUE;
}

void CTabSheet::SetRect()
{
	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;		//�󡢶�������

	GetClientRect(&tabRect);		// ��ȡ����Tab�ؼ���λ�ô�С��
	GetItemRect(0, &itemRect);		// itemָ��ֻ�Ǳ�ǩҳ�������ǻ�ñ�ǩҳ��λ�ô�С

	DWORD style = GetStyle();
	if (style & TCS_VERTICAL)
	{
		// ѡ���TAB�ؼ��Ĳ��
		nY = tabRect.top + nOffset;
		nYc = tabRect.bottom - nY - (nOffset + 1);
		if (style & TCS_BOTTOM)
		{
			// ѡ����ұ�
			nX = tabRect.left + nOffset;
			nXc = itemRect.left - nX - (nOffset + 1);
		}
		else
		{
			// ѡ������
			nX = itemRect.right + nOffset;
			nXc = tabRect.right - nX - (nOffset + 1);
		}
	}
	else
	{
		// ѡ���TAB�ؼ��Ķ�����ײ�
		nX = tabRect.left + nOffset;
		nXc = tabRect.right - nX - (nOffset + 1);
		if (style & TCS_BOTTOM)
		{
			// ѡ��ڵײ�
			nY = tabRect.top + nOffset;
			nYc = itemRect.top - nY - (nOffset + 1);
		}
		else
		{
			// ѡ��ڶ���
			nY = itemRect.bottom + nOffset;
			nYc = tabRect.bottom - nY - (nOffset + 1);
		}
	}
	// �ƶ�ȫ���Ի�����Ϊֻ�е�ǰѡ���Ӧ�ĶԻ�����ʾ�����Բ����ĶԻ����ص�
	for (int i = 0; i < m_nPageNum; i++)
	{
		// ��֪��Ϊʲô��WindowPos�����ʾ��ֻ�ø���MoveWindow
		m_tsPage[i].pDialog->MoveWindow(nX, nY, nXc, nYc);
	}
}
