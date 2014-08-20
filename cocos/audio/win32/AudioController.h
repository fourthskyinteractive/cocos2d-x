
#ifndef _AUDIO_CONTROLLER_H_
#define _AUDIO_CONTROLLER_H_

class AudioController
{
public:
	unsigned int Hash(const char* key) {
		unsigned int len = strlen(key);
		const char *end=key+len;
		unsigned int hash;

		for (hash = 0; key < end; key++)
		{
			hash *= 16777619;
			hash ^= (unsigned int) (unsigned char) toupper(*key);
		}
		return (hash);
	}

	virtual void Initialize() = 0;
	virtual void CreateResources() = 0;
	virtual void ReleaseResources() = 0;
	virtual void Start() = 0;
	virtual void Render() = 0;

	// This flag can be used to tell when the audio system is experiencing critial errors.
	// XAudio2 gives a critical error when the user unplugs their headphones, and a new
	// speaker configuration is generated.
	virtual void SetEngineExperiencedCriticalError() = 0;
	virtual bool HasEngineExperiencedCriticalError() = 0;

	virtual void PlayBackgroundMusic(const char* pszFilePath, bool bLoop) = 0;
	virtual void StopBackgroundMusic(bool bReleaseData) = 0;
	virtual void PauseBackgroundMusic() = 0;
	virtual void ResumeBackgroundMusic() = 0;
	virtual void RewindBackgroundMusic() = 0;
	virtual bool IsBackgroundMusicPlaying() = 0;

	virtual void SetBackgroundVolume(float volume) = 0;
	virtual float GetBackgroundVolume() = 0;
	virtual void SetSoundEffectVolume(float volume) = 0;
	virtual float GetSoundEffectVolume() = 0;

	virtual void PlaySoundEffect(const char* pszFilePath, bool bLoop, unsigned int& sound, bool isMusic = false) = 0;
	virtual void PlaySoundEffect(unsigned int sound) = 0;
	virtual bool IsSoundEffectStarted(unsigned int sound) = 0;
	virtual void StopSoundEffect(unsigned int sound) = 0;
	virtual void PauseSoundEffect(unsigned int sound) = 0;
	virtual void ResumeSoundEffect(unsigned int sound) = 0;
	virtual void RewindSoundEffect(unsigned int sound) = 0;

	virtual void PauseAllSoundEffects() = 0;
	virtual void ResumeAllSoundEffects() = 0;
	virtual void StopAllSoundEffects() = 0;

	virtual void PreloadSoundEffect(const char* pszFilePath, bool isMusic = false) = 0;
	virtual void UnloadSoundEffect(const char* pszFilePath) = 0;
	virtual void UnloadSoundEffect(unsigned int sound) = 0;

protected:
	virtual void RemoveFromList(unsigned int sound) = 0;
};

#endif

