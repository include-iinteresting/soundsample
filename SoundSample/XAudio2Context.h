#pragma once

struct IXAudio2;
struct IXAudio2MasteringVoice;



class CXAudio2Context
{
public:
	static void Initialize();
	static void Finalize();
	static CXAudio2Context * GetInstance();
	IXAudio2 *GetXAudio2Context();
	IXAudio2MasteringVoice *GetMasteringVoice();
private:
	CXAudio2Context();
	virtual ~CXAudio2Context();
protected:
	static CXAudio2Context * m_pInstance;
	IXAudio2 *m_pXAudio2;
	IXAudio2MasteringVoice *m_pMasteringVoice;
};

