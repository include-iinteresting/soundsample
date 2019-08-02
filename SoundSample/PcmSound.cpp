/**
* @file		PcmSound.cpp
* @brief	ワンショット再生用クラス
* @author	hsatomura
* @data		20190217
*/
#include "stdafx.h"
#include "PcmSound.h"
#include <xaudio2.h>
#include <crtdbg.h>

#include "XAudio2Context.h"
#include "SDKwavefile.h"

#pragma comment(lib,"XAudio2.lib")


/**
* @brief	コンストラクタ
*/
CPcmSound::CPcmSound(const TCHAR *pFilename)
{
	m_pFilename = pFilename;
	
	DWORD dwWaveLength = 0;
	m_dwSize = 0;

	//	new CWaveFileObject
	CWaveFile *pWaveFile = new CWaveFile();
	//	WaveFileOpen
	pWaveFile->Open((LPWSTR)pFilename, &m_wfx, WAVEFILE_READ);
	//	GetFormat
	m_wfx = *pWaveFile->GetFormat();
	//	GetWaveFileSize
	dwWaveLength = pWaveFile->GetSize();

	m_pBuffer = new BYTE[dwWaveLength];
	//	WaveFileRead
	pWaveFile->Read(m_pBuffer, dwWaveLength, &m_dwSize);

	if (pWaveFile) {
		pWaveFile->Close();
		delete pWaveFile;
		pWaveFile = NULL;
	}
}

/**	
* @brief	デストラクタ
*/
CPcmSound::~CPcmSound()
{
	if (m_pBuffer) {
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
}

/**
* @brief	ワンショット再生
* @param	[in]	pFilename	ファイルネーム
* @param	[in]	Volume		ボリューム（1.0fが基準で0.0fで無音）
*/
void CPcmSound::Play(float Volume = 1.0f)
{
	HRESULT hr;								//	HRESULT
	TCHAR message[512];						//	ErrorMessage
	IXAudio2SourceVoice *pSourceVoice;		//	IXAudio2SourceVoice
	IXAudio2 *pXAudio2;						//	IXAudio2


	// GetXAudio2
	pXAudio2 = CXAudio2Context::GetInstance()->GetXAudio2Context();
	// CreateSourceVoice
	if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, &m_wfx, 0, 1.0f, NULL))) {
		_stprintf_s(message, _countof(message), _T("Failed to created SourceVoie: %#X\n"), hr);
		_RPT0(_CRT_ERROR, message);
	}

	pSourceVoice->Start();
	pSourceVoice->SetVolume(Volume);

	XAUDIO2_BUFFER buf = { 0 };
	buf.AudioBytes = m_dwSize;
	buf.pAudioData = m_pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	pSourceVoice->SubmitSourceBuffer(&buf);

	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);

	if (state.BuffersQueued == 0) {
		if (pSourceVoice) {
			pSourceVoice->Stop();
			pSourceVoice->DestroyVoice();
			pSourceVoice = NULL;
		}
	}

	
}