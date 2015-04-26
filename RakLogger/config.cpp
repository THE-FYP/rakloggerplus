#include "main.h"

CConfig *cfg = new CConfig();
stPacketEntry CConfig::_badPacketInfo;

CConfigBase::CConfigBase(void)
{
	_ptree.clear();
	_path.clear();
}

bool CConfigBase::Load(string file)
{
	if (file.empty())
		return false;
	_path = file;
	try
	{
		boost::property_tree::ini_parser::read_ini(file, _ptree);
	}
	catch (boost::property_tree::ini_parser_error &err)
	{
		Log( err.what(), LOGOUTPUT_FILE );
		return false;
	}
	return true;
}

bool CConfig::Load(void)
{
	_sessionTime = GetDateTime();
	std::replace(_sessionTime.begin(), _sessionTime.end(), ':', '.');
	_ptree.clear();
	return CConfigBase::Load(_configFilename) && ReadSettings();
}

bool CConfig::ReadSettings(void)
{
	if (_ptree.empty())
		return false;

	try
	{
		decltype(_ptree) &ptset = _ptree.get_child("settings");

		_logging = ptset.get("logging", false);
		_invertedIgnore = ptset.get("invertedIgnore", false);
		_timestamp = ptset.get("printTimestamp", false);
		_JSONLogging = ptset.get("createJSONLog", false);
		_logEachSessionSingly = ptset.get("logEachSessionSingly", false);

		_dumpInFile = ptset.get("dumpInFile", false);
		_dumpInChat = ptset.get("dumpInChat", false);
		_dumpInConsole = ptset.get("dumpInConsole", false);
		_logInFile = ptset.get("logInFile", false);
		_logInChat = ptset.get("logInChat", false);
		_logInConsole = ptset.get("logInConsole", false);
		_logParamsInFile = ptset.get("logParamsInFile", false);
		_logParamsInChat = ptset.get("logParamsInChat", false);
		_logParamsInConsole = ptset.get("logParamsInConsole", false);

		_logIncomingRPC = ptset.get("logIncomingRPC", false);
		_logOutcomingRPC = ptset.get("logOutcomingRPC", false);
		_logIncomingPacket = ptset.get("logIncomingPacket", false);
		_logOutcomingPacket = ptset.get("logOutcomingPacket", false);

		_drawParams = ptset.get("drawParams", false);
		_drawDump = ptset.get("drawDump", false);

		for (size_t i = 0; i < _packetsSize; ++i)
		{
			string rpc = _ptree.get("names.rpc_" + to_string(i), "");
			string packet = _ptree.get("names.packet_" + to_string(i), "");
			_rpc[i]._name = rpc;
			_rpc[i]._id = (int) i;
			_rpc[i]._type = stPacketEntry::RPC;
			_packet[i]._name = packet;
			_packet[i]._id = (int) i;
			_packet[i]._type = stPacketEntry::PACKET;
		}
	}
	catch (exception &e)
	{
		Log(e.what(), LOGOUTPUT_FILE);
		return false;
	}

	try
	{
		// read ignore
		auto pt = _ptree.get_child_optional("ignore");
		if (pt)
		{
			for each (auto &entry in pt.get())
			{
				string value = entry.second.get_value<std::string>("");
				if (value.empty())
					continue;

				stPacketEntry &pkt = GetPacketByName(entry.first);
				if (!pkt)
					continue;
				if (boost::algorithm::icontains(value, "recv"))
					pkt._ignoreIncoming = true;
				if (boost::algorithm::icontains(value, "send"))
					pkt._ignoreOutcoming = true;
			}
		}

		// read packet params
		string sec[3] = { "params_in", "params_out", "params_any" };
		for (size_t p = 0; p < _countof(sec); ++p)
		{
			auto pt = _ptree.get_child_optional(sec[p]);
			if (pt)
			{
				for each (auto &entry in pt.get())
				{
					string params = entry.second.get_value<std::string>("");
					if (params.empty())
						continue;

					stPacketEntry &pkt = GetPacketByName(entry.first);
					if (!pkt)
						continue;

					list<stPacketParam> parlist;
					if (!ParseParams(params, parlist))
						continue;

					switch (p)
					{
					case 0:
						pkt._incomingParamsStr = params;
						pkt._incomingParams = parlist;
						break;
					case 1:
						pkt._outcomingParamsStr = params;
						pkt._outcomingParams = parlist;
						break;
					case 2:
						pkt._incomingParamsStr = pkt._outcomingParamsStr = params;
						pkt._incomingParams = pkt._outcomingParams = parlist;
						break;
					}
				}
			}
		}
	}
	catch (boost::property_tree::ptree_error &)
	{
		return true;
	}
	catch (exception &e)
	{
		Log(e.what());
		return false;
	}
	return true;
}

stPacketEntry &CConfig::GetPacketEntry(stPacketEntry *parr, size_t id)
{
	if (id >= _packetsSize) return _badPacketInfo;
	return parr[id];
}

stPacketEntry &CConfig::GetPacketEntry(stPacketEntry *parr, std::string name, bool part)
{
	// if (boost::algorithm::all(name, boost::algorithm::is_digit()))
	//	return GetPacketInfo(boost::lexical_cast<size_t>(name), packet);
	for (int i = 0; i < _packetsSize; ++i)
	{
		if (part ? boost::algorithm::icontains(parr[i]._name, name) : boost::algorithm::iequals(name, parr[i]._name))
			return parr[i];
	}
	return _badPacketInfo;
}

stPacketEntry &CConfig::GetPacketInfo(size_t id)
{
	return GetPacketEntry(_packet, id);
}

stPacketEntry &CConfig::GetRPCInfo(size_t id)
{
	return GetPacketEntry(_rpc, id);
}

stPacketEntry &CConfig::GetPacketByName(std::string name)
{
	stPacketEntry &rpc = GetPacketEntry(_rpc, name);
	if (rpc)
		return rpc;
	stPacketEntry &pkt = GetPacketEntry(_packet, name);
	if (pkt)
		return pkt;
	return _badPacketInfo;
}

stPacketEntry &CConfig::FindPacket(std::string partOfName)
{
	stPacketEntry &rpc = GetPacketEntry(_rpc, partOfName, true);
	if (rpc)
		return rpc;
	stPacketEntry &pkt = GetPacketEntry(_packet, partOfName, true);
	if (pkt)
		return pkt;
	return _badPacketInfo;
}

bool CConfig::ParseParams(string params, list<stPacketParam> &out)
{
	out.clear();
	list<string> parlist;
	boost::split(parlist, params, boost::is_any_of(",;/") || boost::is_space(), boost::token_compress_on);
	for each (string s in parlist)
	{
		boost::trim(s);
		if (s.empty())
			continue;

		list<string> p;
		regex reg(R"(^(\w+):(\w+)(\[(\w+)\])?(\?(\w+))?$)");
		smatch match;
		if (!regex_match(s, match, reg))
		{
			Log(formatstr("Params parsing error: bad args (%1%).") % s, LOGOUTPUT_FILE);
			break;
		}
		PacketParamType type = ConvertParamType(match[2]);
		if (type == PPT_UNKNOWN)
		{
			Log(formatstr("Params parsing error: unknown type (%1%).") % s, LOGOUTPUT_FILE);
			break;
		}
		stPacketParam param;
		if (match[4].matched) // parse size value
		{
			string sizeStr = match[4];
			if (boost::all(sizeStr, boost::is_digit()))
			{
				param._size = boost::lexical_cast<size_t>(sizeStr);
			}
			else
			{
				auto &it = std::find(out.begin(), out.end(), sizeStr);
				if (it == out.end())
				{
					Log(formatstr("Type parsing error: parameter %1% is unknown") % sizeStr, LOGOUTPUT_FILE);
					break;
				}
				if (GetParamClass(it->_type) != PPC_INTEGER)
				{
					Log("Type parsing error: size value can be only integer", LOGOUTPUT_FILE);
					break;
				}
				param._size = 0;
				param._sizeLink = sizeStr;
				it->_isArraySize = true;
			}
		}
		else
		{
			param._size = 0;
			if (type == PPT_ARRAY || type == PPT_ENCODEDSTRING || type == PPT_FIXEDSTRING)
			{
				Log(formatstr("Type parsing error: type %1% must have definite size") % match[2], LOGOUTPUT_FILE);
				break;
			}
		}
		if (match[6].matched) // parse condition
		{
			string condStr = match[6];
			auto &it = std::find(out.begin(), out.end(), condStr);
			if (it == out.end())
			{
				Log(formatstr("Type parsing error: parameter %1% is unknown") % condStr, LOGOUTPUT_FILE);
				break;
			}
			if (GetParamClass(it->_type) == PPC_STRING || GetParamClass(it->_type) == PPC_UNKNOWN)
			{
				Log("Type parsing error: condition value cannot be string", LOGOUTPUT_FILE);
				break;
			}
			if (it->_size > 0 || !it->_sizeLink.empty())
			{
				Log("Type parsing error: array cannot be a condition", LOGOUTPUT_FILE);
				break;
			}
			param._condLink = condStr;
			it->_isCondition = true;
		}
		param._name = match[1];
		param._type = type;
		param._typeStr = match[2];
		out.push_back(param);
	}
	return !out.empty();
}