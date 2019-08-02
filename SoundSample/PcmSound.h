#pragma once

struct IXAudio2SourceVoice;
class CWaveFile;

class CPcmSound
{
public:
	CPcmSound(const TCHAR *pFilename);
	virtual ~CPcmSound();
	void Play(float Volume);
protected:
	const TCHAR *m_pFilename;		//!<	Filename
	BYTE* m_pBuffer;
	WAVEFORMATEX m_wfx;				//!<	WaveFormatEx
	DWORD m_dwSize;					//!<	WaveLoadSize
};

