/**
* @file		XAudio2Context.cpp
* @brief	XAudio2の初期化などを行うクラス
* @author	hsatomura
* @data		20190217
*/
#include "stdafx.h"
#include "XAudio2Context.h"
#include <xaudio2.h>
#include <crtdbg.h>

#pragma comment(lib,"XAudio2.lib")

CXAudio2Context *CXAudio2Context::m_pInstance = NULL;

/**
* @brief	コンストラクタ
*/
CXAudio2Context::CXAudio2Context()
{
	this->m_pXAudio2 = NULL;
	this->m_pMasteringVoice = NULL;
}

/**
* @brief	デストラクタ
*/
CXAudio2Context::~CXAudio2Context()
{
	if (m_pMasteringVoice) {
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = NULL;
	}

	if (m_pXAudio2) {
		m_pXAudio2->Release();
		m_pXAudio2 = NULL;
	}
}

/**
* @brief	初期化
*/
void CXAudio2Context::Initialize()
{
	HRESULT hr;
	TCHAR message[512];
	BOOL bError = false;

	UINT32 Flags = 0;

	//!<	new XAudio2Context
	m_pInstance = new CXAudio2Context();

	CoInitializeEx(NULL,COINITBASE_MULTITHREADED);

	//!<	CreateXAudio2
	if (FAILED(hr = XAudio2Create(&m_pInstance->m_pXAudio2, Flags))) {
		_stprintf_s(message, _countof(message), _T("Failed Created XAudio2 : %#X\n"), hr);
		_RPT0(_CRT_WARN, message);
		bError = true;
	}

	//!<	CreateMasteringVoice
	if (FAILED(hr = m_pInstance->m_pXAudio2->CreateMasteringVoice(&m_pInstance->m_pMasteringVoice))) {
		_stprintf_s(message, _countof(message), _T("ERROR %#X Failed to CreatedMasteringVoice\n"), hr);
		_RPT0(_CRT_WARN, message);
		bError = true;
	}

	
	if (bError) {
		if (m_pInstance->m_pMasteringVoice) {
			m_pInstance->m_pMasteringVoice->DestroyVoice();
			m_pInstance->m_pMasteringVoice = NULL;
		}
		if (m_pInstance->m_pXAudio2) {
			m_pInstance->m_pXAudio2->Release();
			m_pInstance->m_pXAudio2 = NULL;
		}

		CoUninitialize();
	}


}

/**
* @brief	終了処理
*/
void CXAudio2Context::Finalize()
{
	if (m_pInstance) {
		delete m_pInstance;
		m_pInstance = NULL;
	}

	CoUninitialize();
}

/**
* @brief	インスタンス取得
*/
CXAudio2Context *CXAudio2Context::GetInstance()
{
	if (m_pInstance == NULL)
		Initialize();

	return m_pInstance;
}

/**
* @brief	XAudio2取得
*/
IXAudio2 *CXAudio2Context::GetXAudio2Context()
{
	if (m_pXAudio2)
		m_pXAudio2->AddRef();
	
	return m_pXAudio2;
}

/**
* @brief	MasteringVoiceの取得
*/
IXAudio2MasteringVoice *CXAudio2Context::GetMasteringVoice()
{
	return m_pMasteringVoice;
}