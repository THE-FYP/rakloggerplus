#include "main.h"

void LogPacket(const string &logStr, const stPacketEntry &info, const stRakNetHookParams *params, bool incoming)
{
	// generic info
	string fulldata = logStr;
	if (cfg->_logInChat) Log(logStr, LOGOUTPUT_CHAT, incoming ? COLOR_RECV : COLOR_SEND);
	if (cfg->_logInConsole) Log(logStr, LOGOUTPUT_CONSOLE);
	if (cfg->_logInFile) Log(logStr, LOGOUTPUT_FILE);

	// params
	BSParamsContainer_t parlist = GetBitStreamParams((BitStreamEx *) params->bitStream, incoming ? info._incomingParams : info._outcomingParams);
	int readPtr = params->bitStream->GetReadOffset();
	string pardump = FormatBitStreamParams((BitStreamEx *) params->bitStream, parlist);
	string ch = incoming ? ">" : "<";
	if (!pardump.empty())
	{
		pardump.insert(0, ch + " params:\n");
		if (cfg->_drawParams) fulldata += "\n" + pardump;
		if (cfg->_logParamsInChat) Log(pardump, LOGOUTPUT_CHAT, incoming ? COLOR_PARAMS_RECV : COLOR_PARAMS_SEND);
		if (cfg->_logParamsInConsole) Log(pardump, LOGOUTPUT_CONSOLE);
		if (cfg->_logParamsInFile) Log(pardump, LOGOUTPUT_FILE, -1, false);
	}

	// dump
	string dump = DumpMem(params->bitStream->GetData(), params->bitStream->GetNumberOfBytesUsed());
	if (!dump.empty())
	{
		dump.insert(0, ch + " dump:\n");
		if (cfg->_drawDump) fulldata += "\n" + dump;
		if (cfg->_dumpInChat) Log(dump, LOGOUTPUT_CHAT, incoming ? COLOR_DUMP_RECV : COLOR_DUMP_SEND);
		if (cfg->_dumpInConsole) Log(dump, LOGOUTPUT_CONSOLE);
		if (cfg->_dumpInFile) Log(dump, LOGOUTPUT_FILE, -1, false);
	}

	// render, json
	render->SetString(info._id, info._type == stPacketEntry::RPC, incoming, fulldata);
	params->bitStream->SetReadOffset(readPtr);
	if (cfg->_JSONLogging) LogJSON(parlist, params, info, incoming);
}

bool CALLBACK RakHookIncomingRPC(stRakNetHookParams *params)
{
	try
	{
		int id = params->packetId;
		stPacketEntry &info = cfg->GetRPCInfo(id);
		if (!cfg->_logging || !info)
			return true;
		if (info._ignoreIncoming == cfg->_invertedIgnore && cfg->_logIncomingRPC)
		{
			string str = formatstr("> [RPC Recv] %s(length: %d bytes, %d bits)")
				% GetFormattedPacketName(info)
				% params->bitStream->GetNumberOfBytesUsed()
				% params->bitStream->GetNumberOfBitsUsed();
			LogPacket(str, info, params, true);
		}
	}
	catch (exception &e)
	{
		Log(string("EXCEPTION(RakHookIncomingRPC): ") + e.what(), LOGOUTPUT_FILE);
	}
	return true;
}

bool CALLBACK RakHookOutcomingRPC(stRakNetHookParams *params)
{
	try
	{
		int id = params->packetId;
		stPacketEntry &info = cfg->GetRPCInfo(id);
		if (!cfg->_logging || !info)
			return true;
		if (info._ignoreOutcoming == cfg->_invertedIgnore && cfg->_logOutcomingRPC)
		{
			string str = formatstr("< [RPC Send] %s(length: %d bytes, %d bits, priority: %d, reliability: %d, orderingChannel: %d)")
				% GetFormattedPacketName(info)
				% params->bitStream->GetNumberOfBytesUsed()
				% params->bitStream->GetNumberOfBitsUsed()
				% params->priority
				% params->reliability
				% int(params->orderingChannel);
			LogPacket(str, info, params, false);
		}
	}
	catch (exception &e)
	{
		Log(string("EXCEPTION(RakHookOutcomingRPC): ") + e.what(), LOGOUTPUT_FILE);
	}
	return true;
}

bool CALLBACK RakHookIncomingPacket(stRakNetHookParams *params)
{
	try
	{
		int id = params->packetId;
		stPacketEntry &info = cfg->GetPacketInfo(id);
		if (!cfg->_logging || !info)
			return true;
		if (info._ignoreIncoming == cfg->_invertedIgnore && cfg->_logIncomingPacket)
		{
			string str = formatstr("> [Packet Recv] %s(length: %d bytes, %d bits)")
				% GetFormattedPacketName(info)
				% params->bitStream->GetNumberOfBytesUsed()
				% params->bitStream->GetNumberOfBitsUsed();
			params->bitStream->IgnoreBits(8); // skip packet id
			LogPacket(str, info, params, true);
		}
	}
	catch (exception &e)
	{
		Log(string("EXCEPTION(RakHookIncomingPacket): ") + e.what(), LOGOUTPUT_FILE);
	}
	return true;
}

bool CALLBACK RakHookOutcomingPacket(stRakNetHookParams *params)
{
	try
	{
		int id = params->packetId;
		stPacketEntry &info = cfg->GetPacketInfo(id);
		if (!cfg->_logging || !info)
			return true;
		if (info._ignoreOutcoming == cfg->_invertedIgnore && cfg->_logOutcomingPacket)
		{
			string str = formatstr("< [Packet Send] %s(length: %d bytes, %d bits, priority: %d, reliability: %d, orderingChannel: %d)")
				% GetFormattedPacketName(info)
				% params->bitStream->GetNumberOfBytesUsed()
				% params->bitStream->GetNumberOfBitsUsed()
				% params->priority
				% params->reliability
				% int(params->orderingChannel);
			params->bitStream->IgnoreBits(8); // skip packet id
			LogPacket(str, info, params, false);
		}
	}
	catch (exception &e)
	{
		Log(string("EXCEPTION(RakHookOutcomingPacket): ") + e.what(), LOGOUTPUT_FILE);
	}
	return true;
}

bool CALLBACK D3DHookPresent(const RECT*, const RECT*, HWND, const RGNDATA*)
{
	if (!SF->getRender()->CanDraw())
		return true;
	if (SUCCEEDED(SF->getRender()->BeginRender()))
	{
		render->Draw();
		SF->getRender()->EndRender();
	}
	return true;
}

void CALLBACK GameDestructor(void)
{
	if (cfg->_JSONLogging)
		SaveJSON("");
}

////
//  COMMANDS
///

void CALLBACK cmdRPCList(std::string params)
{
	for (size_t i = 0; i < 256; i++)
	{
		RPCNode *node = SF->getSAMP()->getInfo()->GetRPCNodeByIndex(i);
		if (node != nullptr)
		{
			stPacketEntry &rpc = cfg->GetRPCInfo(i);
			Log(formatstr("RPC: %s [%d] offset = 0x%X") % (rpc._name.empty() ? "UNKNOWN" : rpc._name) % i % ((uint32_t) node->staticFunctionPointer - SAMP->getSAMPAddr()), LOGOUTPUT_FILE);
		}	
	}
}

void CALLBACK cmdLog(std::string params)
{
	Log(params, LOGOUTPUT_FILE);
}

void CALLBACK cmdReload(std::string params)
{
	if (!params.empty() && _access(params.c_str(), 0) != -1)
		cfg->_configFilename = params;
	cfg->Load();
}

void CALLBACK cmdDrawRecv(std::string params)
{
	stringstream p(params);
	string name;
	int x = 0, y = 0;
	p >> name >> x >> y;
	stPacketEntry &pkt = cfg->FindPacket(name);
	if (!pkt)
		return Log(name + " not found.");
	render->ToggleRender(pkt._id, x, y, pkt._type == stPacketEntry::RPC, true);
	Log(GetFormattedPacketName(pkt));
}

void CALLBACK cmdDrawSend(std::string params)
{
	stringstream p(params);
	string name;
	int x = 0, y = 0;
	p >> name >> x >> y;
	stPacketEntry &pkt = cfg->FindPacket(name);
	if (!pkt)
		return Log(name + " not found.");
	render->ToggleRender(pkt._id, x, y, pkt._type == stPacketEntry::RPC, false);
	Log(GetFormattedPacketName(pkt));
}

void CALLBACK cmdSaveJSON(std::string params)
{
	SaveJSON(params);
}

void CALLBACK cmdCmdList(std::string params)
{
	Log(" --- RakLogger+ Command List ---");
	Log(" rpclist - saves a list of all registered RPC in the log-file");
	Log(" raklog [text] - prints text in the log-file");
	Log(" rakreload [filename(optional)] - reloads settings");
	Log(" drawsend [part_of_name] [x] [y] - toggles drawing of outcoming packet/rpc");
	Log(" drawrecv [part_of_name] [x] [y] - toggles drawing of incoming packet/rpc");
	Log(" savejson [filename(optional)] - saves packets dump in JSON format to file");
	Log(" --------------------------------");
}