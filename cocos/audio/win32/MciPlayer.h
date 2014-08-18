#ifndef _MCI_PLAYER_WIN32_H_
#define _MCI_PLAYER_WIN32_H_

#include "CCStdC.h"
#include "AudioController.h"

#include <mmsystem.h>
#include <string>
#include <map>
using namespace std;

namespace CocosDenshion {

class MciPlayer
{
public:
    MciPlayer();
    ~MciPlayer();

    void Close();

    /**
    @brief  Play sound file
    @param pFileName    Sound's file name,include the file path.
    @param nTimes    Play mode£¬default value is 1,paly once
    */
    void Open(const char* pFileName, UINT uId);

    void Play(UINT uTimes = 1);

    /**
    @brief Pause play
    */
    void Pause();

    /**
    @brief Resume play
    */
    void Resume();

    /**
    @brief Stop play
    */
    void Stop();

    /**
    @brief Replay
    */
    void Rewind();

    /**
    @brief Is Playing
    */
    bool IsPlaying();

    /**
    @brief  Get playing sound's ID
    @return Sound's ID
    */
    UINT GetSoundID();

private:
    friend LRESULT WINAPI _SoundPlayProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    void _SendGenericCommand(int nCommand, DWORD_PTR param1 = 0, DWORD_PTR parma2 = 0);

    HWND        _wnd;
    MCIDEVICEID _dev;
    UINT        _soundID;
    UINT        _times;
    bool        _playing;
    std::string strExt;
};

} // end of namespace CocosDenshion

class MciAudioController : public AudioController
{
private:
	typedef std::map<unsigned int, CocosDenshion::MciPlayer*> EffectList;
	typedef std::pair<unsigned int, CocosDenshion::MciPlayer*> Effect;
	EffectList				    m_soundEffects;

	bool                        m_engineExperiencedCriticalError;

	unsigned int                m_backgroundID;
	std::string                 m_backgroundFile;
	bool                        m_backgroundLoop;

public:
	MciAudioController();

	void Initialize();
	void CreateResources();
	void ReleaseResources();
	void Start();
	void Render();

	// This flag can be used to tell when the audio system is experiencing critial errors.
	// XAudio2 gives a critical error when the user unplugs their headphones, and a new
	// speaker configuration is generated.
	void SetEngineExperiencedCriticalError()
	{
		m_engineExperiencedCriticalError = true;
	}

	bool HasEngineExperiencedCriticalError()
	{
		return m_engineExperiencedCriticalError;
	}

	void PlayBackgroundMusic(const char* pszFilePath, bool bLoop);
	void StopBackgroundMusic(bool bReleaseData);
	void PauseBackgroundMusic();
	void ResumeBackgroundMusic();
	void RewindBackgroundMusic();
	bool IsBackgroundMusicPlaying();

	void SetBackgroundVolume(float volume) { }
	float GetBackgroundVolume() { return 1.0f; }
	void SetSoundEffectVolume(float volume) { }
	float GetSoundEffectVolume() { return 1.0f; }

	void PlaySoundEffect(const char* pszFilePath, bool bLoop, unsigned int& sound, bool isMusic = false);
	void PlaySoundEffect(unsigned int sound);
	bool IsSoundEffectStarted(unsigned int sound);
	void StopSoundEffect(unsigned int sound);
	void PauseSoundEffect(unsigned int sound);
	void ResumeSoundEffect(unsigned int sound);
	void RewindSoundEffect(unsigned int sound);

	void PauseAllSoundEffects();
	void ResumeAllSoundEffects();
	void StopAllSoundEffects();

	void PreloadSoundEffect(const char* pszFilePath, bool isMusic = false);
	void UnloadSoundEffect(const char* pszFilePath);
	void UnloadSoundEffect(unsigned int sound);

protected:
	void RemoveFromList(unsigned int sound);
};

#endif
