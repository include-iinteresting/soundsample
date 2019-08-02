/**
* @file		PcmCallback.cpp
* @brief	コールバック用クラス
* @author	hsatomura
* @data		20190217
*/

#include "stdafx.h"
#include "PcmCallback.h"


CPcmCallback::CPcmCallback()
{
	m_hBufferEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}


CPcmCallback::~CPcmCallback()
{
	CloseHandle(m_hBufferEndEvent);
}
