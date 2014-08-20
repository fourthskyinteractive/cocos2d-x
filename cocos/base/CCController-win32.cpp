/****************************************************************************
 Copyright (c) 2014 cocos2d-x.org
 Copyright (c) 2014 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "CCController.h"
#include "base/CCPlatformConfig.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)

#include "ccMacros.h"
#include "CCEventDispatcher.h"
#include "CCEventController.h"
#include "CCEventListenerController.h"
#include "CCDirector.h"

#include <windows.h>
#include <dinput.h>
#include <XInput.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

// XInput
static HMODULE xinputDLL = 0;
typedef DWORD (*XInputGetStateFunc)(DWORD, XINPUT_STATE*);
XInputGetStateFunc _XInputGetState;
#define GetState _XInputGetState

bool InitializeXInput();

// DirectInput8
LPDIRECTINPUT8          g_pDI = nullptr; 


#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
#define GetState XInputGetState

#endif

NS_CC_BEGIN

class ControllerImpl
{
public:
    ControllerImpl(Controller* controller)
    : _controller(controller)
    , _diController(nullptr)
	, _eventHandle(0)
    {
        
    }
	
	static BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext ); 
	static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext ); 
    
    Controller* _controller;
	HANDLE _eventHandle;
    LPDIRECTINPUTDEVICE8 _diController;
};

void Controller::startDiscoveryController()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	{
		// Initializing DirectInput
		HRESULT hr;

		// Register with the DirectInput subsystem and get a pointer 
		// to a IDirectInput interface we can use. 
		// Create a DInput object 
		hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&g_pDI, nullptr);
		if (SUCCEEDED(hr))
		{
			// Look for a simple joystick we can use for this sample program. 
			if (SUCCEEDED(hr = g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL,
				ControllerImpl::EnumJoysticksCallback,
				nullptr,
				DIEDFL_ATTACHEDONLY)))
			{


				// TODO Create a thread to listen for devices


			}
		}
	}
		
#endif


	// onConnected block
	/*
		auto controller = new Controller();
        controller->_impl->_gcController = gcController;
        controller->_deviceName = [gcController.vendorName UTF8String];
        
        s_allController.push_back(controller);
        
        controller->registerListeners();
        controller->getDeviceName();
        
        controller->onConnected();
	
	*/
	
	//onDisconnected block
	/*
		auto iter = std::find_if(s_allController.begin(), 
								 s_allController.end(), 
								 [gcController](Controller* c){ return c->_impl->_gcController == gcController; });
        
        if(iter == s_allController.end())
        {
            log("disconnect:Could not find the controller");
            return;
        }
        
        (*iter)->onDisconnected();
        s_allController.erase(iter);
	*/
	
}

void Controller::stopDiscoveryController()
{
	
}

Controller::Controller()
: _controllerTag(TAG_UNSET)
, _impl(new ControllerImpl(this))
, _connectEvent(nullptr)
, _keyEvent(nullptr)
, _axisEvent(nullptr)
, _deviceId(0)
{
    init();
}

Controller::~Controller()
{
    delete _impl;
    
    delete _connectEvent;
    delete _keyEvent;
    delete _axisEvent;
}

void Controller::registerListeners()
{
	
}

bool Controller::isConnected() const
{
	HRESULT hr = _impl->_diController->Acquire();
	
	return hr == S_OK;
}

void Controller::receiveExternalKeyEvent(int externalKeyCode,bool receive)
{
}



#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

bool InitializeXInput()
{
	if (xinputDLL == 0)
	{
		xinputDLL = LoadLibraryA("xinput1_4.dll");
		if (xinputDLL == 0) 
		{
			xinputDLL = LoadLibraryA("xinput1_3.dll");
			if (xinputDLL == 0) 
			{
				xinputDLL = LoadLibraryA("xinput9_1_0.dll");
				if (xinputDLL == 0)
				{
					return false;
				}
			}
		}
	}
	
	_XInputGetState = (XInputGetStateFunc) GetProcAddress(xinputDLL, "XInputGetState");
	
	return true;
}

BOOL CALLBACK ControllerImpl::EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
	//auto pEnumContext = reinterpret_cast<DI_ENUM_CONTEXT*>( pContext ); 
    HRESULT hr = S_OK; 
	LPDIRECTINPUTDEVICE8 _joystick = nullptr;
	hr = g_pDI->CreateDevice( pdidInstance->guidInstance, &_joystick, nullptr );
	if ( SUCCEEDED( hr ) ) 
	{
		// Set the data format to "simple joystick" - a predefined data format  
		// 
		// A data format specifies which controls on a device we are interested in, 
		// and how they should be reported. This tells DInput that we will be 
		// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState(). 
		hr = _joystick->SetDataFormat( &c_dfDIJoystick2 );
		
		// Set the cooperative level to let DInput know how this device should 
		// interact with the system and with other DInput applications. 
		hr = _joystick->SetCooperativeLevel( nullptr, DISCL_EXCLUSIVE | DISCL_FOREGROUND );
		
		// Enumerate the joystick objects. The callback function enabled user 
		// interface elements for objects that are found, and sets the min/max 
		// values property for discovered axes. 
		hr = _joystick->EnumObjects( EnumObjectsCallback, (VOID*)_joystick, DIDFT_ALL );
	
		DIDEVICEINSTANCE di;
		_joystick->GetDeviceInfo( &di );
	
		auto controller = new Controller();
        controller->_impl->_diController = _joystick;
		controller->_impl->_eventHandle = CreateEventW( nullptr, 0, 0, di.tszInstanceName );
		
		// Register DirectInput to receive notification
		controller->_impl->_diController->SetEventNotification( controller->_impl->_eventHandle );
		
		Controller::s_allController.push_back(controller);
        
        controller->registerListeners();
        controller->getDeviceName();
        controller->onConnected();
	}	
	
	return DIENUM_CONTINUE; 
}

BOOL CALLBACK ControllerImpl::EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
	LPDIRECTINPUTDEVICE _joystick = (LPDIRECTINPUTDEVICE) pContext;
	
	// For axes that are returned, set the DIPROP_RANGE property for the 
    // enumerated axis in order to scale min/max values. 
    if( pdidoi->dwType & DIDFT_AXIS ) 
    { 
        DIPROPRANGE diprg; 
        diprg.diph.dwSize = sizeof( DIPROPRANGE ); 
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER ); 
        diprg.diph.dwHow = DIPH_BYID; 
        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis 
        diprg.lMin = -1000; 
        diprg.lMax = +1000; 
 
        // Set the range for the axis 
        if( FAILED( _joystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
            return DIENUM_STOP; 
    } 
	
	return DIENUM_CONTINUE; 
}

#endif

NS_CC_END

#endif // #if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
