#pragma once

#include <xaudio2.h>

#pragma comment(lib,"XAudio2.lib")

class CPcmCallback : public IXAudio2VoiceCallback
{
public:
	CPcmCallback();
	virtual ~CPcmCallback();

	// コールバック
	virtual void __stdcall OnVoiceProcessingPassStart(UINT32) override {};
	virtual void __stdcall OnVoiceProcessingPassEnd() override {};
	virtual void __stdcall OnStreamEnd() override {};
	virtual void __stdcall OnBufferStart(void*) override {};
	virtual void __stdcall OnBufferEnd(void*) override {
		SetEvent(m_hBufferEndEvent);
	};
	virtual void __stdcall OnLoopEnd(void*) override {};
	virtual void __stdcall OnVoiceError(void*, HRESULT) override {};

protected:
	HANDLE m_hBufferEndEvent;
};

