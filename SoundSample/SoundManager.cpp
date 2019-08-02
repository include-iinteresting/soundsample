/**
* @file		SoundManager.cpp
* @brief	�Q�[���Ɏg�����y���Ǘ����邽�߂̃N���X
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
* @brief	�R���X�g���N�^
*/
CSoundManager::CSoundManager()
{
	m_pPcmStreamSounds.clear();
	m_pPcmSounds.clear();
}

/**
* @brief	�f�X�g���N�^
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
* @brief	������
*/
void CSoundManager::Initialize()
{
	m_pInstance = new CSoundManager();
}

/**
* @brief	�I������
*/
void CSoundManager::Finalize()
{
	SAFE_DELETE(m_pInstance);
}

/**
* @brief	�C���X�^���X�擾
* @return	CSoundManager Object
*/
CSoundManager *CSoundManager::GetInstance()
{
	if (m_pInstance == NULL)
		Initialize();

	return m_pInstance;
}

/**
* @brief	���[�h
* @param	[in]	pFilename	�t�@�C���l�[��
* @param	[in]	bLoopFlag	���[�v�t���O(true�Ń��[�v����)
*/
void CSoundManager::LoadStreamSound(const TCHAR *pFilename, BOOL bLoopFlag)
{
	CSoundManager *pInstance = GetInstance();
	pInstance->m_pPcmStreamSounds.push_back(new CPcmStreamSound(pFilename, bLoopFlag));
}

/**
* @brief	�Đ�
* @param	[in]	id	�Đ�����Ȃ̔z��̗v�f�ԍ�
* @return	id ���z��̗v�f�ԍ��ł���� true
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
* @brief	�ꎞ��~
* @param	[in]	id	�ꎞ��~����Ȃ̔z��̗v�f�ԍ�
* @return	id ���z��̗v�f�ԍ��ł���� true
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
* @brief	��~
* @param	[in]	id	��~����Ȃ̔z��̗v�f�ԍ�
* @return	id ���z��̗v�f�ԍ��ł���� true
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
* @brief	�{�����[���Z�b�g
* @param	[in]	id	�{�����[�����Z�b�g����Ȃ̔z��̗v�f�ԍ�
* @param	[in]	Volume	�Z�b�g����{�����[��
* @return	id ���z��̗v�f�ԍ��ł���� true
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
* @brief	�{�����[���擾
* @param	[in]	id	�{�����[�����擾����Ȃ̔z��̗v�f�ԍ�
* @return	id ���z��̗v�f�ԍ��ł���� �w�肵���Ȃ̌��݂̉����Ԃ�
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
* @brief	�{�����[���Z�b�g(�f�V�x��)
* @param	[in]	id	�{�����[�����Z�b�g����Ȃ̔z��̗v�f�ԍ�
* @param	[in]	Volume	�Z�b�g����f�V�x���l
* @return	id ���z��̗v�f�ԍ��ł���� true
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
* @brief	�����V���b�g���y�̓ǂݍ���
* @param	[in]	pFilename	�t�@�C���l�[��
*/
void CSoundManager::LoadOneShot(const TCHAR *pFilename)
{
	CSoundManager *pInstance = GetInstance();
	pInstance->m_pPcmSounds.push_back(new CPcmSound(pFilename));
}

/**
* @brief	�����V���b�g���y�̍Đ�
* @param	[in]	id	�Đ����������y�̊i�[����Ă���z��̗v�f�ԍ�
* @param	[in]	Volume	����
* @return	id ���z��̗v�f�ԍ��ł���� true
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