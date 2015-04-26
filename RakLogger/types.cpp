#include "main.h"

PacketParamType ConvertParamType(string type)
{
	type = StringCutTo(type, " :", false);
	if (type.empty())
		return PPT_UNKNOWN;

	struct
	{
		string _name;
		PacketParamType _type;
	} types[] = {
		{ "bool", PPT_BOOL },
		{ "char", PPT_CHAR },
		{ "byte", PPT_BYTE },
		{ "short", PPT_SHORT },
		{ "ushort", PPT_USHORT },
		{ "int", PPT_INT },
		{ "uint", PPT_UINT },
		{ "int64", PPT_INT64 },
		{ "uint64", PPT_UINT64 },
		{ "float", PPT_FLOAT },
		{ "double", PPT_DOUBLE },
		{ "normquat", PPT_NORMQUAT },
		{ "normvec", PPT_NORMVEC },
		{ "vec", PPT_VEC },
		{ "string", PPT_STRING },
		{ "longstring", PPT_LONGSTRING },
		{ "array", PPT_ARRAY },
		{ "floatvec", PPT_FLOATVEC },
		{ "floatquat", PPT_FLOATQUAT },
		{ "fixedstring", PPT_FIXEDSTRING },
		{ "encodedstring", PPT_ENCODEDSTRING },
		{ "healtharmor", PPT_HEALTHARMOR },
		{ "playermarker", PPT_PLAYERMARKER },
		{ "weapon", PPT_WEAPON },

	};
	for each (auto &t in types)
	{
		if (boost::algorithm::iequals(t._name, type))
			return t._type;
	}
	return PPT_UNKNOWN;
}

PacketParamClass GetParamClass(PacketParamType type)
{
	switch (type)
	{
		case PPT_BOOL:
		case PPT_CHAR:
		case PPT_BYTE:
		case PPT_SHORT:
		case PPT_USHORT:
		case PPT_INT:
		case PPT_UINT:
		case PPT_INT64:
		case PPT_UINT64:
			return PPC_INTEGER;
		case PPT_FLOAT:
		case PPT_DOUBLE:
			return PPC_FLOAT;
		case PPT_NORMQUAT:
		case PPT_NORMVEC:
		case PPT_VEC:
		case PPT_STRING:
		case PPT_LONGSTRING:
		case PPT_ARRAY:
		case PPT_FLOATVEC:
		case PPT_FLOATQUAT:
		case PPT_FIXEDSTRING:
		case PPT_ENCODEDSTRING:
		case PPT_HEALTHARMOR:
		case PPT_PLAYERMARKER:
		case PPT_WEAPON:
			return PPC_STRING;
	}
	return PPC_UNKNOWN;
}