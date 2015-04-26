extern std::ofstream g_Log;
extern boost::property_tree::ptree g_PTreeJSONLog;

enum LogMsgType
{
	LOGTYPE_PACKET,
	LOGTYPE_DUMP,
	LOGTYPE_PARAMS
};

enum LogOutput
{
	LOGOUTPUT_FILE = 1,
	LOGOUTPUT_CHAT = 2,
	LOGOUTPUT_CONSOLE = 3,
};

enum LogColor : D3DCOLOR
{
	COLOR_INFO = 0xFFDDDDDD,
	COLOR_SEND = 0xFFFFFFBB,
	COLOR_RECV = 0xFF88FFBB,
	COLOR_DUMP_SEND = 0xFFFFFFD9,
	COLOR_DUMP_RECV = 0xFFBFFFDF,
	COLOR_PARAMS_SEND = 0xFFFFFFCC,
	COLOR_PARAMS_RECV = 0xFF99FFCC,
};

class formatstr
{
public:
	formatstr(const char* fmt) : _fmt(fmt) {}
	formatstr(std::string fmt) : _fmt(fmt) {}
	formatstr(const std::string &fmt) : _fmt(fmt) {}

	template<class T>
	formatstr& operator%(const T& arg)
	{
		_fmt % arg;
		return *this;
	}
	operator std::string() const
	{
		return _fmt.str();
	}
private:
	boost::format _fmt;
};

typedef std::list< std::pair<stPacketParam, std::string>> BSParamsContainer_t;

void Log(std::string msg, LogOutput output, D3DCOLOR color = -1, bool timestamp = true);
void Log(std::string msg);
void LogJSON(const BSParamsContainer_t &paramlist, const stRakNetHookParams *params, const stPacketEntry &pktinfo, bool incoming);
void SaveJSON(std::string filename);
std::string GetDateTime(void);
std::string GetTime(void);
std::string GetFormattedPacketName(stPacketEntry &pe);
string FormatBitStreamParams(BitStreamEx *bs, BSParamsContainer_t &params);
BSParamsContainer_t GetBitStreamParams(BitStreamEx *bs, const std::list<stPacketParam> &params);
std::string DumpMem(byte *address, size_t len);
std::string BinToHex(byte *bin, size_t size);
std::string StringCutTo(std::string str, std::string delim, bool strict);
std::string StringCutFrom(std::string str, std::string delim);