/**
* @file		PcmCallback.cpp
* @brief	�R�[���o�b�N�p�N���X
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
