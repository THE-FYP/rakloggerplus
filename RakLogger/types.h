enum PacketParamType
{
	PPT_UNKNOWN = 0,
	PPT_BOOL,
	PPT_CHAR,
	PPT_BYTE,
	PPT_SHORT,
	PPT_USHORT,
	PPT_INT,
	PPT_UINT,
	PPT_INT64,
	PPT_UINT64,
	PPT_FLOAT,
	PPT_DOUBLE,
	PPT_NORMQUAT,
	PPT_NORMVEC,
	PPT_VEC,
	PPT_STRING,
	PPT_LONGSTRING,
	PPT_ARRAY,
	PPT_FLOATVEC,
	PPT_FLOATQUAT,
	PPT_FIXEDSTRING,
	PPT_ENCODEDSTRING,
	PPT_HEALTHARMOR,
	PPT_PLAYERMARKER,
	PPT_WEAPON,
};

enum PacketParamClass
{
	PPC_UNKNOWN,
	PPC_INTEGER,
	PPC_FLOAT,
	PPC_STRING,
};

struct stPacketParam
{
	bool operator==(const char *str)
	{
		return _name == str;
	}
	bool operator==(const std::string &str)
	{
		return _name == str;
	}
	std::string						_name;
	std::string						_typeStr;
	PacketParamType					_type = PPT_UNKNOWN;
	size_t							_size = 0;
	std::string						_condLink;
	std::string						_sizeLink;
	bool							_isCondition = false;
	bool							_isArraySize = false;
};

PacketParamType ConvertParamType(std::string type);
PacketParamClass GetParamClass(PacketParamType type);