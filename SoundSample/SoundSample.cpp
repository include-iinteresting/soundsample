// SoundSample.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include "SoundManager.h"


void TextRender();

int main()
{
	CSoundManager::LoadOneShot(_T("res/test.wav"));
	CSoundManager::LoadStreamSound(_T("res/testbgm.wav"),true);
	bool bFlag = false;
	TextRender();

	while (true) {
		if (!bFlag) {
			if (GetAsyncKeyState('1')) {
				CSoundManager::PlayOneShot(0, 1.0f);
				TextRender();
				bFlag = true;
			}
			else if (GetAsyncKeyState('2')) {
				CSoundManager::PlayStreamSound(0, 1.0f);
				TextRender();
				bFlag = true;
			}
			else if (GetAsyncKeyState('3')) {
				CSoundManager::StopStreamSound(0);
				TextRender();
				bFlag = true;
			}
			else if (GetAsyncKeyState('4')) {
				CSoundManager::DoneStreamSound(0);
				TextRender();
				bFlag = true;
			}
			else if (GetAsyncKeyState('5')) {
				float volume = CSoundManager::GetVolumeStreamSound(0);
				CSoundManager::SetVolumeStreamSound(0, volume += 0.1f);
				TextRender();
				bFlag = true;
			}
			else if (GetAsyncKeyState('6')) {
				float volume = CSoundManager::GetVolumeStreamSound(0);
				CSoundManager::SetVolumeStreamSound(0, volume -= 0.1f);
				TextRender();
				bFlag = true;
			}
			else if (GetAsyncKeyState(VK_ESCAPE)) {
				break;
			}
		}
		else {
			if (!GetAsyncKeyState('1') && !GetAsyncKeyState('2') && !GetAsyncKeyState('3') && !GetAsyncKeyState('4') && !GetAsyncKeyState('5') && !GetAsyncKeyState('6'))
				bFlag = false;
		}
	}

	CSoundManager::Finalize();

    return 0;
}


/**
* @brief	文字表示関数
*/
void TextRender() 
{
	printf("\n1:効果音再生\n2:BGM再生\n3:BGM一時停止\n4:BGM完全停止\n5:BGM音量アップ\n6:BGM音量ダウン\nesc:終了\n");
}

