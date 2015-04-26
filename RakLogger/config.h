#include <string>
using namespace std;

struct stPacketEntry
{
	operator bool() const
	{
		return _id != -1 && _type != UNK;
	}
	string						_name;
	string						_incomingParamsStr;
	string						_outcomingParamsStr;
	list<stPacketParam>			_incomingParams;
	list<stPacketParam>			_outcomingParams;
	int							_id = -1;
	bool						_ignoreIncoming = false;
	bool						_ignoreOutcoming = false;
	enum PacketType 
	{ UNK, PACKET, RPC }		_type = UNK;
};

class CConfigBase
{
protected:
	boost::property_tree::iptree _ptree;
	string						_path;

public:
	/* ctor */					CConfigBase(void);
	bool						Load(string file);
};

class CConfig : public CConfigBase
{
public:
	string						_sessionTime;
	string						_configFilename = "SAMPFUNCS\\RakLogger.ini";
	static const size_t			_packetsSize = 256;

	/* ctor */					CConfig(void) : CConfigBase() {}

	// params
	bool						_logging;
	bool						_invertedIgnore;
	bool						_timestamp;
	bool						_JSONLogging;
	bool						_logEachSessionSingly;
	bool						_dumpInFile, _dumpInChat, _dumpInConsole;
	bool						_logInFile, _logInChat, _logInConsole;
	bool						_logParamsInFile, _logParamsInChat, _logParamsInConsole;
	bool						_logIncomingRPC, _logOutcomingRPC, _logIncomingPacket, _logOutcomingPacket;
	bool						_drawParams, _drawDump;

	bool						ReadSettings(void);
	stPacketEntry				&GetPacketEntry(stPacketEntry *parr, size_t id);
	stPacketEntry				&GetPacketEntry(stPacketEntry *parr, std::string name, bool part = false);
	stPacketEntry				&GetPacketInfo(size_t id);
	stPacketEntry				&GetRPCInfo(size_t id);
	stPacketEntry				&GetPacketByName(std::string name);
	stPacketEntry				&FindPacket(std::string partOfName);
	bool						Load(void);
	
	static bool					ParseParams(string params, list<stPacketParam> &out);

private:
	stPacketEntry				_rpc[_packetsSize], _packet[_packetsSize];
	static stPacketEntry		_badPacketInfo;
};

extern CConfig *cfg;