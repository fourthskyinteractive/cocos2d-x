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

#include <XInput.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
static HANDLE xinputDLL = 0;
typedef DWORD (*XInputGetStateFunc)(DWORD, XINPUT_STATE*);
XInputGetStateFunc _XInputGetState;

#define GetState _XInputGetState

static bool InitializeXInput()
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
			}
		}
	}
	
	if (xinputDLL != 0)
	{
		_XInputGetState = GetProcAddress(xinputDLL, "XInputGetState");
	}
}

static bool InitializeDirectInput()
{
	
}

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
#define GetState XInputGetState

#endif

NS_CC_BEGIN

class ControllerImpl
{
public:
    ControllerImpl(Controller* controller)
    : _controller(controller)
    , _controllerIndex(-1)
    {
        
    }
    
    Controller* _controller;
    int _controllerIndex;
};

void Controller::startDiscoveryController()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	InitializeXInput();
		
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
	
}

void Controller::receiveExternalKeyEvent(int externalKeyCode,bool receive)
{
}

NS_CC_END

#endif // #if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
