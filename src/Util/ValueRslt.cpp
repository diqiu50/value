////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
#include "Util/ValueRslt.h"

ValueRslt::ValueRslt()
:
mType(DataType::eNull)
{
	mValue.obj_value= 0;
	mDoubleBits = -1;
}


ValueRslt::ValueRslt(const ValueRslt &rhs)
{
	mType = DataType::eNull;
	mValue.obj_value= 0;
	if (rhs.mType == DataType::eLongString)
	{
		int len = *(int*)(rhs.mValue.obj_value);
		mValue.obj_value = new char[len + sizeof(int)];
		AOS_COUNT_VALUE_RSLT_MEMORY;

		memcpy(mValue.obj_value, rhs.mValue.obj_value, len + sizeof(int));
	}
	else
	{
		mValue = rhs.mValue;
	}
	mType = rhs.mType;
	mDoubleBits = rhs.mDoubleBits;
}

ValueRslt::ValueRslt(const u64 value)
{
	mType = DataType::eU64;
	mDoubleBits = -1;
	mValue.u64_value = value;
}


ValueRslt::ValueRslt(const bool value)
{
	mType = DataType::eBool;
	mDoubleBits = -1;
	mValue.bool_value = value;
}


ValueRslt::ValueRslt(const i64 value)
{
	mType = DataType::eInt64;
	mDoubleBits = -1;
	mValue.i64_value = value;
}


ValueRslt::ValueRslt(const double value)
{
	mType = DataType::eDouble;
	mDoubleBits = -1;
	mValue.double_value = value;
}


ValueRslt::ValueRslt(const string &value)
{
	mDoubleBits = -1;
	int len = value.length();
	if (len + sizeof(int) >= eDefaultSize)
	{
		mType = DataType::eLongString;
		mValue.obj_value = new char[len + sizeof(int)];
		AOS_COUNT_VALUE_RSLT_MEMORY;

		*(int*)(mValue.obj_value) = len;
		memcpy((char *)(mValue.obj_value) + sizeof(int), value.data(), len);
	}
	else
	{
		mType = DataType::eShortString;
		*(int*)(mValue.str_value) = len;
		memcpy((char *)(mValue.str_value) + sizeof(int), value.data(), len);
	}
}


ValueRslt::ValueRslt(const char value)
{
	mType = DataType::eChar;
	mDoubleBits = -1;
	mValue.char_value = value;
}


ValueRslt::~ValueRslt()
{
	deleteMemory();
}


void
ValueRslt::reset()
{
}


void
ValueRslt::setDouble(const double vv)
{
	deleteMemory();
	mType = DataType::eDouble;
	mValue.double_value = vv;
}


void
ValueRslt::setU64(const u64 vv)
{
	deleteMemory();
	mType = DataType::eU64;
	mValue.u64_value = vv;
}


void
ValueRslt::setI64(const i64 vv)
{
	deleteMemory();
	mType = DataType::eInt64;
	mValue.i64_value = vv;
}


void
ValueRslt::setStr(const string &vv)
{
	int len = vv.length();
	if (len + sizeof(int) >= eDefaultSize)
	{
		if (mType == DataType::eLongString && mValue.obj_value && len < *(int*)(mValue.obj_value))
		{
			*(int*)(mValue.obj_value) = len;
			memcpy((char *)(mValue.obj_value) + sizeof(int), vv.data(), len);
			mType = DataType::eLongString;
			return;
		}
		deleteMemory();
		mValue.obj_value = new char[len + sizeof(int)];
		AOS_COUNT_VALUE_RSLT_MEMORY;

		*(int*)(mValue.obj_value) = len;
		memcpy((char *)(mValue.obj_value) + sizeof(int), vv.data(), len);
		mType = DataType::eLongString;
	}
	else
	{
		deleteMemory();
		mType = DataType::eShortString;
		*(int*)(mValue.str_value) = len;
		memcpy(mValue.str_value + sizeof(int), vv.data(), len);
	}
}


void
ValueRslt::setBool(const bool vv)
{
	deleteMemory();
	mType = DataType::eBool;
	mValue.bool_value = vv;
}


void
ValueRslt::setChar(const char vv)
{
	deleteMemory();
	mType = DataType::eChar;
	mValue.char_value = vv;
}


char
ValueRslt::getChar() const
{
	switch (mType)
	{
		case DataType::eBool:
			 return mValue.bool_value;
		case DataType::eChar:
			 return mValue.char_value;
		default:
			 OmnAlarm << "error" << enderr;
			 return mValue.char_value;
	}
	return mValue.char_value;
}


const char* const
ValueRslt::getCStr(int &len) const
{
	switch (mType)
	{
	case DataType::eShortString:
		 len = *(int*)(mValue.str_value);
		 return mValue.str_value + sizeof(int);
	case DataType::eLongString:
		 len = *(int*)(mValue.obj_value);
		 return (char *)(mValue.obj_value) + sizeof(int);
	case DataType::eDouble:
		 {
			char ch[eSPrintfSize+1];
			sprintf(ch, "%lf", mValue.double_value);
			len = strlen(ch);
			char *pos = ch + len - 1;
			while(*pos == '0')
			{
				*pos-- = '\0';
			}
			if(*pos == '.')
			{
				*pos = '\0';
			}
			return pos;
		 }
	case DataType::eInt64:
		 char ch[eSPrintfSize+1];
		 sprintf(ch, "%ld", mValue.i64_value);
		 len = strlen(ch);
		 return ch;
	default:
		OmnAlarm << "error" << enderr;
	}
	return nullptr;
}


const char*
ValueRslt::getCStrValue(int &len) const
{
	string rr = getStr();
	len =  rr.length();
	return rr.data();
}


BuffPtr
ValueRslt::getBuff() const
{
	aos_assert_r(mType == DataType::eBuff, nullptr);
	return *(BuffPtr*)mValue.obj_value;
}


DataType::E
ValueRslt::getType() const
{
	if (mType == DataType::eShortString || mType == DataType::eLongString)
		return DataType::eString;
	return mType;
}


ValueRslt&
ValueRslt::operator = (const ValueRslt &rhs)
{
	// a = a;
	if (this == &rhs) return *this;
	deleteMemory();

	mType = rhs.mType;
	if (mType == DataType::eLongString)
	{
		int len = *(int *)(rhs.mValue.obj_value);
		mValue.obj_value = new char[len + sizeof(int)];

		AOS_COUNT_VALUE_RSLT_MEMORY;
		memcpy(mValue.obj_value, rhs.mValue.obj_value, len + sizeof(int));
	}
	else
	{
		mValue = rhs.mValue;
	}
	mDoubleBits = rhs.mDoubleBits;
	return *this;
}


bool
ValueRslt::getBool() const
{
	switch(getType())
	{
	case DataType::eChar:
		 return mValue.char_value !=  0;
	case DataType::eBool:
		 return mValue.bool_value;
	case DataType::eInt64:
		 return mValue.i64_value !=  0;
	case DataType::eU64:
		 return mValue.u64_value !=  0;
	case DataType::eDouble:
		 return mValue.double_value !=  0;
	case DataType::eString:
		 return getStr().length() != 0;
	default:
		 OmnAlarm << enderr;
		 return false;
	}
	return false;
}


Set&
ValueRslt::getSet() const
{
	static Set return_value(DataType::eInt64);
	aos_assert_r(mType == DataType::eSet
			|| mType == DataType::eSetCount, return_value);
	Set* set= (Set*)(mValue.obj_value);
	return *set;
}


u64
ValueRslt::getU64() const
{
	switch(mType)
	{
	case DataType::eChar:
	case DataType::eBool:
		 return mValue.bool_value;
	case DataType::eInt64:
		 return mValue.i64_value;
	case DataType::eU64:
	case DataType::eU16:
		 return mValue.u64_value;
	case DataType::eDouble:
		 return mValue.double_value;
	default:
		 OmnAlarm << enderr;
		 return 0;
	}
	return 0;
}


i64
ValueRslt::getI64() const
{
	switch(mType)
	{
	case DataType::eChar:
	case DataType::eBool:
		 return mValue.bool_value;
	case DataType::eInt64:
		 return mValue.i64_value;
	case DataType::eU64:
		 return mValue.u64_value;
	case DataType::eDouble:
		 return mValue.double_value;
	default:
		 OmnAlarm << enderr;
		 return 0;
	}
	return 0;
}


double
ValueRslt::getDouble() const
{
	switch(mType)
	{
	case DataType::eChar:
		 return mValue.char_value;
	case DataType::eBool:
		 return mValue.bool_value;
	case DataType::eDouble:
		 return mValue.double_value;
	case DataType::eInt64:
		 return mValue.i64_value;
	case DataType::eU64:
		 return mValue.u64_value;
	default:
		 OmnAlarm << enderr;
		 return 0;
	}
	return 0;

}


string
ValueRslt::getStr() const
{
	char ch[eSPrintfSize+1];
	memset(ch, 0, eSPrintfSize + 1);
	switch(getType())
	{
	case DataType::eChar:
		 ch[0] =  mValue.char_value;
		 ch[1] = 0;
		 break;

	case DataType::eBool:
		 if (mValue.bool_value)
		 	strcpy(ch, "true");
		 else
		 	strcpy(ch, "false");
		 break;

	case DataType::eInt64:
		 sprintf(ch, "%ld", mValue.i64_value);
		 break;

	case DataType::eU64:
		 sprintf(ch, "%lu", mValue.u64_value);
		 break;

	case DataType::eDouble:
		 {
			 sprintf(ch, "%lf", mValue.double_value);
			 int len = strlen(ch);
			 char *pos = ch + len - 1;
			 while(*pos == '0')
			 {
				 *pos-- = '\0';
			 }
			 if(*pos == '.')
			 {
				 *pos = '\0';
			 }
			 break;
		 }

	case DataType::eString:
		 {
			 int len = 0;
			 if(mType == DataType::eShortString)
			 {
				 len = *(int *)(mValue.str_value);
				 string v1(mValue.str_value + sizeof(int), len);
			 	 return v1;
			 }
			 if (mType == DataType::eLongString)
			 {
				 len = *(int*)(mValue.obj_value);
				 string v1((char *)((char *)(mValue.obj_value) + sizeof(int)), len);
			 	 return v1;
			 }
			 OmnAlarm << enderr;
			 break;
		 }

	case DataType::eNull:
		return "__NULL__";

	default:
		OmnAlarm << enderr;
		return "";
	}
	string vv(ch, strlen(ch));
	return vv;
}


void
ValueRslt::deleteMemory()
{
	if (mType == DataType::eLongString)
	{
		delete [] (char*)mValue.obj_value;
		return;
	}
	return;
}


bool
ValueRslt::operator < (const ValueRslt &rhs) const
{
	if (isNull()) return true;
	if (rhs.isNull()) return false;
	DataType::E type = DataType::autoTypeConvert(getType(), rhs.getType());
	return doComparison(Opr::eOpr_lt, type, *this, rhs);
}


bool
ValueRslt::operator <= (const ValueRslt &rhs) const
{
	if (isNull()) return true;
	if (rhs.isNull()) return false;
	DataType::E type = DataType::autoTypeConvert(getType(), rhs.getType());
	return doComparison(Opr::eOpr_le, type, *this, rhs);
}


bool
ValueRslt::operator > (const ValueRslt &rhs) const
{
	if (isNull()) return true;
	if (rhs.isNull()) return false;
	DataType::E type = DataType::autoTypeConvert(getType(), rhs.getType());
	return doComparison(Opr::eOpr_gt, type, *this, rhs);
}


bool
ValueRslt::operator >= (const ValueRslt &rhs) const
{
	if (isNull()) return true;
	if (rhs.isNull()) return false;
	DataType::E type = DataType::autoTypeConvert(getType(), rhs.getType());
	return doComparison(Opr::eOpr_ge, type, *this, rhs);
}


bool
ValueRslt::operator == (const ValueRslt &rhs) const
{
	if (isNull()) return false;
	if (rhs.isNull()) return false;
	DataType::E type = DataType::autoTypeConvert(getType(), rhs.getType());
	return doComparison(Opr::eOpr_eq , type, *this, rhs);
}


bool
ValueRslt::doComparison(
		const Opr opr,
		const DataType::E value_type,
		const ValueRslt &lv,
		const ValueRslt &rv)
{
	switch (opr)
	{
	case Opr::eOpr_gt:
		switch (value_type)
		{
		case DataType::eChar:
			return (lv.getChar() > rv.getChar());
		case DataType::eBool:
			return (lv.getBool() > rv.getBool());
		case DataType::eString:
			return lv.getStr() > rv.getStr();
		case DataType::eU64:
			return (lv.getU64() > rv.getU64());
		case DataType::eDateTime:
		case DataType::eInt64:
			return (lv.getI64() > rv.getI64());
		case DataType::eDate:
			return (lv.getDate() > rv.getDate());
		case DataType::eDouble:
			return (lv.getDouble() > rv.getDouble());
		case DataType::eTime:
			return (lv.getTime() > rv.getTime());
		case DataType::eTimeStamp:
			return (lv.getTimeStamp() > rv.getTimeStamp());
		case DataType::eNumber:
			{
				if(lv.getType() == DataType::eNumber && rv.getType() == DataType::eDouble)
				{
					Number num(lv.getNumber().getPrecision(), lv.getNumber().getScale());
					num.setValue(rv.getDouble());
					return (lv.getNumber() > num);
				}
				else if(lv.getType() == DataType::eDouble && rv.getType() == DataType::eNumber)
				{
					Number num(rv.getNumber().getPrecision(), rv.getNumber().getScale());
					num.setValue(lv.getDouble());
					return (num > rv.getNumber());
				}
				else
				{
					return (lv.getNumber() > rv.getNumber());
				}
			}
		default:
			OmnAlarm << enderr;
			break;
		}
		break;

	case Opr::eOpr_ge:
		switch (value_type)
		{
		case DataType::eChar:
			return (lv.getChar() >= rv.getChar());
		case DataType::eBool:
			return (lv.getBool() >= rv.getBool());
		case DataType::eString:
			return lv.getStr() >= rv.getStr();
		case DataType::eU64:
			return (lv.getU64() >= rv.getU64());
		case DataType::eDateTime:
		case DataType::eInt64:
			return (lv.getI64() >= rv.getI64());
		case DataType::eDate:
			return (lv.getDate() >= rv.getDate());
		case DataType::eDouble:
			return (lv.getDouble() >= rv.getDouble());
		case DataType::eTime:
			return (lv.getTime() >= rv.getTime());
		case DataType::eTimeStamp:
			return (lv.getTimeStamp() >= rv.getTimeStamp());
		case DataType::eNumber:
			{
				if(lv.getType() == DataType::eNumber && rv.getType() == DataType::eDouble)
				{
					Number num(lv.getNumber().getPrecision(), lv.getNumber().getScale());
					num.setValue(rv.getDouble());
					return (lv.getNumber() >= num);
				}
				else if(lv.getType() == DataType::eDouble && rv.getType() == DataType::eNumber)
				{
					Number num(rv.getNumber().getPrecision(), rv.getNumber().getScale());
					num.setValue(lv.getDouble());
					return (num >= rv.getNumber());
				}
				else
				{
					return (lv.getNumber() >= rv.getNumber());
				}
			}
		default:
			OmnAlarm << enderr;
			break;
		}
		break;

	case Opr::eOpr_eq:
		switch (value_type)
		{
		case DataType::eChar:
			return (lv.getChar() == rv.getChar());
		case DataType::eBool:
			return (lv.getBool() == rv.getBool());
		case DataType::eString:
			return lv.getStr() == rv.getStr();
		case DataType::eU64:
			return (lv.getU64() == rv.getU64());
		case DataType::eDateTime:
		case DataType::eInt64:
			return (lv.getI64() == rv.getI64());
		case DataType::eDate:
			return (lv.getDate() == rv.getDate());
		case DataType::eDouble:
			return (lv.getDouble() == rv.getDouble());
		case DataType::eTime:
			return (lv.getTime() == rv.getTime());
		case DataType::eTimeStamp:
			return (lv.getTimeStamp() == rv.getTimeStamp());
		case DataType::eNumber:
			{
				if(lv.getType() == DataType::eNumber && rv.getType() == DataType::eDouble)
				{
					Number num(lv.getNumber().getPrecision(), lv.getNumber().getScale());
					num.setValue(rv.getDouble());
					return (lv.getNumber() == num);
				}
				else if(lv.getType() == DataType::eDouble && rv.getType() == DataType::eNumber)
				{
					Number num(rv.getNumber().getPrecision(), rv.getNumber().getScale());
					num.setValue(lv.getDouble());
					return (num == rv.getNumber());
				}
				else
				{
					return (lv.getNumber() == rv.getNumber());
				}
			}
		default:
			OmnAlarm << enderr;
			break;
		}
		break;

	case Opr::eOpr_lt:
		switch (value_type)
		{
		case DataType::eChar:
			return (lv.getChar() < rv.getChar());
		case DataType::eBool:
			return (lv.getBool() < rv.getBool());
		case DataType::eString:
			return lv.getStr() < rv.getStr();
		case DataType::eU64:
			return (lv.getU64() < rv.getU64());
		case DataType::eDateTime:
		case DataType::eInt64:
			return (lv.getI64() < rv.getI64());
		case DataType::eDate:
			return (lv.getDate() < rv.getDate());
		case DataType::eDouble:
			return (lv.getDouble() < rv.getDouble());
		case DataType::eTime:
			return (lv.getTime() < rv.getTime());
		case DataType::eTimeStamp:
			return (lv.getTimeStamp() < rv.getTimeStamp());
		case DataType::eNumber:
			{
				if(lv.getType() == DataType::eNumber && rv.getType() == DataType::eDouble)
				{
					Number num(lv.getNumber().getPrecision(), lv.getNumber().getScale());
					num.setValue(rv.getDouble());
					return (lv.getNumber() < num);
				}
				else if(lv.getType() == DataType::eDouble && rv.getType() == DataType::eNumber)
				{
					Number num(rv.getNumber().getPrecision(), rv.getNumber().getScale());
					num.setValue(lv.getDouble());
					return (num < rv.getNumber());
				}
				else
				{
					return (lv.getNumber() < rv.getNumber());
				}
			}
		default:
			OmnAlarm << enderr;
			break;
		}
		break;

	case Opr::eOpr_le:
		switch (value_type)
		{
		case DataType::eChar:
			return (lv.getChar() <= rv.getChar());
		case DataType::eBool:
			return (lv.getBool() <= rv.getBool());
		case DataType::eString:
			return lv.getStr() <= rv.getStr();
		case DataType::eU64:
			return (lv.getU64() <= rv.getU64());
		case DataType::eDateTime:
		case DataType::eInt64:
			return (lv.getI64() <= rv.getI64());
		case DataType::eDate:
			return (lv.getDate() <= rv.getDate());
		case DataType::eDouble:
			return (lv.getDouble() <= rv.getDouble());
		case DataType::eTime:
			return (lv.getTime() <= rv.getTime());
		case DataType::eTimeStamp:
			return (lv.getTimeStamp() <= rv.getTimeStamp());
		case DataType::eNumber:
			{
				if(lv.getType() == DataType::eNumber && rv.getType() == DataType::eDouble)
				{
					Number num(lv.getNumber().getPrecision(), lv.getNumber().getScale());
					num.setValue(rv.getDouble());
					return (lv.getNumber() <= num);
				}
				else if(lv.getType() == DataType::eDouble && rv.getType() == DataType::eNumber)
				{
					Number num(rv.getNumber().getPrecision(), rv.getNumber().getScale());
					num.setValue(lv.getDouble());
					return (num <= rv.getNumber());
				}
				else
				{
					return (lv.getNumber() <= rv.getNumber());
				}
			}
		default:
			OmnAlarm << enderr;
			break;
		}
		break;

	case Opr::eOpr_ne:
		switch (value_type)
		{
		case DataType::eChar:
			return (lv.getChar() != rv.getChar());
		case DataType::eBool:
			return (lv.getBool() != rv.getBool());
		case DataType::eString:
			return lv.getStr() != rv.getStr();
		case DataType::eU64:
			return (lv.getU64() != rv.getU64());
		case DataType::eDateTime:
		case DataType::eInt64:
			return (lv.getI64() != rv.getI64());
		case DataType::eDate:
			return (lv.getDate() != rv.getDate());
		case DataType::eDouble:
			return (lv.getDouble() != rv.getDouble());
		case DataType::eTime:
			return (lv.getTime() != rv.getTime());
		case DataType::eTimeStamp:
			return (lv.getTimeStamp() != rv.getTimeStamp());
		case DataType::eNumber:
			{
				if(lv.getType() == DataType::eNumber && rv.getType() == DataType::eDouble)
				{
					Number num(lv.getNumber().getPrecision(), lv.getNumber().getScale());
					num.setValue(rv.getDouble());
					return (lv.getNumber() != num);
				}
				else if(lv.getType() == DataType::eDouble && rv.getType() == DataType::eNumber)
				{
					Number num(rv.getNumber().getPrecision(), rv.getNumber().getScale());
					num.setValue(lv.getDouble());
					return (num != rv.getNumber());
				}
				else
				{
					return (lv.getNumber() != rv.getNumber());
				}
			}
		default:
			OmnAlarm << enderr;
			break;
		}
		break;
	default:
		OmnAlarm << enderr;
		break;
	}

	OmnAlarm << enderr;
	return false;
}


ValueRslt
ValueRslt::doArith(
		const ArithOpr::E opr,
		const DataType::E return_type,
		const ValueRslt &lv,
		const ValueRslt &rv)
{
	ValueRslt vv;

	switch (opr)
	{
		case ArithOpr::eAdd:
			switch (return_type)
			{
				case DataType::eU64:
					return ValueRslt(lv.getU64() + rv.getU64());
				case DataType::eInt64:
					return ValueRslt(lv.getI64() + rv.getI64());
				case DataType::eDouble:
					{
						//return ValueRslt(lv.getDouble() + rv.getDouble());
						int r_bits = lv.getDoubleBits();
						int l_bits = rv.getDoubleBits();
						int rslt_bits = max(r_bits, l_bits);
						ValueRslt value;
						value.setDouble(lv.getDouble() + rv.getDouble(), rslt_bits);
						return value;
					}
				case DataType::eNumber:
					return ValueRslt(lv.getDouble() + rv.getDouble());
				default:
					OmnAlarm << enderr;
					break;
			}
			break;

		case ArithOpr::eSub:
			switch (return_type)
			{
				case DataType::eU64:
					return ValueRslt(lv.getU64() - rv.getU64());
				case DataType::eInt64:
					return ValueRslt(lv.getI64() - rv.getI64());
				case DataType::eDouble:
					{
						//return ValueRslt(lv.getDouble() - rv.getDouble());
						int r_bits = lv.getDoubleBits();
						int l_bits = rv.getDoubleBits();
						int rslt_bits = max(r_bits, l_bits);
						ValueRslt value;
						value.setDouble(lv.getDouble() - rv.getDouble(), rslt_bits);
						return value;
					}
				case DataType::eNumber:
					return ValueRslt(lv.getDouble() - rv.getDouble());
				default:
					OmnAlarm << enderr;
					break;
			}
			break;

		case ArithOpr::eMul:
			switch (return_type)
			{
				case DataType::eU64:
					return ValueRslt(lv.getU64() * rv.getU64());
				case DataType::eInt64:
					return ValueRslt(lv.getI64() * rv.getI64());
				case DataType::eDouble:
					{
						//return ValueRslt(lv.getDouble() * rv.getDouble());
						int r_bits = lv.getDoubleBits();
						int l_bits = rv.getDoubleBits();
						if (r_bits < 0 || l_bits < 0)
						{
							return ValueRslt(lv.getDouble() * rv.getDouble());
						}
						int rslt_bits = r_bits + l_bits;
						ValueRslt value;
						value.setDouble(lv.getDouble() * rv.getDouble(), rslt_bits);
						return value;
					}
				case DataType::eNumber:
					return ValueRslt(lv.getDouble() * rv.getDouble());
				default:
					OmnAlarm << enderr;
					break;
			}
			break;

		case ArithOpr::eDiv:
			switch (return_type)
			{
				case DataType::eU64:
					return (!rv.getU64()) ? (vv) : (ValueRslt(lv.getU64() / rv.getU64()));
				case DataType::eInt64:
					return (!rv.getU64()) ? (vv) : (ValueRslt(lv.getI64() / rv.getI64()));
				case DataType::eDouble:
					return (abs(rv.getDouble()) <= 1e-15) ? (vv) : (ValueRslt(lv.getDouble() / rv.getDouble()));
				case DataType::eNumber:
					return (abs(rv.getDouble()) <= 1e-15) ? (vv) : ValueRslt(lv.getDouble() / rv.getDouble());
				default:
					OmnAlarm << enderr;
					break;
			}
			break;

		case ArithOpr::eMod:
			switch (return_type)
			{
				case DataType::eU64:
					return (!rv.getU64()) ? (vv) : (ValueRslt(lv.getU64() % rv.getU64()));
				case DataType::eInt64:
					return (!rv.getI64()) ? (vv) : (ValueRslt(lv.getI64() % rv.getI64()));
				default:
					OmnAlarm << enderr;
					break;
			}
			break;

		default:
			OmnAlarm << enderr;
			break;
	}

	OmnAlarm << enderr;
	return vv;
}


i64
ValueRslt::getInstanceCount()
{
	OmnScreen << "ValueRslt memory count: " << sgMemoryCount << endl;
	return sgInstanceCount;
}


i64
ValueRslt::getTotalInstances()
{
	return sgTotalInstances;
}


int
ValueRslt::getDoubleBits() const
{
	return mDoubleBits;
}


int
ValueRslt::getDataLen(const DataType::E data_type, const char *data)
{
	// This function determines the length of the value, which is dependent on data type.
	switch (data_type)
	{
		case DataType::eChar:
			return sizeof(char);
		case DataType::eBool:
			return sizeof(bool);
		case DataType::eInt64:
			return sizeof(i64);
		case DataType::eU64:
			return sizeof(u64);
		case DataType::eDouble:
			return sizeof(double);
		case DataType::eString:
			return *(int*)data + 4;
		default:
			OmnAlarm << "error type" << enderr;
			return false;
	}
	return false;
}


// Gavin, 2017/05/12
const char *
ValueRslt::getBinaryData() const
{
	return (char*)&mValue;
}

