#include "main.h"

CSAMPFUNCS *SF = new CSAMPFUNCS();
CSAMP *SAMP = nullptr;
CGame *Game = nullptr;

void CALLBACK loop(void)
{
	static bool _inited = false, _dontLoad = false;
	if (_dontLoad || !SF->getSAMP()->IsInitialized() )
		return;
	if (!_inited)
	{
		if (!cfg->Load())
		{
			Log("Can't load settings. Aborting...", LOGOUTPUT_FILE);
			_dontLoad = true;
			return;
		}

		SAMP = SF->getSAMP();
		Game = SF->getGame();

		// setup raknet hooks
		SF->getRakNet()->registerRakNetCallback(RAKHOOK_TYPE_INCOMING_RPC, RakHookIncomingRPC);
		SF->getRakNet()->registerRakNetCallback(RAKHOOK_TYPE_OUTCOMING_RPC, RakHookOutcomingRPC);
		SF->getRakNet()->registerRakNetCallback(RAKHOOK_TYPE_INCOMING_PACKET, RakHookIncomingPacket);
		SF->getRakNet()->registerRakNetCallback(RAKHOOK_TYPE_OUTCOMING_PACKET, RakHookOutcomingPacket);
		
		// init commands
		SAMP->registerChatCommand("rakcmds", cmdCmdList);
		SAMP->registerChatCommand("rpclist", cmdRPCList);
		SAMP->registerChatCommand("raklog", cmdLog);
		SAMP->registerChatCommand("rakreload", cmdReload);
		SAMP->registerChatCommand("drawrecv", cmdDrawRecv);
		SAMP->registerChatCommand("drawsend", cmdDrawSend);
		SAMP->registerChatCommand("savejson", cmdSaveJSON);

		// install misc hooks
		SF->getRender()->registerD3DCallback(D3DMETHOD_PRESENT, D3DHookPresent);
		SF->getGame()->registerGameDestructorCallback(GameDestructor);

		// hello message
		SAMP->getChat()->AddChatMessage(0x99FFC0, "RakLogger+ by FYP loaded. // BLASTHACK.NET");
		Log(formatstr("Session started on %1%.") % cfg->_sessionTime, LOGOUTPUT_FILE);

		_inited = true;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
	if (dwReasonForCall == DLL_PROCESS_ATTACH)
		return SF->initPlugin(loop, hModule);
	return TRUE;
}

// Made by FYP
// BLASTHACK.NET