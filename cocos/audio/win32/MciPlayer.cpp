#include "MciPlayer.h"
#include "cocos2d.h"
USING_NS_CC;

#define WIN_CLASS_NAME        "CocosDenshionCallbackWnd"
#define BREAK_IF(cond)      if (cond) break;

static std::string _FullPath(const char * szPath)
{
	return FileUtils::getInstance()->fullPathForFilename(szPath);
}


namespace CocosDenshion {

static HINSTANCE s_hInstance;
static MCIERROR  s_mciError;

LRESULT WINAPI _SoundPlayProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

MciPlayer::MciPlayer()
: _wnd(NULL)
, _dev(0L)
, _soundID(0)
, _times(0)
, _playing(false)
, strExt("")
{
    if (! s_hInstance)
    {
        s_hInstance = GetModuleHandle( NULL );            // Grab An Instance For Our Window

        WNDCLASS  wc;        // Windows Class Structure

        // Redraw On Size, And Own DC For Window.
        wc.style          = 0;  
        wc.lpfnWndProc    = _SoundPlayProc;                    // WndProc Handles Messages
        wc.cbClsExtra     = 0;                              // No Extra Window Data
        wc.cbWndExtra     = 0;                                // No Extra Window Data
        wc.hInstance      = s_hInstance;                    // Set The Instance
        wc.hIcon          = 0;                                // Load The Default Icon
        wc.hCursor        = LoadCursor( NULL, IDC_ARROW );    // Load The Arrow Pointer
        wc.hbrBackground  = NULL;                           // No Background Required For GL
        wc.lpszMenuName   = NULL;                           // We Don't Want A Menu
        wc.lpszClassName  = WIN_CLASS_NAME;                 // Set The Class Name

        if (! RegisterClass(&wc)
            && 1410 != GetLastError())
        {
            return;
        }
    }

    _wnd = CreateWindowEx(
        WS_EX_APPWINDOW,                                    // Extended Style For The Window
        WIN_CLASS_NAME,                                        // Class Name
        NULL,                                        // Window Title
        WS_POPUPWINDOW,/*WS_OVERLAPPEDWINDOW*/               // Defined Window Style
        0, 0,                                                // Window Position
        0,                                                    // Window Width
        0,                                                    // Window Height
        NULL,                                                // No Parent Window
        NULL,                                                // No Menu
        s_hInstance,                                        // Instance
        NULL );
    if (_wnd)
    {
        SetWindowLongPtr(_wnd, GWLP_USERDATA, (LONG_PTR)this);
    }
}

MciPlayer::~MciPlayer()
{
    Close();
    DestroyWindow(_wnd);
}

void MciPlayer::Open(const char* pFileName, UINT uId)
{
//     WCHAR * pBuf = NULL;
    do 
    {
        BREAK_IF(! pFileName || ! _wnd);
        int nLen = (int)strlen(pFileName);
        BREAK_IF(! nLen);
//         pBuf = new WCHAR[nLen + 1];
//         BREAK_IF(! pBuf);
//         MultiByteToWideChar(CP_ACP, 0, pFileName, nLen + 1, pBuf, nLen + 1);
        
        std::string strFile(pFileName);
        int nPos = strFile.rfind(".") + 1;
        strExt = strFile.substr(nPos, strFile.length() - nPos);

        Close();

        MCI_OPEN_PARMS mciOpen = {0};
        MCIERROR mciError;
        mciOpen.lpstrDeviceType = (LPCTSTR)MCI_ALL_DEVICE_ID;
        mciOpen.lpstrElementName = pFileName;

        mciError = mciSendCommand(0,MCI_OPEN, MCI_OPEN_ELEMENT, reinterpret_cast<DWORD_PTR>(&mciOpen));
        BREAK_IF(mciError);

        _dev = mciOpen.wDeviceID;
        _soundID = uId;
        _playing = false;
    } while (0);
}

void MciPlayer::Play(UINT uTimes /* = 1 */)
{
    if (! _dev)
    {
        return;
    }
    MCI_PLAY_PARMS mciPlay = {0};
    mciPlay.dwCallback = reinterpret_cast<DWORD_PTR>(_wnd);
    s_mciError = mciSendCommand(_dev,MCI_PLAY, MCI_FROM|MCI_NOTIFY,reinterpret_cast<DWORD_PTR>(&mciPlay));
    if (! s_mciError)
    {
        _playing = true;
        _times = uTimes;
    }
}

void MciPlayer::Close()
{
    if (_playing)
    {
        Stop();
    }
    if (_dev)
    {
         _SendGenericCommand(MCI_CLOSE);
    }
    _dev = 0;
    _playing = false;
}

void MciPlayer::Pause()
{
    _SendGenericCommand(MCI_PAUSE);
}

void MciPlayer::Resume()
{
    if (strExt == "mid" || strExt == "MID")
    {
        // midi not supprt MCI_RESUME, should get the position and use MCI_FROM
        MCI_STATUS_PARMS mciStatusParms;
        MCI_PLAY_PARMS   mciPlayParms;  
        mciStatusParms.dwItem = MCI_STATUS_POSITION;   
        _SendGenericCommand(MCI_STATUS, MCI_STATUS_ITEM, reinterpret_cast<DWORD_PTR>(&mciStatusParms)); // MCI_STATUS   
        mciPlayParms.dwFrom = mciStatusParms.dwReturn;  // get position  
        _SendGenericCommand(MCI_PLAY, MCI_FROM, reinterpret_cast<DWORD_PTR>(&mciPlayParms)); // MCI_FROM
    } 
    else
    {
        _SendGenericCommand(MCI_RESUME);
    }   
}

void MciPlayer::Stop()
{
    _SendGenericCommand(MCI_STOP);
    _playing = false;
}

void MciPlayer::Rewind()
{
    if (! _dev)
    {
        return;
    }
    mciSendCommand(_dev, MCI_SEEK, MCI_SEEK_TO_START, 0);

    MCI_PLAY_PARMS mciPlay = {0};
    mciPlay.dwCallback = reinterpret_cast<DWORD_PTR>(_wnd);
    _playing = mciSendCommand(_dev, MCI_PLAY, MCI_NOTIFY,reinterpret_cast<DWORD_PTR>(&mciPlay)) ? false : true;
}

bool MciPlayer::IsPlaying()
{
    return _playing;
}

UINT MciPlayer::GetSoundID()
{
    return _soundID;
}

//////////////////////////////////////////////////////////////////////////
// private member
//////////////////////////////////////////////////////////////////////////
void MciPlayer::_SendGenericCommand( int nCommand, DWORD_PTR param1 /*= 0*/, DWORD_PTR parma2 /*= 0*/ )
{
    if (! _dev)
    {
        return;
    }
    mciSendCommand(_dev, nCommand, param1, parma2);
}


//////////////////////////////////////////////////////////////////////////
// static function
//////////////////////////////////////////////////////////////////////////

LRESULT WINAPI _SoundPlayProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    MciPlayer * pPlayer = NULL;
    if (MM_MCINOTIFY == Msg 
        && MCI_NOTIFY_SUCCESSFUL == wParam
        &&(pPlayer = (MciPlayer *)GetWindowLongPtr(hWnd, GWLP_USERDATA)))
    {
        if (pPlayer->_times)
        {
            --pPlayer->_times;
        }

        if (pPlayer->_times)
        {
            mciSendCommand(lParam, MCI_SEEK, MCI_SEEK_TO_START, 0);

            MCI_PLAY_PARMS mciPlay = {0};
            mciPlay.dwCallback = reinterpret_cast<DWORD_PTR>(hWnd);
            mciSendCommand(lParam, MCI_PLAY, MCI_NOTIFY,reinterpret_cast<DWORD_PTR>(&mciPlay));
        }
        else
        {
            pPlayer->_playing = false;
        }
        return 0;
    }
    return DefWindowProc(hWnd, Msg, wParam, lParam);
}

} // end of namespace CocosDenshion

MciAudioController::MciAudioController()
	: m_backgroundID(0)
{

}

void MciAudioController::Initialize()
{
	m_engineExperiencedCriticalError = false;
}

void MciAudioController::CreateResources()
{

}

void MciAudioController::ReleaseResources()
{
	EffectList::iterator EffectIter = m_soundEffects.begin();
	for (; EffectIter != m_soundEffects.end(); EffectIter++)
	{
		EffectIter->second->Close();
		delete EffectIter->second;
	}

	m_soundEffects.clear();
}

void MciAudioController::Start()
{
	if (m_engineExperiencedCriticalError)
	{
		return;
	}

	if (!m_backgroundFile.empty())
		PlayBackgroundMusic(m_backgroundFile.c_str(), m_backgroundLoop);
}

void MciAudioController::Render()
{
	if (m_engineExperiencedCriticalError)
	{
		ReleaseResources();
		Initialize();
		CreateResources();
		Start();
		if (m_engineExperiencedCriticalError)
		{
			return;
		}
	}
}

void MciAudioController::PlayBackgroundMusic(const char* pszFilePath, bool bLoop)
{
	m_backgroundFile = pszFilePath;
	m_backgroundLoop = bLoop;

	if (m_engineExperiencedCriticalError) {
		return;
	}

	StopBackgroundMusic(true);
	PlaySoundEffect(pszFilePath, bLoop, m_backgroundID, true);
}

void MciAudioController::StopBackgroundMusic(bool bReleaseData)
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	StopSoundEffect(m_backgroundID);

	if (bReleaseData)
		UnloadSoundEffect(m_backgroundID);
}

void MciAudioController::PauseBackgroundMusic()
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	PauseSoundEffect(m_backgroundID);
}

void MciAudioController::ResumeBackgroundMusic()
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	ResumeSoundEffect(m_backgroundID);
}

void MciAudioController::RewindBackgroundMusic()
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	RewindSoundEffect(m_backgroundID);
}

bool MciAudioController::IsBackgroundMusicPlaying()
{
	return IsSoundEffectStarted(m_backgroundID);
}

void MciAudioController::PlaySoundEffect(const char* pszFilePath, bool bLoop, unsigned int& sound, bool isMusic)
{
	sound = Hash(pszFilePath);

	if (m_soundEffects.end() == m_soundEffects.find(sound))
	{
		PreloadSoundEffect(pszFilePath, isMusic);
	}

	if (m_soundEffects.end() == m_soundEffects.find(sound))
		return;

	m_soundEffects[sound]->Play((bLoop) ? -1 : 1);
}

void MciAudioController::PlaySoundEffect(unsigned int sound)
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	if (m_soundEffects.end() == m_soundEffects.find(sound))
		return;

	StopSoundEffect(sound);

	m_soundEffects[sound]->Play(1);
}

bool MciAudioController::IsSoundEffectStarted(unsigned int sound)
{
	if (m_soundEffects.end() == m_soundEffects.find(sound))
		return false;

	return m_soundEffects[sound]->IsPlaying();
}

void MciAudioController::StopSoundEffect(unsigned int sound)
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	if (m_soundEffects.end() == m_soundEffects.find(sound))
		return;

	m_soundEffects[sound]->Stop();
}

void MciAudioController::PauseSoundEffect(unsigned int sound)
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	if (m_soundEffects.end() == m_soundEffects.find(sound))
		return;

	m_soundEffects[sound]->Pause();
}

void MciAudioController::ResumeSoundEffect(unsigned int sound)
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	if (m_soundEffects.end() == m_soundEffects.find(sound))
		return;

	m_soundEffects[sound]->Resume();
}

void MciAudioController::RewindSoundEffect(unsigned int sound)
{
	StopSoundEffect(sound);
	PlaySoundEffect(sound);
}

void MciAudioController::PauseAllSoundEffects()
{
	EffectList::iterator iter;
	for (iter = m_soundEffects.begin(); iter != m_soundEffects.end(); iter++)
	{
		PauseSoundEffect(iter->first);
	}
}

void MciAudioController::ResumeAllSoundEffects()
{
	EffectList::iterator iter;
	for (iter = m_soundEffects.begin(); iter != m_soundEffects.end(); iter++)
	{
		ResumeSoundEffect(iter->first);
	}
}

void MciAudioController::StopAllSoundEffects()
{
	EffectList::iterator iter;
	for (iter = m_soundEffects.begin(); iter != m_soundEffects.end(); iter++)
	{
		StopSoundEffect(iter->first);
	}
}

void MciAudioController::PreloadSoundEffect(const char* pszFilePath, bool isMusic)
{
	if (m_engineExperiencedCriticalError)
		return;

	int sound = Hash(pszFilePath);

	m_soundEffects.insert(Effect(sound, new CocosDenshion::MciPlayer()));
	CocosDenshion::MciPlayer* pPlayer = m_soundEffects[sound];
	pPlayer->Open(_FullPath(pszFilePath).c_str(), sound);
}

void MciAudioController::UnloadSoundEffect(const char* pszFilePath)
{
	int sound = Hash(pszFilePath);

	UnloadSoundEffect(sound);
}

void MciAudioController::UnloadSoundEffect(unsigned int sound)
{
	if (m_engineExperiencedCriticalError) {
		return;
	}

	if (m_soundEffects.end() == m_soundEffects.find(sound))
		return;

	RemoveFromList(sound);
}

void MciAudioController::RemoveFromList(unsigned int sound)
{
	if (m_soundEffects.end() == m_soundEffects.find(sound))
		return;

	m_soundEffects[sound]->Close();
	delete m_soundEffects[sound];
	m_soundEffects.erase(sound);
}
