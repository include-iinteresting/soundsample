/**
* @file		SoundManager.cpp
* @brief	ゲームに使う音楽を管理するためのクラス
* @author	hsatmura
* @data		20190222
*/
#include "stdafx.h"
#include "SoundManager.h"
#include "PcmStreamSound.h"
#include "PcmSound.h"
#include "XAudio2Context.h"

#define SAFE_RELEASE(o) if (o){ (o)->Release(); o = NULL; };
#define SAFE_DELETE(o) if (o){ delete (o); o = NULL; };
#define SAFE_DELETE_ARRAY(o) if (o){ delete[] (o); o = NULL; };

CSoundManager *CSoundManager::m_pInstance = NULL;

/**
* @brief	コンストラクタ
*/
CSoundManager::CSoundManager()
{
	m_pPcmStreamSounds.clear();
	m_pPcmSounds.clear();
}

/**
* @brief	デストラクタ
*/
CSoundManager::~CSoundManager()
{
	for (std::vector<CPcmStreamSound*>::iterator it = m_pPcmStreamSounds.begin(); it != m_pPcmStreamSounds.end(); ++it) {
		SAFE_DELETE(*it);
	}
	for (std::vector<CPcmSound*>::iterator it = m_pPcmSounds.begin(); it != m_pPcmSounds.end(); ++it) {
		SAFE_DELETE(*it);
	}

	m_pPcmStreamSounds.clear();
	m_pPcmSounds.clear();

	CXAudio2Context::Finalize();
}

/**
* @brief	初期化
*/
void CSoundManager::Initialize()
{
	m_pInstance = new CSoundManager();
}

/**
* @brief	終了処理
*/
void CSoundManager::Finalize()
{
	SAFE_DELETE(m_pInstance);
}

/**
* @brief	インスタンス取得
* @return	CSoundManager Object
*/
CSoundManager *CSoundManager::GetInstance()
{
	if (m_pInstance == NULL)
		Initialize();

	return m_pInstance;
}

/**
* @brief	ロード
* @param	[in]	pFilename	ファイルネーム
* @param	[in]	bLoopFlag	ループフラグ(trueでループする)
*/
void CSoundManager::LoadStreamSound(const TCHAR *pFilename, BOOL bLoopFlag)
{
	CSoundManager *pInstance = GetInstance();
	pInstance->m_pPcmStreamSounds.push_back(new CPcmStreamSound(pFilename, bLoopFlag));
}

/**
* @brief	再生
* @param	[in]	id	再生する曲の配列の要素番号
* @return	id が配列の要素番号であれば true
*/
BOOL CSoundManager::PlayStreamSound(UINT id, float Volume)
{
	CSoundManager *pInstance = GetInstance();
	if (id < pInstance->m_pPcmStreamSounds.size()) {
		pInstance->m_pPcmStreamSounds[id]->Play(Volume);
		return true;
	}
	else if (id == CSoundManager::ALL) {
		for (unsigned int i = 0; i < pInstance->m_pPcmStreamSounds.size(); ++i) {
			pInstance->m_pPcmStreamSounds[i]->Play(Volume);
		}
		return true;
	}
	return false;
}

/**
* @brief	一時停止
* @param	[in]	id	一時停止する曲の配列の要素番号
* @return	id が配列の要素番号であれば true
*/
BOOL CSoundManager::StopStreamSound(UINT id)
{
	CSoundManager *pInstance = GetInstance();
	if (id < pInstance->m_pPcmStreamSounds.size()) {
		pInstance->m_pPcmStreamSounds[id]->Stop();
		return true;
	}
	else if (id == CSoundManager::ALL) {
		for (unsigned int i = 0; i < pInstance->m_pPcmStreamSounds.size(); ++i) {
			pInstance->m_pPcmStreamSounds[i]->Stop();
		}
		return true;
	}
	return false;
}

/**
* @brief	停止
* @param	[in]	id	停止する曲の配列の要素番号
* @return	id が配列の要素番号であれば true
*/
BOOL CSoundManager::DoneStreamSound(UINT id)
{
	CSoundManager *pInstance = GetInstance();
	if (id < pInstance->m_pPcmStreamSounds.size()) {
		pInstance->m_pPcmStreamSounds[id]->Done();
		return true;
	}
	else if (id == CSoundManager::ALL) {
		for (unsigned int i = 0; i < pInstance->m_pPcmStreamSounds.size(); ++i) {
			pInstance->m_pPcmStreamSounds[i]->Done();
		}
		return true;
	}
	return false;
}

/**
* @brief	ボリュームセット
* @param	[in]	id	ボリュームをセットする曲の配列の要素番号
* @param	[in]	Volume	セットするボリューム
* @return	id が配列の要素番号であれば true
*/
BOOL CSoundManager::SetVolumeStreamSound(UINT id, float Volume)
{
	CSoundManager *pInstance = GetInstance();
	if (id < pInstance->m_pPcmStreamSounds.size()) {
		pInstance->m_pPcmStreamSounds[id]->SetVolume(Volume);
		return true;
	}
	else if (id == CSoundManager::ALL) {
		for (unsigned int i = 0; i < pInstance->m_pPcmStreamSounds.size(); ++i) {
			pInstance->m_pPcmStreamSounds[i]->SetVolume(Volume);
		}
		return true;
	}
	return false;
}

/**
* @brief	ボリューム取得
* @param	[in]	id	ボリュームを取得する曲の配列の要素番号
* @return	id が配列の要素番号であれば 指定した曲の現在の音が返る
*/
float CSoundManager::GetVolumeStreamSound(UINT id)
{
	CSoundManager *pInstance = GetInstance();
	if (id < pInstance->m_pPcmStreamSounds.size()) {
		return pInstance->m_pPcmStreamSounds[id]->GetVolume();
	}
	else {
		return -1.0f;
	}
}

/**
* @brief	ボリュームセット(デシベル)
* @param	[in]	id	ボリュームをセットする曲の配列の要素番号
* @param	[in]	Volume	セットするデシベル値
* @return	id が配列の要素番号であれば true
*/
BOOL CSoundManager::SetVolumeDBStreamSound(UINT id, float db)
{
	CSoundManager *pInstance = GetInstance();
	if (id < pInstance->m_pPcmStreamSounds.size()) {
		pInstance->m_pPcmStreamSounds[id]->SetVolumeDB(db);
		return true;
	}
	else if (id == CSoundManager::ALL) {
		for (unsigned int i = 0; i < pInstance->m_pPcmStreamSounds.size(); ++i) {
			pInstance->m_pPcmStreamSounds[i]->SetVolumeDB(db);
		}
		return true;
	}
	return false;
}


/**
* @brief	ワンショット音楽の読み込み
* @param	[in]	pFilename	ファイルネーム
*/
void CSoundManager::LoadOneShot(const TCHAR *pFilename)
{
	CSoundManager *pInstance = GetInstance();
	pInstance->m_pPcmSounds.push_back(new CPcmSound(pFilename));
}

/**
* @brief	ワンショット音楽の再生
* @param	[in]	id	再生したい音楽の格納されている配列の要素番号
* @param	[in]	Volume	音量
* @return	id が配列の要素番号であれば true
*/
BOOL CSoundManager::PlayOneShot(UINT id, float Volume)
{
	CSoundManager *pInstance = GetInstance();
	if (id < pInstance->m_pPcmSounds.size()) {
		pInstance->m_pPcmSounds[id]->Play(Volume);
		return true;
	}
	else if (id == CSoundManager::ALL) {
		for (int i = 0; i < pInstance->m_pPcmSounds.size(); ++i) {
			pInstance->m_pPcmSounds[i]->Play(Volume);
		}
		return true;
	}
	return false;
}