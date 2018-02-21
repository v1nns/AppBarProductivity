#pragma once


// CTotalTasks
#include "Resource.h"

class CTotalTasks : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CTotalTasks)

public:
	CTotalTasks();
	virtual ~CTotalTasks();

protected:
	enum { IDD = IDD_NODETEXT };

	DECLARE_MESSAGE_MAP()
};


