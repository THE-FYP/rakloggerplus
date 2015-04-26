#include "main.h"

std::ofstream g_Log;
boost::property_tree::ptree g_PTreeJSONLog;

void Log(std::string msg)
{
	Log(msg, LOGOUTPUT_FILE);
	Log(msg, LOGOUTPUT_CHAT, COLOR_INFO);
}

void Log(std::string msg, LogOutput output, D3DCOLOR color, bool timestamp)
{
	switch (output)
	{
		case LOGOUTPUT_FILE:
			if (!g_Log.is_open())
			{
				string path = "SAMPFUNCS\\" + (cfg->_logEachSessionSingly ? ("RakLogger_" + cfg->_sessionTime + ".log") : "RakLogger.log");
				g_Log.open(path, std::fstream::app);
			}
			if (g_Log.fail())
				return;
			if (cfg->_timestamp && timestamp)
				g_Log << "[" << GetTime() << "] ";
			g_Log << msg << endl;
			break;

		case LOGOUTPUT_CHAT:
		{
			list<string> strings;
			boost::split(strings, msg, boost::is_any_of("\n\r"));
			if (SAMP != nullptr)
			{
				for each (auto &str in strings)
				{
					SAMP->getChat()->AddChatMessage(color, "%s", str.c_str());
				}
			}
			break;
		}

		case LOGOUTPUT_CONSOLE:
			SF->LogConsole(msg);
			break;
	}
}

void LogJSON(const BSParamsContainer_t &paramlist, const stRakNetHookParams *params, const stPacketEntry &pktinfo, bool incoming)
{
	boost::property_tree::ptree ptparams, ptparam, ptentry, &ptree = g_PTreeJSONLog;
	for each (auto &p in paramlist) // make params array
	{
		ptparam.clear();
		ptparam.put("name", p.first._name);
		ptparam.put("value", p.second);
		ptparam.put("type", p.first._typeStr);
		if (p.first._size != 0) ptparam.put("size", p.first._size);
		ptparams.push_back(make_pair("", ptparam));
	}
	// write known packet info
	ptentry.put("id", pktinfo._id);
	ptentry.put("name", pktinfo._name);
	ptentry.put("lenBytes", params->bitStream->GetNumberOfBytesUsed());
	ptentry.put("lenBits", params->bitStream->GetNumberOfBitsUsed());
	ptentry.put("unreadBits", params->bitStream->GetNumberOfUnreadBits());
	if (!incoming)
	{
		// add additional packet info if it is outcoming
		ptentry.put("priority", params->priority);
		ptentry.put("reliability", params->reliability);
		ptentry.put("orderingChannel", int(params->orderingChannel));
	}
	ptentry.put("data", BinToHex(params->bitStream->GetData(), params->bitStream->GetNumberOfBytesUsed()));
	ptentry.put("datetime", GetDateTime());
	ptentry.add_child("params", ptparams);
	string path = formatstr("%1%.%2%") % (incoming ? "incoming" : "outcoming") % (pktinfo._type == stPacketEntry::RPC ? "rpc" : "packet");
	boost::property_tree::ptree &ptblock = ptree.get_child_optional(path) ? ptree.get_child(path) : ptree.put(path, "");
	ptblock.push_back(make_pair("", ptentry));
}

void SaveJSON(string filename)
{
	string name = "RakLogger_" + cfg->_sessionTime;
	if (!filename.empty())
		name = filename;
	boost::property_tree::json_parser::write_json("SAMPFUNCS\\" + name + ".json", g_PTreeJSONLog);
}

std::string GetDateTime(void)
{
	char buf[32];
	time_t t = time(0);
	strftime(buf, sizeof(buf) - 1, "%d.%m.%Y-%H:%M:%S", localtime(&t));
	return buf;
}

std::string GetTime(void)
{
	char buf[32];
	SYSTEMTIME time;
	GetLocalTime(&time);
	sprintf_s(buf, "%02d:%02d:%02d.%03d", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
	return buf;
}

std::string GetFormattedPacketName(stPacketEntry &pe)
{
	if (!pe._name.empty())
		return formatstr("%s[%d]") % pe._name % pe._id;
	else
		return formatstr("[%d]") % pe._id;
}

string FormatBitStreamParams(BitStreamEx *bs, BSParamsContainer_t &params)
{
	string result;
	if (params.empty())
		return result;
	for each (auto &p in params)
	{
		result += formatstr("%1%: %2%\n") % p.first._name % p.second;
	}
	result += formatstr("unreadBits: %1%") % bs->GetNumberOfUnreadBits();
	bs->ResetReadPointer();
	return result;
}

BSParamsContainer_t GetBitStreamParams(BitStreamEx *bs, const std::list<stPacketParam> &params)
{
	BSParamsContainer_t result;
	if (!params.empty())
	{
		std::map<string, bool> _condMap;
		std::map<string, size_t> _arrsizeMap;
		auto forEach = [&_arrsizeMap](std::function<void()> func, const stPacketParam &par){
			size_t size = par._size == 0 ? 1 : par._size;
			if (!par._sizeLink.empty())
				size = _arrsizeMap[par._sizeLink];
			for (size_t i = 0; i < size; ++i)
			{
				func();
			}
		};
		for each (auto &par in params)
		{
			switch (par._type)
			{
				case PPT_BOOL:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							bool val = false;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val;
							if (par._isArraySize) _arrsizeMap[par._name] = val;
							result.push_back(make_pair(par, formatstr("%1%") % val));
						}, par);
					}
					break;
				}
				case PPT_CHAR:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							char val = 0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val != 0;
							if (par._isArraySize) _arrsizeMap[par._name] = val;
							result.push_back(make_pair(par, formatstr("%1% (0x%1$X)") % int(val)));
						}, par);
					}
					break;
				}
				case PPT_BYTE:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							byte val = 0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val != 0;
							if (par._isArraySize) _arrsizeMap[par._name] = val;
							result.push_back(make_pair(par, formatstr("%1% (0x%1$X)") % int(val)));
						}, par);
					}
					break;
				}
				case PPT_SHORT:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							short val = 0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val != 0;
							if (par._isArraySize) _arrsizeMap[par._name] = val;
							result.push_back(make_pair(par, formatstr("%1% (0x%1$X)") % val));
						}, par);
					}
					break;
				}
				case PPT_USHORT:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							unsigned short val = 0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val != 0;
							if (par._isArraySize) _arrsizeMap[par._name] = val;
							result.push_back(make_pair(par, formatstr("%1% (0x%1$X)") % val));
						}, par);
					}
					break;
				}
				case PPT_INT:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							int val = 0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val != 0;
							result.push_back(make_pair(par, formatstr("%1% (0x%1$X)") % val));
						}, par);
					}
					break;
				}
				case PPT_UINT:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							unsigned int val = 0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val != 0;
							if (par._isArraySize) _arrsizeMap[par._name] = val;
							result.push_back(make_pair(par, formatstr("%1% (0x%1$X)") % val));
						}, par);
					}
					break;
				}
				case PPT_INT64:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							__int64 val = 0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val != 0;
							if (par._isArraySize) _arrsizeMap[par._name] = (size_t)val;
							result.push_back(make_pair(par, formatstr("%1% (0x%1$X)") % val));
						}, par);
					}
					break;
				}
				case PPT_UINT64:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							unsigned __int64 val = 0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = val != 0;
							if (par._isArraySize) _arrsizeMap[par._name] = (size_t)val;
							result.push_back(make_pair(par, formatstr("%1% (0x%1$X)") % val));
						}, par);
					}
					break;
				}
				case PPT_FLOAT:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							float val = 0.0f;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = fabs(val) >= FLT_EPSILON;
							result.push_back(make_pair(par, formatstr("%1$.4f") % val));
						}, par);
					}
					break;
				}
				case PPT_DOUBLE:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							double val = 0.0;
							bs->Read(val);
							if (par._isCondition) _condMap[par._name] = fabs(val) >= DBL_EPSILON;
							result.push_back(make_pair(par, formatstr("%1$.4f") % val));
						}, par);
					}
					break;
				}
				case PPT_NORMQUAT:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							float quat[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
							bs->ReadNormQuat(quat[0], quat[1], quat[2], quat[3]);
							result.push_back(make_pair(par, formatstr("%1%, %2%, %3%, %4%") % quat[0] % quat[1] % quat[2] % quat[3]));
						}, par);
					}
					break;
				}
				case PPT_NORMVEC:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							float vec[3] = { 0.0f, 0.0f, 0.0f };
							bs->ReadNormVector(vec[0], vec[1], vec[2]);
							result.push_back(make_pair(par, formatstr("%1%, %2%, %3%") % vec[0] % vec[1] % vec[2]));
						}, par);
					}
					break;
				}
				case PPT_VEC:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							float vec[3] = { 0.0f, 0.0f, 0.0f };
							bs->ReadVector(vec[0], vec[1], vec[2]);
							result.push_back(make_pair(par, formatstr("%1%, %2%, %3%") % vec[0] % vec[1] % vec[2]));
						}, par);
					}
					break;
				}
				case PPT_STRING:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							char str[256];
							byte val = 0;
							bs->Read(val);
							bs->Read(str, val);
							str[val] = '\0';
							result.push_back(make_pair(par, formatstr("%1% [%2%]") % str % int(val)));
						}, par);
					}
					break;
				}
				case PPT_LONGSTRING:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							unsigned int val = 0;
							bs->Read(val);
							string str = bs->ReadString(val);
							result.push_back(make_pair(par, formatstr("%1% [%2%]") % str % val));
						}, par);
					}
					break;
				}
				case PPT_ARRAY:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						size_t size = par._sizeLink.empty() ? par._size : _arrsizeMap[par._sizeLink];
						if (size >= (size_t)BITS_TO_BYTES(bs->GetNumberOfUnreadBits()))
							size = (size_t)BITS_TO_BYTES(bs->GetNumberOfUnreadBits());
						byte *arr = new byte[size];
						bs->Read((char *) arr, size);
						result.push_back(make_pair(par, formatstr("%1%") % BinToHex(arr, size)));
						delete[] arr;
					}
					break;
				}
				case PPT_FLOATQUAT:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							float quat[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
							bs->Read(quat);
							result.push_back(make_pair(par, formatstr("%1%, %2%, %3%, %4%") % quat[0] % quat[1] % quat[2] % quat[3]));
						}, par);
					}
					break;
				}
				case PPT_FLOATVEC:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							float vec[3] = { 0.0f, 0.0f, 0.0f };
							bs->Read(vec);
							result.push_back(make_pair(par, formatstr("%1%, %2%, %3%") % vec[0] % vec[1] % vec[2]));
						}, par);
					}
					break;
				}
				case PPT_FIXEDSTRING:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						size_t size = par._sizeLink.empty() ? par._size : _arrsizeMap[par._sizeLink];
						string str = bs->ReadString(size);
						result.push_back(make_pair(par, formatstr("%1% [%2%]") % str % size));
					}
					break;
				}
				case PPT_ENCODEDSTRING:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						size_t size = par._sizeLink.empty() ? par._size : _arrsizeMap[par._sizeLink];
						if (size >= (size_t)BITS_TO_BYTES(bs->GetNumberOfUnreadBits()))
							size = (size_t)BITS_TO_BYTES(bs->GetNumberOfUnreadBits());
						char *str = new char[size];
						SF->getRakNet()->DecodeString(str, size, bs);
						result.push_back(make_pair(par, formatstr("%1% [%2%]") % str % size));
						delete[] str;
					}
					break;
				}
				case PPT_HEALTHARMOR:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							byte byteHealthArmour;
							byte byteHealth, byteArmour;
							byte byteArmTemp = 0, byteHlTemp = 0;

							bs->Read(byteHealthArmour);
							byteArmTemp = byteHealthArmour & 0x0F;
							byteHlTemp = byteHealthArmour >> 4;
							if (byteArmTemp == 0xF) byteArmour = 100;
							else if (byteArmTemp == 0) byteArmour = 0;
							else byteArmour = byteArmTemp * 7;
							if (byteHlTemp == 0xF) byteHealth = 100;
							else if (byteHlTemp == 0) byteHealth = 0;
							else byteHealth = byteHlTemp * 7;

							result.push_back(make_pair(par, formatstr("%1%, %2% [%3%]") % int(byteHealth) % int(byteArmour) % int(byteHealthArmour)));
						}, par);
					}
					break;
				}
				case PPT_PLAYERMARKER:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							unsigned short playerId;
							bool isActive;
							short pos[3];
							bs->Read(playerId);
							bs->Read(isActive);
							if (isActive)
							{
								bs->Read(pos[0]);
								bs->Read(pos[1]);
								bs->Read(pos[2]);
							}
							result.push_back(make_pair(par, formatstr("playerId: %1%, isActive: %2%, pos: %3%, %4%, %5%") % playerId % isActive % pos[0] % pos[1] % pos[2]));
						}, par);
					}
				}
				case PPT_WEAPON:
				{
					if (par._condLink.empty() || _condMap[par._condLink])
					{
						forEach([&]()
						{
							byte slot, id;
							unsigned short ammo;
							if (bs->GetNumberOfUnreadBits() > 0)
							{
								bs->Read(slot);
								bs->Read(id);
								bs->Read(ammo);
								result.push_back(make_pair(par, formatstr("slot: %1%, weapon: %2%, ammo: %3%") % int(slot) % int(id) % ammo));
							}
						}, par);
					}
				}
			}
		}
	}
	return result;
}

string DumpMem(byte *address, size_t len)
{
	string dump;
	dump.clear();
	if (address == nullptr || len == 0)
		return dump;

	string shex, stext;
	size_t pos = 0;
	char tmp[0x10];
	while (pos < len)
	{
		shex.clear();
		stext.clear();
		sprintf_s(tmp, "%04X  ", pos);
		dump += tmp;
		int spaces = 16;
		for (int i = 0; i < 0x10 && pos < len; i++)
		{
			byte ch = address[pos];
			sprintf_s(tmp, "%02X", (byte) ch);
			shex += tmp;
			shex += ' ';
			if (i == 7)
				shex += ' ';
			stext += ch < 32 ? '.' : ch;
			pos++;
			spaces--;
		}
		dump += shex;
		for (int i = 0; i < (spaces + 1) * 3; i++)
			dump += ' ';
		dump += stext;
		if (pos < len)
			dump += '\n';
	}
	return dump;
}

std::string BinToHex(byte *bin, size_t size)
{
	std::string hex = "";
	if (bin == nullptr || size == 0)
		return "";

	char	b[4];
	for (size_t i = 0; i < size; i++)
	{
		_snprintf_s(b, sizeof(b) - 1, "%02X", bin[i]);
		hex += b;
	}

	return hex;
}

std::string StringCutTo(std::string str, std::string delim, bool strict)
{
	std::string result;
	if (str.empty() || delim.empty())
		return result;
	size_t p = str.find_first_of(delim, 0);
	if (p != string::npos)
		result = str.substr(0, p);
	else if (strict == false)
		result = str;
	return result;
}

std::string StringCutFrom(std::string str, std::string delim)
{
	std::string result;
	if (str.empty() || delim.empty())
		return result;
	size_t p = str.find_first_of(delim);
	if (p != string::npos)
		p = str.find_first_not_of(delim, p);
	if (p != string::npos)
		result = str.substr(p, str.length());
	return result;
}