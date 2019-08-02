#pragma once

#define XAUDIO2_HELPER_FUNCTIONS

#define MAX_BUFFER_COUNT 3
#define STREAMING_BUFFER_SIZE 65536

#define QUEUE_SIZE 7
#define QUEUE_NEXT(o) (((o) + 1) % QUEUE_SIZE) 

//!<	�X�g���[���̏��
enum StreamPhase {
	STREAMPHASE_PREPARE = 0,	//!<	����
	STREAMPHASE_RUN	 = 1,		//!<	����
	STREAMPHASE_WAIT = 2,		//!<	�I���҂�
	STREAMPHASE_DONE = 3		//!<	�I��
};

//!<	�T�E���h�̏��
enum SoundState {
	SOUND_PLAY = 1,				//!<	�Đ�
	SOUND_STOP = 2,				//!<	�ꎞ��~
	SOUND_DONE = 3				//!<	��~
};

//!<	�T�E���h�ɑ΂��閽�߂��󂯂郊���O�o�b�t�@
typedef struct {
	SoundState state[QUEUE_SIZE];
	short head;
	short tail;
}SoundQueue;

struct IXAudio2SourceVoice;

class CPcmStreamSound
{
public:
	CPcmStreamSound(const TCHAR *pFilename, BOOL LoopFlag);
	virtual ~CPcmStreamSound();
	void Play(float Volume);									//!<	NormalPlay
	void Stop();												//!<	Stop
	void Done();												//!<	Done
	void SetVolume(float Volume);								//!<	NormalVolumeSet
	float GetVolume();											//!<	NormalVolumeGet
	void SetVolumeDB(float db);									//!<	DecibelsVolumeSet
private:
	static void ThreadProcLauncher(void* arg);					//!<	ThreadProcLapper
	void ThreadProc();											//!<	ThreadProc
	void Lock();												//!<	CriticalSectionLock
	void Unlock();												//!<	CriticalSectionUnlock
	void EnQueue(SoundState state);								//!<	RingBufferEnQueue
	void DeQueue(SoundState *state);							//!<	RingBufferDeQueue

protected:
	IXAudio2SourceVoice * m_pSourceVoice;						//!<	SourceVoice
	const TCHAR *m_pFilename;									//!<	Filename
	BOOL m_bLoopFlag;											//!<	LoopFlag
	float m_Volume;												//!<	Volume

	StreamPhase m_StreamPhase;									//!<	StreamPhase
	BYTE m_Buffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];	//!<	PcmDataBuffers
	DWORD m_dwCurrentPosition;									//!<	CurrentPosition
	DWORD m_dwWaveLength;										//!<	WaveLength
	DWORD m_dwCurrentReadBuffer;								//!<	ReadCurrentBuffer

	SoundQueue m_Queue;											//!<	SoundQueue
	SoundState m_SoundState;									//!<	SoundState

	
	struct {
		BOOL m_bThreadActive;
		BOOL m_bThreadDone;
		BOOL m_bStopCommand;
	}ThreadFlags;												//!<	ThreadFlags

	CRITICAL_SECTION m_CriticalSection;							//!<	CriticalSection
};

