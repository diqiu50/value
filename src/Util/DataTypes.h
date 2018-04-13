////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
#ifndef Util_DataType_h
#define Util_DataType_h

namespace value
{
enum class DataType
{
	eInvalid,

	eBool,
	eChar,
	eInt8,
	eInt16,
	eInt32,
	eInt64,
	eU8,
	eU16,
	eU32,
	eU64,
	eFloat,
	eDouble,
	eString,
	eLongString,
	eShortString,
	eNull,
	eMaxDataType
};
}
#endif
