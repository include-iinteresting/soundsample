/**
* @file		PcmStreamSound.cpp
* @brief	�X�g���[�~���O�Đ��p�N���X
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
* @brief	�R���X�g���N�^
* @param	[in]	pFilename	Wave�t�@�C���̖��O
* @param	[in]	LoopFlag	���[�v�t���O�itrue�Ń��[�v����j
* @details	new ���ꂽ����_beginthread����
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
* @brief	�f�X�g���N�^
*/
CPcmStreamSound::~CPcmStreamSound()
{
	Lock();
	ThreadFlags.m_bThreadActive = false;
	Unlock();
	
	//	�X���b�h���I���܂ł�CRITICAL_SECTION��Delete����Ă��܂��̂�h��
	while (!ThreadFlags.m_bThreadDone) {

	}

	DeleteCriticalSection(&m_CriticalSection);
}



/**
* @brief	�Đ�
* @param	[in]	Volume	�{�����[��
*/
void CPcmStreamSound::Play(float Volume)
{
	SetVolume(Volume);
	EnQueue(SOUND_PLAY);
}

/**
* @brief	�ꎞ��~
*/
void CPcmStreamSound::Stop()
{
	EnQueue(SOUND_STOP);
}

/**
* @brief	��~
*/
void CPcmStreamSound::Done()
{
	EnQueue(SOUND_DONE);
}

/**
* @brief	���ʃZ�b�g
* @param	[in]	Volume	�{�����[���l
* @detail	1.0����@0�Ŗ���
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
* @brief	���ʎ擾
* @return	m_Volume	���݂̃{�����[���l
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
* @brief	���ʃZ�b�g
* @param	[in]	db	�f�V�x���l
* @details	(6��2�{,-6��2����1�{�j0.0f���
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
* @brief	_beginthread��ThreadProc���\�b�h���g�����߂̃��b�p�[
*/
void CPcmStreamSound::ThreadProcLauncher(void* arg)
{
	if (!arg)
		return;

	reinterpret_cast<CPcmStreamSound*>(arg)->ThreadProc();
}

/**
* @brief	�ʃX���b�h�Ŏ��ۂɌĂяo����郁�\�b�h
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
				//!<	�{�C�X�̋󂫂��m�F
				Lock();
				m_pSourceVoice->GetState(&state);
				Unlock();

				if (state.BuffersQueued < MAX_BUFFER_COUNT - 1) {
					//!<	�t�@�C���ǂݍ���
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
				//	���[�v�t���O��true�Ȃ烋�[�v
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
			//!<	�I���҂�
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
			//!<	�I��
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
* @brief	���b�N
*/
void CPcmStreamSound::Lock()
{
	EnterCriticalSection(&m_CriticalSection);
}

/**
* @brief	�A�����b�N
*/
void CPcmStreamSound::Unlock()
{
	LeaveCriticalSection(&m_CriticalSection);
}

/**
* @brief	�L���[�Ƀf�[�^��}��
* @param	[in]	state	�T�E���h�̏��
*/
void CPcmStreamSound::EnQueue(SoundState State)
{
	//	�L���[�����t���ǂ����̊m�F
	if (QUEUE_NEXT(m_Queue.tail) == m_Queue.head)
		return;

	//	�L���[�̖����Ƀf�[�^��}������
	m_Queue.state[m_Queue.tail] = State;

	//	�L���[�̎���}���ʒu�����肷��
	m_Queue.tail = QUEUE_NEXT(m_Queue.tail);
}

/**
* @brief	�L���[����f�[�^�����o��
* @param	[out]	pState	�T�E���h�̏��
*/
void CPcmStreamSound::DeQueue(SoundState *pState)
{
	//	�L���[�Ɏ��o���f�[�^�����݂��邩���m�F����
	if (m_Queue.head == m_Queue.tail)
		return;

	//	�L���[����f�[�^���擾����
	*pState = m_Queue.state[m_Queue.head];

	//	���̃f�[�^�擾�ʒu�����肷��
	m_Queue.head = QUEUE_NEXT(m_Queue.head);
}

