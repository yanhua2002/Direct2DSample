#pragma once

#include "FirstTry.h"
#include "UIAnimationHelper.h"

// Event handler object for manager status changes
class CManagerEventHandler :public CUIAnimationManagerEventHandlerBase<CManagerEventHandler>
{
protected:
	FirstTry *m_pFirstTry;
	CManagerEventHandler():m_pFirstTry(NULL)
	{}

public:
	static HRESULT CreateInstance(FirstTry *pFirstTry, IUIAnimationManagerEventHandler **ppManagerEventHandler)throw()
	{
		CManagerEventHandler *pManagerEventHandler;
		HRESULT hr = CUIAnimationCallbackBase::CreateInstance(ppManagerEventHandler, &pManagerEventHandler);

		if (SUCCEEDED(hr))
			pManagerEventHandler->m_pFirstTry = pFirstTry;

		return hr;
	}

	// IUIAnimationManagerEventHandler
	IFACEMETHODIMP OnManagerStatusChanged(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus)
	{
		HRESULT hr = S_OK;
		if (newStatus == UI_ANIMATION_MANAGER_BUSY)
			hr = m_pFirstTry->Invalidate();

		return hr;
	}
};