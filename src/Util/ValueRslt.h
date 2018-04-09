////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////
#ifndef Util_ValueRslt_h
#define Util_ValueRslt_h

namespace value
{
class ValueRslt
{
private:
	enum E
	{
		eDefaultSize = 24,
		eSPrintfSize = 50
	};

	union DataValue
	{
		int					char_value;
		bool 				bool_value;
		i64 				i64_value;
		u64 				u64_value;
		double 				double_value;
		char				str_value[eDefaultSize];
		void				*obj_value;
	};

private:
	DataValue			mValue;
	DataType::E			mType;

public:
	ValueRslt();

	ValueRslt(const ValueRslt &rhs);
	explicit ValueRslt(const bool value);
	explicit ValueRslt(const u64 value);
	explicit ValueRslt(const i64 value);
	explicit ValueRslt(const double value);
	explicit ValueRslt(const string &value);
	explicit ValueRslt(const char value);

	~ValueRslt();

	void reset();
	DataType::E getType() const;

	void setNull() {deleteMemory(); mType = DataType::eNull;}
	bool isNull() const {return mType==DataType::eNull;}
	static inline string getNullStr() {return "NULL";}

	// Set Value
	void setDouble(const double vv);
	void setU64(const u64 vv);
	void setI64(const i64 vv);
	void setStr(const string &vv);
	void setBool(const bool vv);
	void setChar(const char vv);

	double 				getDouble() const;
	u64 				getU64() const;
	i64 				getI64() const;
	string 				getStr() const;
	bool				getBool() const;
	char				getChar() const;
	const char* const 	getCStr(int &len) const;
	const char* 		getCStrValue(int &len) const;

	ValueRslt & operator = (const ValueRslt &rhs);
	bool operator < (const ValueRslt &rhs) const ;
	bool operator <= (const ValueRslt &rhs) const ;
	bool operator > (const ValueRslt &rhs) const ;
	bool operator >= (const ValueRslt &rhs) const ;
	bool operator == (const ValueRslt &rhs) const ;

	static ValueRslt doArith(
			const ArithOpr::E opr,
			const DataType::E return_type,
			const ValueRslt &lv,
			const ValueRslt &rv);

	static bool doComparison(
			const Opr opr,
			const DataType::E value_type,
			const ValueRslt &lv,
			const ValueRslt &rv) ;
private:
	string	setStringValue();
	string 	getStrValue();
	void 		deleteMemory();

};
};

#endif

