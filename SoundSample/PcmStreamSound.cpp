/**
* @file		PcmStreamSound.cpp
* @brief	ストリーミング再生用クラス
* @author	hsatomura
* @data		20190217
* @data		20190317
*/
#include "stdafx.h"
#include "PcmStreamSound.h"
#include <process.h>
#include <crtdbg.h>

#include "SDKwavefile.h"
#include "XAudio2Context.h"
#include "PcmCallback.h"

/**
* @brief	コンストラクタ
* @param	[in]	pFilename	Waveファイルの名前
* @param	[in]	LoopFlag	ループフラグ（trueでループする）
* @details	new された時に_beginthreadする
*/
CPcmStreamSound::CPcmStreamSound(const TCHAR *pFilename, BOOL bLoopFlag)
{
	m_pSourceVoice = NULL;
	m_pFilename = pFilename;
	m_bLoopFlag = bLoopFlag;
	m_Volume = 1.0f;

	for (int i = 0; i < MAX_BUFFER_COUNT; ++i) {
		memset(m_Buffers[i], NULL, sizeof(m_Buffers[i]));
	}
	m_dwCurrentPosition = 0;
	m_dwWaveLength = 0;
	m_dwCurrentReadBuffer = 0;
	m_StreamPhase = StreamPhase::STREAMPHASE_PREPARE;

	memset(m_Queue.state, NULL, sizeof(m_Queue.state));
	m_Queue.head = 0;
	m_Queue.tail = 0;

	ThreadFlags.m_bThreadActive = true;
	ThreadFlags.m_bStopCommand = false;
	ThreadFlags.m_bThreadDone = false;

	InitializeCriticalSection(&m_CriticalSection);

	_beginthread(ThreadProcLauncher, 0, this);
}

/**
* @brief	デストラクタ
*/
CPcmStreamSound::~CPcmStreamSound()
{
	Lock();
	ThreadFlags.m_bThreadActive = false;
	Unlock();
	
	//	スレッドが終わるまでにCRITICAL_SECTIONがDeleteされてしまうのを防ぐ
	while (!ThreadFlags.m_bThreadDone) {

	}

	DeleteCriticalSection(&m_CriticalSection);
}



/**
* @brief	再生
* @param	[in]	Volume	ボリューム
*/
void CPcmStreamSound::Play(float Volume)
{
	SetVolume(Volume);
	EnQueue(SOUND_PLAY);
}

/**
* @brief	一時停止
*/
void CPcmStreamSound::Stop()
{
	EnQueue(SOUND_STOP);
}

/**
* @brief	停止
*/
void CPcmStreamSound::Done()
{
	EnQueue(SOUND_DONE);
}

/**
* @brief	音量セット
* @param	[in]	Volume	ボリューム値
* @detail	1.0が基準　0で無音
*/
void CPcmStreamSound::SetVolume(float Volume)
{
	Lock();
	if (m_pSourceVoice) {
		if (Volume != m_Volume) {
			m_pSourceVoice->SetVolume(Volume);
			m_Volume = Volume;
		}
	}
	Unlock();
}


/**
* @brief	音量取得
* @return	m_Volume	現在のボリューム値
*/
float CPcmStreamSound::GetVolume()
{
	Lock();
	if (m_pSourceVoice)
		m_pSourceVoice->GetVolume(&m_Volume);
	Unlock();
	return m_Volume;
}

/**
* @brief	音量セット
* @param	[in]	db	デシベル値
* @details	(6が2倍,-6が2分の1倍）0.0fが基準
*/
void CPcmStreamSound::SetVolumeDB(float db) 
{
	Lock();
	if (m_pSourceVoice) {
		float Volume = XAudio2DecibelsToAmplitudeRatio(db);
		if (Volume != m_Volume) {
			m_pSourceVoice->SetVolume(Volume);
			m_Volume = Volume;
		}
	}
	Unlock();
}





/**********private**********/

/**
* @brief	_beginthreadでThreadProcメソッドを使うためのラッパー
*/
void CPcmStreamSound::ThreadProcLauncher(void* arg)
{
	if (!arg)
		return;

	reinterpret_cast<CPcmStreamSound*>(arg)->ThreadProc();
}

/**
* @brief	別スレッドで実際に呼び出されるメソッド
*/
void CPcmStreamSound::ThreadProc()
{
	HRESULT hr = S_OK;
	TCHAR message[512];

	CWaveFile *pWaveFile = NULL;
	WAVEFORMATEX wfx = { 0 };

	IXAudio2 *pXAudio2 = NULL;
	CPcmCallback *pContext = NULL;
	BOOL bStop;

	while (ThreadFlags.m_bThreadActive) {
		bStop = (ThreadFlags.m_bStopCommand == 0);

		switch (m_StreamPhase) {
		case STREAMPHASE_PREPARE:
			pWaveFile = new CWaveFile();
			//!<	FileOpen
			pWaveFile->Open((LPWSTR)m_pFilename, &wfx, WAVEFILE_READ);
			//!<	GetFormat
			wfx = *pWaveFile->GetFormat();
			//!<	GetWaveSize
			m_dwWaveLength = pWaveFile->GetSize();
			//!<	New CallbackClass
			pContext = new CPcmCallback();

			//!< GetXAudio2
			while (pXAudio2 == NULL)
				pXAudio2 = CXAudio2Context::GetInstance()->GetXAudio2Context();
			
			//!< CreateSourceVoice
			Lock();
			while (m_pSourceVoice == NULL)
				pXAudio2->CreateSourceVoice(&m_pSourceVoice, &wfx, XAUDIO2_VOICE_USEFILTER, 1.0f, pContext);
			Unlock();
			if (FAILED(hr)) {
				_stprintf_s(message, _countof(message), _T("Failed to created SourceVoie: %#X\n"), hr);
				_RPT0(_CRT_ERROR, message);
				m_StreamPhase = STREAMPHASE_DONE;
				break;
			}

			m_StreamPhase = StreamPhase::STREAMPHASE_RUN;
		case STREAMPHASE_RUN:

			if (m_dwCurrentPosition < m_dwWaveLength) {
				DWORD read;
				XAUDIO2_VOICE_STATE state;
				//!<	ボイスの空きを確認
				Lock();
				m_pSourceVoice->GetState(&state);
				Unlock();

				if (state.BuffersQueued < MAX_BUFFER_COUNT - 1) {
					//!<	ファイル読み込み
					DWORD cbValid = min(STREAMING_BUFFER_SIZE, m_dwWaveLength - m_dwCurrentPosition);
					pWaveFile->Read(m_Buffers[m_dwCurrentReadBuffer], cbValid, &read);
					m_dwCurrentPosition += read;

					XAUDIO2_BUFFER buf = { 0 };
					buf.AudioBytes = cbValid;
					buf.pAudioData = m_Buffers[m_dwCurrentReadBuffer];
					if (m_dwCurrentPosition >= m_dwWaveLength)
						buf.Flags = XAUDIO2_END_OF_STREAM;
					Lock();
					m_pSourceVoice->SubmitSourceBuffer(&buf);
					Unlock();

					m_dwCurrentReadBuffer = (m_dwCurrentReadBuffer + 1) % MAX_BUFFER_COUNT;
				}
			}
			else {
				//	ループフラグがtrueならループ
				if (m_bLoopFlag) {
					pWaveFile->ResetFile();
					m_dwCurrentPosition = 0;
					break;
				}
				else {
					m_StreamPhase = STREAMPHASE_WAIT;
				}
			}

			DeQueue(&m_SoundState);
			switch (m_SoundState) {
			case SOUND_PLAY:
				Lock();
				m_pSourceVoice->Start();
				Unlock();
				break;
			case SOUND_STOP:
				Lock();
				m_pSourceVoice->Stop(XAUDIO2_PLAY_TAILS);
				Unlock();
				break;
			case SOUND_DONE:
				Lock();
				m_pSourceVoice->Stop();
				m_pSourceVoice->FlushSourceBuffers();
				pWaveFile->ResetFile();
				m_dwCurrentPosition = 0;
				Unlock();
				break;
			}

			break;
		case STREAMPHASE_WAIT:
			//!<	終了待ち
			if (!ThreadFlags.m_bStopCommand) {
				XAUDIO2_VOICE_STATE state;
				Lock();
				m_pSourceVoice->GetState(&state);
				Unlock();
				if (state.BuffersQueued > 0)
					break;
			}
			Lock();
			m_pSourceVoice->Stop(0);
			m_pSourceVoice->DestroyVoice();
			m_pSourceVoice = NULL;
			Unlock();
			m_StreamPhase = STREAMPHASE_DONE;
			break;
		case STREAMPHASE_DONE:
			//!<	終了
			ThreadFlags.m_bThreadActive = false;
			break;
		}

	}

	if (pContext) {
		delete pContext;
		pContext = NULL;
	}

	if (pWaveFile) {
		pWaveFile->Close();
		delete pWaveFile;
		pWaveFile = NULL;
	}


	if (m_pSourceVoice) {
		Lock();
		m_pSourceVoice->Stop(0);
		m_pSourceVoice->DestroyVoice();
		m_pSourceVoice = NULL;
		Unlock();
	}

	ThreadFlags.m_bThreadDone = true;
}

/**
* @brief	ロック
*/
void CPcmStreamSound::Lock()
{
	EnterCriticalSection(&m_CriticalSection);
}

/**
* @brief	アンロック
*/
void CPcmStreamSound::Unlock()
{
	LeaveCriticalSection(&m_CriticalSection);
}

/**
* @brief	キューにデータを挿入
* @param	[in]	state	サウンドの状態
*/
void CPcmStreamSound::EnQueue(SoundState State)
{
	//	キューが満杯かどうかの確認
	if (QUEUE_NEXT(m_Queue.tail) == m_Queue.head)
		return;

	//	キューの末尾にデータを挿入する
	m_Queue.state[m_Queue.tail] = State;

	//	キューの次回挿入位置を決定する
	m_Queue.tail = QUEUE_NEXT(m_Queue.tail);
}

/**
* @brief	キューからデータを取り出す
* @param	[out]	pState	サウンドの状態
*/
void CPcmStreamSound::DeQueue(SoundState *pState)
{
	//	キューに取り出すデータが存在するかを確認する
	if (m_Queue.head == m_Queue.tail)
		return;

	//	キューからデータを取得する
	*pState = m_Queue.state[m_Queue.head];

	//	次のデータ取得位置を決定する
	m_Queue.head = QUEUE_NEXT(m_Queue.head);
}

