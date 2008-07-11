/******************************************************************************
          .d8888b.   .d8888b.  
         d88P  Y88b d88P  Y88b 
         888    888        888 
88888b.  888    888      .d88P 
888 "88b 888    888  .od888P"  
888  888 888    888 d88P"      
888  888 Y88b  d88P 888"       
888  888  "Y8888P"  888888888              Open Kaillera Arcade Netplay Library
*******************************************************************************
Copyright (c) Open Kaillera Team 2003-2008

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice must be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#include "transport.h"
#include "common.h"

namespace n02 {
	static void N02CCNV defaultTransportInitialize()
	{
		LOG(0);
	}
	static void N02CCNV defaultTransportTerminate()
	{
		LOG(0);
	}
	static void N02CCNV defaultTransportActivete()
	{
		LOG(0);
	}
	static void N02CCNV defaultTransportActiveteGUI()
	{
		LOG(0);

	}
	static int N02CCNV defaultTransportIdleStep()
	{
		LOG(0);
		return false;
	}
	static int  N02CCNV defaultTransportSynchronizeGame(void * syncData, int len)
	{
		char strBuffer[256];
		StringUtils::bytesToStr(strBuffer, (unsigned char*)syncData, len);
		LOG(%s; %i, strBuffer, len);
		return 0;
	}
	static void N02CCNV defaultTransportEndGame()
	{
		LOG(0);
	}
	static void N02CCNV defaultTransportSendAsyncData(const void * value, const int len, const int mode)
	{
		char strBuffer[256];
		StringUtils::bytesToStr(strBuffer, (unsigned char*)value, len);
		LOG(%s; %i; %i, strBuffer, len, mode);
	}
	static void N02CCNV defaultTransportSendSyncData(const void * value, const int len)
	{
		char strBuffer[256];
		StringUtils::bytesToStr(strBuffer, (unsigned char*)value, len);
		LOG(%s; %i, strBuffer, len);
	}
	static int  N02CCNV defaultTransportRecvSyncData(void * value, const int len)
	{
		LOG(%x; %i, value, len);
		return 0;
	}
	static int  N02CCNV defaultTransportSyncData(void * value, const int len)
	{
		char strBuffer[256];
		StringUtils::bytesToStr(strBuffer, (unsigned char*)value, len);
		LOG(%s; %i, strBuffer, len);
		return 0;
	}
	static int  N02CCNV defaultTransportSyncDataCheck(int)
	{
		LOG(0);
		return 0;
	}
	static void N02CCNV defaultTransportSendChat(const char * message)
	{
		LOG(%s, message);
	}
	static int  N02CCNV defaultTransportGetSelectedAutorunAlgorithm()
	{
		LOG(0);
		return 0;
	}
	static int  N02CCNV defaultTransportGetSelectedAutorunSmoothing()
	{
		LOG(0);
		return 0;
	}
	static int N02CCNV defaultTransportIsRecordingEnabled()
	{
		LOG(0);
		return true;
	}

	static int N02CCNV defaultTransportGetGameplayType() {
		return GAME_TYPE_GAMEPLAY;
	}

	static void N02CCNV  defaultTransportReserved(int, int)
	{
		LOG(0);
	}

	static TransportModule02 defaultTransport = {
		defaultTransportInitialize,
		defaultTransportTerminate,
		defaultTransportActivete,
		defaultTransportIdleStep,
		defaultTransportSynchronizeGame,
		defaultTransportEndGame,
		defaultTransportSendAsyncData,
		defaultTransportSendSyncData,
		defaultTransportRecvSyncData,
		defaultTransportSyncData,
		defaultTransportSyncDataCheck,
		defaultTransportSendChat,
		defaultTransportActiveteGUI,
		defaultTransportGetSelectedAutorunAlgorithm,
		defaultTransportGetSelectedAutorunSmoothing,
		defaultTransportIsRecordingEnabled,
		defaultTransportGetGameplayType,
		defaultTransportReserved,
		defaultTransportReserved,
		defaultTransportReserved
	};

	TransportModule02 transport;

	static char activeModName[32];
	static TCHAR activeModNameT[32];

	CONFIG_START(transportConfig)
		CONFIG_STRVAR(_T("active"), activeModNameT, 32, _T(""))
		CONFIG_END

	void transportReset() {
		transport = defaultTransport;
	}

	static void transportActivate(ModuleAbstraction02 * mod);

	void transportInitialize()
	{
		transportReset();

		char active[128];
		ConfigurationManager config(transportConfig);
		config.load(_T("transport"));
		StringUtils::TCHARToUTF8(reinterpret_cast<unsigned char*>(active), activeModNameT);

		if (strlen(active) > 0
			&& strlen(active) < 16
			&& modHelper.modHandler->getByName(active) != 0
			&& strcmp(modHelper.modHandler->getByName(active)->type, MTYPE02_TRANSPORT)==0) {
				transportSetActiveByName(active);
		} else {
			transportActivate(modHelper.modHandler->find(MTYPE02_TRANSPORT));
		}	
	}

	void transportTerminate()
	{
		StringUtils::UTF8ToTCHAR(activeModNameT, reinterpret_cast<unsigned char*>(activeModName), 32);
		ConfigurationManager config(transportConfig);
		config.save(_T("transport"));
	}

	static bool recentlyActivated;

	static void transportActivate(ModuleAbstraction02 * mod)
	{
		if (mod != 0) {
			if (strcmp(mod->type, MTYPE02_TRANSPORT)==0) {
				if (mod->getInterface(&transport)==0) {
					recentlyActivated = true;
					strcpy(activeModName, mod->name);
				} else {
					LOG(Failed to retrive the module interface structure);
				}
			} else {
				LOG(Invalid type of module specified %s, mod->type);
			}
		} else {
			LOG(Could not find specified transport module index);
		}
	}

	void N02CCNV transportSetActiveByName(char * name) {
		LOG(%s, name);
		if (name != 0 && strlen(name) > 0) {
			if (strcmp(name, "dummy"))
				transportActivate(modHelper.modHandler->getByName(name));
			else
				transportReset();
		} else {
			LOG(Invalid module name specified);
		}
	}

	void N02CCNV transportSetActiveByIndex(int index) {
		LOG(%i, index);
		if (index >= 0) {
			transportActivate(modHelper.modHandler->getByIndex(index));
		} else {
			LOG(Invalid module index specified);
		}
	}

	void transportResetActivation() {
		recentlyActivated = false;
	}
	bool transportWasReActivated() {
		return recentlyActivated;
	}

	//void N02CCNV callbackActiveModuleChange() {
	//		// Make things faster
	//		//n02->gameplay.synchronizeGame = activeMod.synchronizeGame;
	//		n02->gameplay.endGame = activeMod.endGame;
	//		n02->gameplay.sendAsyncData = activeMod.sendAsyncData;
	//		n02->gameplay.sendSyncData = activeMod.sendSyncData;	
	//		n02->gameplay.recvSyncData = activeMod.recvSyncData;
	//		//n02->gameplay.syncData = activeMod.syncData;
	//		n02->gameplay.syncDataCheck = activeMod.syncDataCheck;
	//		n02->gameplay.sendChat = activeMod.sendChat;
	//	}

};

//	int  N02CCNV activateTransportModule (char * name, TransportModule02 * mod){
//		LOG(%i %i, name, mod);
//		TransportModule02 * modToActivate = 0;
//		int modToActivateIndex = 0;
//		if (!mod && name) {
//			LOG(Search my name);
//			if (transportModules.size()) {
//				for (int x = 0; x < transportModules.size();  x++) {
//					if (strcmp(transportModules[x]->name, name)==0) {
//						LOG(Search my name Item found);
//						modToActivate = transportModules[modToActivateIndex = x];
//						break;
//					}
//				}
//			}
//		}
//
//		if (!modToActivate && mod) {
//			// make sure this module is in our list
//			LOG(MOD Pointer);
//			if (transportModules.size()) {
//				for (int x = 0; x < transportModules.size();  x++) {
//					if (strcmp(transportModules[x]->name, mod->name)==0) {
//						modToActivate = transportModules[modToActivateIndex = x];
//						break;
//					}
//				}
//			}
//		}
//
//		modeActivatedRecently = 1;
//
//		if (modToActivate && strcmp(modToActivate->name, activeMod.name)) {
//			LOG(Activating MOD %i, activeModIndex);
//			activeMod = *modToActivate;
//			activeModIndex = modToActivateIndex;
//
//			callbackActiveModuleChange();
//			return 0;
//		} else {
//			LOG(No Mod Activate);
//		}
//		return 1;
//	}
//

