#pragma once

#ifndef __SOUNDMANAGER_H__
#define __SOUNDMANAGER_H__

#include <vector>

class CPcmStreamSound;
class CPcmSound;


class CSoundManager
{
private:
	CSoundManager();
	virtual ~CSoundManager();
public:
	static void Initialize();
	static void Finalize();
	static CSoundManager * GetInstance();

	static void LoadStreamSound(const TCHAR *pFilename,BOOL LoopFlag);
	static BOOL PlayStreamSound(UINT id, float Volume);
	static BOOL StopStreamSound(UINT id);
	static BOOL DoneStreamSound(UINT id);
	static BOOL SetVolumeStreamSound(UINT id, float Volume);
	static float GetVolumeStreamSound(UINT id);
	static BOOL SetVolumeDBStreamSound(UINT id, float db);
	
	static void LoadOneShot(const TCHAR *pFilename);
	static BOOL PlayOneShot(UINT id, float Volume);

	static const UINT ALL = 0xffffff;
protected:
	static CSoundManager * m_pInstance;					//	CSoundManagerObjectInstance
	std::vector<CPcmStreamSound*> m_pPcmStreamSounds;	//	Streaming
	std::vector<CPcmSound*> m_pPcmSounds;				//	OneShot
};

#endif