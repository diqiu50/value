////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005
// Packet Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification is not permitted unless authorized in writing by a duly
// appointed officer of Packet Engineering, Inc. or its derivatives
//
// Description:
// This class simulates the records of the following format:
// 		ddd$ddd$ddd
// where '$' is a substring that serves as the separator that separates
// fields in the record. The separator is configurable.
//
// When data are set to this record, it separates the fields into mFields.
// There shall be an AosDataElem for each field. The AosDataElem is responsible
// for how to interpret the field.
//
// Modification History:
// 2013/12/27 Created by Chen Ding
////////////////////////////////////////////////////////////////////////////
#include "DataRecord/RecordCSV.h"

#include "Alarm/AlarmMacros.h"
#include "API/AosApi.h"
#include "Debug/Except.h"
#include "MetaData/MetaData.h"
#include "Rundata/Rundata.h"
#include "Util/ValueRslt.h"
#include "XmlUtil/XmlTag.h"
#include "Debug/Debug.h"
#include "API/AosApiC.h"


AosRecordCSV::AosRecordCSV(const bool flag)
:
AosDataRecord(AosDataRecordType::eCSV, AOSRECORDTYPE_CSV, flag),
mRecordLen(0),
mEstimateRecordLen(0),
mNoTextQualifier(false),
mNeedEscapeQualifier(0),
mBackslashEscape(true),
mCrtRcdLen(0),
mCrtFieldIdx(0),
mTrimCondition(eInvalid)
{
}


AosRecordCSV::AosRecordCSV(
		const AosRecordCSV &rhs,
		AosRundata *rdata)
:
AosDataRecord(rhs, rdata),
mRecordLen(rhs.mRecordLen),
mEstimateRecordLen(rhs.mEstimateRecordLen),
mRowDelimiter(rhs.mRowDelimiter),
mFieldDelimiter(rhs.mFieldDelimiter),
mTextQualifier(rhs.mTextQualifier),
mNoTextQualifier(rhs.mNoTextQualifier),
mNeedEscapeQualifier(0),
mBackslashEscape(rhs.mBackslashEscape),
mCrtRcdLen(0),
mCrtFieldIdx(0),
mRowDelimiterChar(rhs.mRowDelimiterChar),
mFieldsNum(rhs.mFieldsNum),
mTrimCondition(rhs.mTrimCondition)
{
	if (mNumFields > 0)
	{
		mNeedEscapeQualifier = OmnNew bool[mNumFields];
		memset(mNeedEscapeQualifier, 0, mNumFields);
	}
}


AosRecordCSV::~AosRecordCSV()
{
	delete [] mNeedEscapeQualifier;
	mNeedEscapeQualifier = 0;
}


bool
AosRecordCSV::config(
		const AosXmlTagPtr &def,
		AosRundata *rdata)
{
	//aos_assert_r(def, false);
	OmnString msg;
	if (!def)
	{
		aos_jql_msg(rdata, msg_id::e_config_is_null);
		OmnAlarm << msg << enderr;
		return false;
	}

	OmnScreen << def->toString() << endl;

	bool rslt = AosDataRecord::config(def, rdata);
	aos_assert_r(rslt, false);

	OmnString trim = def->getAttrStr("trim");
	if (trim == "ALL") mTrimCondition = eAll;
	if (trim == "HEAD") mTrimCondition = eHead;
	if (trim == "TAIL") mTrimCondition = eTail;
	if (trim == "NULL") mTrimCondition = eNull;

    OmnString tmp = def->getAttrStr(AOSTAG_ROW_DELIMITER);
   	tmp.toLower();
	if (tmp != "crlf" && tmp != "lf")
	{
		aos_jql_msg(rdata, msg_id::e_lines_config_error, def->getAttrStr(AOSTAG_ROW_DELIMITER).data());
		OmnAlarm << msg << enderr;
		return false;
	}
	OmnString rowDelimiter = AosParseRowDelimiter(tmp);
	if (rowDelimiter == "")
	{
		aos_jql_msg(rdata, msg_id::e_lines_terminated_null_string);
		OmnAlarm << msg << enderr;
		return false;
	}

	mRowDelimiter = rowDelimiter;

	mRowDelimiterChar = mRowDelimiter.data()[0];

	OmnString field_delimiter = AosParseFieldDelimiter(def->getAttrStr(AOSTAG_FIELD_DELIMITER));

	AosConvertAsciiBinary(field_delimiter);

	if (field_delimiter.length() <= 0)
	{
		aos_jql_msg(rdata, msg_id::e_fields_terminated_error, def->getAttrStr(AOSTAG_FIELD_DELIMITER).data());
		OmnAlarm << msg << enderr;
		return false;
	}
	mFieldDelimiter = field_delimiter;
	if (mFieldDelimiter.length() == 1 && !mFieldDelimiter.data()[0])
	{
		aos_jql_msg(rdata, msg_id::e_fields_terminated_error, def->getAttrStr(AOSTAG_FIELD_DELIMITER).data());
		OmnAlarm << msg << enderr;
		return false;
	}

	mTextQualifier = 0;
	mBackslashEscape = false;
	mNoTextQualifier = def->getAttrBool("no_text_qualifier", false);
	if (!mNoTextQualifier)
	{
	    OmnString tmp = def->getAttrStr(AOSTAG_TEXT_QUALIFIER);
    	tmp.toLower();
		if (tmp != "dqm" && tmp != "sqm" && tmp != "null")
		{
			aos_jql_msg(rdata, msg_id::e_field_enclosed_error, def->getAttrStr(AOSTAG_TEXT_QUALIFIER).data());
			OmnAlarm << msg << enderr;
			return false;
		}
		OmnString text_qualify_str = AosParseTextQualifier(tmp);
		if (text_qualify_str != "")
		{
			mTextQualifier = text_qualify_str[0];
		}
		if (mTextQualifier == '\1')
		{
			mNoTextQualifier = true;
		}

		OmnString ss = def->getAttrStr("quote_escape");
		if (ss == "backslash") mBackslashEscape = true;
	}

	if (mNumFields <= 0)
	{
		aos_jql_msg(rdata, msg_id::e_schema_missing_field);
		OmnAlarm << msg << enderr;
		return false;
	}

	int field_num = mNumFields;
	mEstimateRecordLen = 0;
	for (u32 i=0; i<mNumFields; i++)
	{
		if (!mFieldsRaw[i]->needValueFromField())
		{
			int field_len = mFieldsRaw[i]->mFieldInfo.field_len;
			if (field_len < eEstimateEachFieldLen)
			{
				field_len = eEstimateEachFieldLen;
			}
			mEstimateRecordLen += field_len;
		}
		if (mFieldsRaw[i]->isVirtualField())
		{
			field_num --;
		}
	}
	mFieldsNum = field_num;

	if (mNeedEscapeQualifier)
	{
		AosSetError(rdata, "internal_error") << enderr;
		delete [] mNeedEscapeQualifier;
		mNeedEscapeQualifier = 0;
	}

	/*
	for (u32 i=0; i<mNumFields; i++)
	{
		mFieldsRaw[i]->setFixedFlag(true);
	}
	*/

	mNeedEscapeQualifier = OmnNew bool[mNumFields];
	memset(mNeedEscapeQualifier, 0, mNumFields);

	return true;
}


AosDataRecordObjPtr
AosRecordCSV::clone(AosRundata *rdata) const
{
	try
	{
		return OmnNew AosRecordCSV(*this, rdata);
	}

	catch (...)
	{
		OmnAlarm << "Failed creating object" << enderr;
		return nullptr;
	}
}


AosJimoPtr
AosRecordCSV::cloneJimo() const
{
	try
	{
		return OmnNew AosRecordCSV(*this);
	}

	catch (...)
	{
		OmnAlarm << "Failed creating object" << enderr;
		return nullptr;
	}
}


AosDataRecordObjPtr
AosRecordCSV::create(
		const AosXmlTagPtr &def,
		const u64 task_docid,
		AosRundata *rdata) const
{
	AosRecordCSV * record = OmnNew AosRecordCSV(false);
	record->setTaskDocid(task_docid);
	bool rslt = record->config(def, rdata);
	aos_assert_r(rslt, nullptr);
	return record;
}


bool
AosRecordCSV::getFieldValue(
		const int idx,
		AosValueRslt &value,
		const bool copy_flag,
		AosRundata* rdata,
		AosQuery::QueryContextObj *context)
{
	//OmnTagFuncInfo << endl;
	aos_assert_r(idx >= 0 && (u32)idx < mNumFields, false);
	aos_assert_r(mFieldValues, false);

	if (mFieldValFlags[idx])
	{
		value = mFieldValues[idx];
		return true;
	}

	bool rslt = true;
	int index = 0;
	/*
	if (mFieldsRaw[idx]->isConst())
	{
		rslt = mFieldsRaw[idx]->getValueFromRecord(
			this, mMemory, mMemLen, index, value, copy_flag, rdata);
		aos_assert_r(rslt, false);

		mFieldValFlags[idx] = true;
		mFieldValues[idx] = value;
		return true;
	}
	*/

	index = 0;
	rslt = mFieldsRaw[idx]->getValueFromRecord(
		this, mMemory, mMemLen, index, value, true, rdata, context);
	aos_assert_r(rslt, false);

	mFieldValues[idx] = value;
	mFieldValFlags[idx] = true;

	if (mNeedEscapeQualifier[idx])
	{
		OmnString str = mFieldValues[idx].getStr();
		aos_assert_r(str.length() > 2, false);

		char c = str[0];
		OmnString str_temp(&str.data()[1], str.length() - 2);
		OmnString new_str(&c, 1);
		str.setLength(2);
		str.getBuffer()[0] = mBackslashEscape ? '\\' : c;
		str.getBuffer()[1] = c;
		str_temp.replace(str, new_str, true);
		mFieldValues[idx].setStr(str_temp);
		value = mFieldValues[idx];
		mNeedEscapeQualifier[idx] = false;
	}

	return true;
}


bool
AosRecordCSV::setFieldValue(
		const int idx,
		AosValueRslt &value,
		bool &outofmem,
		AosRundata* rdata)
{
	if(mCache) return AosDataRecord::setFieldValue(idx, value);
	if(mFields[idx]->isVirtualField())
	{
		return true;
	}
	aos_assert_r(idx == mCrtFieldIdx, false);

	outofmem = false;
	aos_assert_r(idx >= 0 && idx < mFieldsNum, false);
	aos_assert_r(mMemory && mMemLen > 0, false);
	int offset = mCrtRcdLen;

	if (idx != 0)
	{
		if (offset + 1 >= mMemLen)
		{
			outofmem = true;
			return true;
		}
		if (mFieldDelimiter.length() == 1)
		{
			mMemory[offset] = mFieldDelimiter.data()[0];
			offset++;
		}
		else
		{
			memcpy(&mMemory[offset], mFieldDelimiter.data(), mFieldDelimiter.length());
			offset += mFieldDelimiter.length();
		}
	}

	if (!value.isNull() && mTrimCondition != eInvalid && mFieldsRaw[idx]->getType() == AosDataFieldType::eString)
	{
		OmnString str = value.getStr();
		if (mTrimCondition == eAll)
		{
			str.removeWhiteSpaces();
			value.setStr(str);
		}
		else if (mTrimCondition == eHead)
		{
			str.removeLeadingWhiteSpace();
			value.setStr(str);
		}
		else if (mTrimCondition == eTail)
		{
			str.removeTailWhiteSpace();
			value.setStr(str);
		}
		else if (mTrimCondition == eNull)
		{
			str.removeTailWhiteSpace();
			if (str == "")
			{
				value.setNull();
			}
			else
			{
				value.setStr(str);
			}
		}
	}

	AosValueRslt new_value = value;
	AosDataRecord::setFieldValue(idx, value, outofmem, rdata);
	if(!value.isNull())
	{
		AosDataType::E fieldDataType = mFields[idx]->getDataType(rdata, this);
		if (fieldDataType == AosDataType::eString)
		{
			// 1. Text Qualifier
			if (!mNoTextQualifier && mTextQualifier)
			{
				if (offset + 1 >= mMemLen)
				{
					outofmem = true;
					return true;
				}
				mMemory[offset] = mTextQualifier;
				offset++;
			}

			// 2. escape
			OmnString str = value.getStr();
			if (str != "" && str.indexOf(0, mTextQualifier) != -1)
			{
				OmnString pattern(&mTextQualifier, 1);
				OmnString new_str(2, mTextQualifier, true);
				if (mBackslashEscape) new_str.getBuffer()[0] = '\\';
				str.replace(pattern, new_str, true);
			}
			new_value.setStr(str);
		}

		mFields[idx]->mFieldInfo.field_offset = offset;
		mFields[idx]->setValueToRecord(mMemory, mMemLen, new_value, outofmem, rdata);
		if (outofmem)
		{
			return true;
		}
		offset += mFields[idx]->mFieldInfo.field_len;
		if (fieldDataType == AosDataType::eString)
		{
			if (!mNoTextQualifier && mTextQualifier)
			{
				if (offset + 1 >= mMemLen)
				{
					outofmem = true;
					return true;
				}
				mMemory[offset] = mTextQualifier;
				offset++;
			}
		}
	}

	//if (idx == (i64)(mNumFields - 1) && mRowDelimiter != "")
	if (idx == (i64)(mFieldsNum - 1) && mRowDelimiter != "")
	{
		if (offset + mRowDelimiter.length() >= mMemLen)
		{
			outofmem = true;
			return true;
		}
		memcpy(&mMemory[offset], mRowDelimiter.data(), mRowDelimiter.length());
		offset += mRowDelimiter.length();
		mRecordLen = offset;
	}
	mCrtRcdLen = offset;
	mCrtFieldIdx++;
	return true;
}


int
AosRecordCSV::getRecordLen()
{
	return mRecordLen;
}


char *
AosRecordCSV::getData(AosRundata *rdata)
{
	return mMemory;
}


void
AosRecordCSV::clear()
{
	AosDataRecord::clear();

	mIsLastRecordMissRowDelimiter = false;
	mIsEOF = false;
	mCrtRcdLen = 0;
	mRecordLen = 0;
	mCrtFieldIdx = 0;
	memset(mFieldValFlags, 0, mNumFields);
	memset(mNeedEscapeQualifier, 0, mNumFields);
	for (u32 i = 0; i < mNumFields; i++)
	{
		mFieldsRaw[i]->setNotNull();
	}
}


bool
AosRecordCSV::setData(
		char *data,
		const int len,
		AosMetaData *metaData,
		int &status)
		//const int64_t offset)
{
	aos_assert_r(data && len > 0, false);
	int rcd_len = 0;
	if (mFieldDelimiter.length() == 1)
	{
		parseData(data, len, rcd_len, status);
	}
	else
	{
		parseData2(data, len, rcd_len, status);
	}
	mMemory = data;
	mMemLen = len;
	mMetaData = metaData;
	mMetaDataRaw = metaData;
	//mOffset = offset;

	mRecordLen = rcd_len;
	//barry JIMODB-1532
	if (metaData && status == 0)
	{
		metaData->setRecordLength(rcd_len);
	}

	return true;
}


bool
AosRecordCSV::parseData(
		char *data,
		const int64_t &len,
		int &record_len,
		int &status)
{
	// this function is to scan all the data until find one record
	// eg: there are three fields, the following is error case
	// 1. abc, bcd\n
	// 2. abc, bcd, def, hij\n
	// 3. abc, b"\n"d, def\n
	// 4. abc, "abc"d, def\n
	//
	// the following is correct case:
	// 1. abc, bcd, def\n
	// 2. abc, "bcd\n", def\n
	//
	// if field has '\000', it must replace to ' '
	// eg : abc\000, bcd ===> abc , bcd


	// jimodb-953, 2015.10.14
	// Optimize RecordCSV
	int crtidx = 0;
	int numFields = 0;
	int field_idx = 0;
	int pre_field_pos = 0, field_end = 0;
	int crt_status = eFieldBegin;
	mFieldsRaw[field_idx]->setNotNull();
	int rcd_len = mRowDelimiter.length();
	bool needEscapeQualifier = false;
	char field_delimiter = mFieldDelimiter.data()[0];

	while(crtidx<len)
	{
		if (data[crtidx] == 0) data[crtidx]= ' ';

		switch(crt_status)
		{
			case eFieldBegin:
				if (data[crtidx] == field_delimiter)
				{
					crt_status = eFieldEnd;
					mFieldsRaw[field_idx]->setNull();
					break;
				}

				if (data[crtidx] == mRowDelimiterChar || '\r' == data[crtidx])
				{
					crt_status = eRecordEnd;
					mFieldsRaw[field_idx]->setNull();
					break;
				}
				if (!mNoTextQualifier && data[crtidx] == mTextQualifier)
				{
					crt_status = eStrField;
				}

				else
				{
					crt_status = eField;
				}
				crtidx++;
				break;

			case eField:
				if (data[crtidx] == field_delimiter)
				{
					crt_status = eFieldEnd;
					break;
				}

				if (data[crtidx] == mRowDelimiterChar || '\r' == data[crtidx])
				{
					crt_status = eRecordEnd;
					break;
				}

				crtidx++;
				break;

			case eStrField:
				if (data[crtidx] == mTextQualifier)
				{
					crtidx++;
					if (crtidx>=len)
					{
						record_len = -1;
						status = -1;
						return true;
					}
					if (data[crtidx] != mTextQualifier)
					{
						if (data[crtidx] == field_delimiter)
						{
							crt_status = eFieldEnd;
							if (crtidx-pre_field_pos == 2)
							{
								mFieldsRaw[field_idx]->setNull();
							}
							break;
						}

						else if (mRowDelimiterChar == data[crtidx] || '\r' == data[crtidx])
						{
							crt_status = eRecordEnd;
							if(crtidx-pre_field_pos == 2)
							{
								mFieldsRaw[field_idx]->setNull();
							}
							break;
						}
						else
						{
							OmnScreen << "invalid record" << endl;
							status =-2;
							record_len = ++crtidx;
							return true;
						}
					}
					else if (!mBackslashEscape)
					{
						// It is escaped
						needEscapeQualifier = true;
						crtidx++;
						continue;
					}
				}
				crtidx++;
				break;

			case eFieldEnd:
				crt_status = eFieldBegin;
				field_end = crtidx;
				if (!mNoTextQualifier && mTextQualifier && data[pre_field_pos] == mTextQualifier)
				{
					if (needEscapeQualifier)
					{
						mNeedEscapeQualifier[field_idx] = true;
						field_end++;
						pre_field_pos--;
					}
					mFieldsRaw[field_idx]->mFieldInfo.field_offset = pre_field_pos + 1;
					mFieldsRaw[field_idx]->mFieldInfo.field_data_len = field_end - pre_field_pos - 2;
				}
				else
				{
					mFieldsRaw[field_idx]->mFieldInfo.field_offset = pre_field_pos;
					mFieldsRaw[field_idx]->mFieldInfo.field_data_len = field_end - pre_field_pos;
				}

				crtidx++;
				field_idx ++;
				if (field_idx >= mFieldsNum || !isValidField(data, field_idx-1))
				{
					OmnScreen << "invalid record" << endl;
					status = -2;
					record_len = crtidx;
					return true;
				}
				numFields++;
				pre_field_pos = crtidx;
				needEscapeQualifier = false;
				break;

			case eRecordEnd:
				numFields++;
				if (data[crtidx] == '\r')
				{
					crtidx++;
					if (crtidx < len)
					{
						if (data[crtidx] != '\n')
						{
							OmnScreen << "invalid record" << endl;
							status = -2;
							record_len = ++crtidx;
							return true;
						}
						rcd_len = 2;
					}
					else
					{
						status = -1;
						record_len = -1;
						return true;
					}
				}
				if (numFields == mFieldsNum)
				{
					field_end = crtidx;
					if (!mNoTextQualifier && mTextQualifier && data[pre_field_pos] == mTextQualifier)
					{
						if (needEscapeQualifier)
						{
							mNeedEscapeQualifier[field_idx] = true;
							field_end++;
							pre_field_pos--;
						}
						mFieldsRaw[field_idx]->mFieldInfo.field_offset = pre_field_pos+1;
						mFieldsRaw[field_idx]->mFieldInfo.field_data_len = field_end - pre_field_pos -rcd_len -1;
					}
					else
					{
						mFieldsRaw[field_idx]->mFieldInfo.field_offset = pre_field_pos;
						mFieldsRaw[field_idx]->mFieldInfo.field_data_len= field_end - pre_field_pos - rcd_len +1;
					}
					if (!isValidField(data, field_idx))
					{
						OmnScreen << "invalid record" << endl;
						status = -2;
						record_len = crtidx+1;
						return true;
					}
					crtidx++;
					status = 0;
					record_len = crtidx;
					crtidx = len;
					needEscapeQualifier = false;
					return true;
				}
				else
				{
					OmnString s = "invalid record:\n";
					if (len < 2048)
						s << OmnString(data, len);
					else
						s << OmnString(data, 2048);
					OmnScreen << s << endl;
					status = -2;
					record_len = ++crtidx;
					return true;
				}

			default:
				OmnScreen << "invalid record" << endl;
				status = -2;
				record_len = ++crtidx;
				return true;
		};
	}

	if (crtidx >= len)
	{
		if (crtidx == len && numFields+1 == mFieldsNum && mIsEOF)
		{
			//this means this is the last line of file, allow to missing mRowDelimiterChar
			record_len = crtidx;
			status = 0;
			mIsLastRecordMissRowDelimiter = true;
			return true;
		}
		record_len = -1;
		status = -1;
		return true;
	}
	return true;
}

bool
AosRecordCSV::parseData2(
		char *data,
		const int64_t &len,
		int &record_len,
		int &status)
{
	// this function is to scan all the data until find one record
	// eg: there are three fields, the following is error case
	// 1. abc, bcd\n
	// 2. abc, bcd, def, hij\n
	// 3. abc, b"\n"d, def\n
	// 4. abc, "abc"d, def\n
	//
	// the following is correct case:
	// 1. abc, bcd, def\n
	// 2. abc, "bcd\n", def\n
	//
	// if field has '\000', it must replace to ' '
	// eg : abc\000, bcd ===> abc , bcd


	// jimodb-953, 2015.10.14
	// Optimize RecordCSV2
	int crtidx = 0;
	int numFields = 0;
	int field_idx = 0;
	int pre_field_pos = 0, field_end = 0;
	int crt_status = eFieldBegin;
	mFieldsRaw[field_idx]->setNotNull();
	int rcd_len = mRowDelimiter.length();
	int FDLen = mFieldDelimiter.length();
	bool needEscapeQualifier = false;

	while(crtidx<len)
	{
		if (data[crtidx] == 0) data[crtidx]= ' ';

		switch(crt_status)
		{
			case eFieldBegin:
				if (crtidx + 1 >= FDLen && !memcmp(&data[crtidx-FDLen+1], mFieldDelimiter.data(), FDLen))
				{
					crt_status = eFieldEnd;
					mFieldsRaw[field_idx]->setNull();
					break;
				}

				if (data[crtidx] == mRowDelimiterChar || '\r' == data[crtidx])
				{
					crt_status = eRecordEnd;
					mFieldsRaw[field_idx]->setNull();
					break;
				}
				if (!mNoTextQualifier && data[crtidx] == mTextQualifier)
				{
					crt_status = eStrField;
				}

				else
				{
					crt_status = eField;
				}
				crtidx++;
				break;

			case eField:
				if (crtidx + 1 >= FDLen && !memcmp(&data[crtidx-FDLen+1], mFieldDelimiter.data(), FDLen))
				{
					crt_status = eFieldEnd;
					if (pre_field_pos + FDLen == crtidx + 1)
						mFieldsRaw[field_idx]->setNull();
					break;
				}

				if (data[crtidx] == mRowDelimiterChar || '\r' == data[crtidx])
				{
					crt_status = eRecordEnd;
					break;
				}

				crtidx++;
				break;

			case eStrField:
				if (data[crtidx] == mTextQualifier)
				{
					crtidx++;
					if (crtidx >= len)
					{
						record_len = -1;
						status = -1;
						return true;
					}
					if (data[crtidx] != mTextQualifier)
					{
						if (crtidx + 1 >= FDLen && !memcmp(&data[crtidx-FDLen+1], mFieldDelimiter.data(), FDLen))
						{
							crt_status = eFieldEnd;
							if (crtidx - pre_field_pos == 2 * FDLen)
							{
								mFieldsRaw[field_idx]->setNull();
							}
							break;
						}

						else if (mRowDelimiterChar == data[crtidx] || '\r' == data[crtidx])
						{
							crt_status = eRecordEnd;
							if(crtidx - pre_field_pos == 2 * FDLen)
							{
								mFieldsRaw[field_idx]->setNull();
							}
							break;
						}
						else
						{
							OmnScreen << "invalid record" << endl;
							status =-2;
							record_len = ++crtidx;
							return true;
						}
					}
					else if (!mBackslashEscape)
					{
						// It is escaped
						needEscapeQualifier = true;
						crtidx++;
						continue;
					}
				}
				crtidx++;
				break;

			case eFieldEnd:
				crt_status = eFieldBegin;
				crtidx++;
				if (crtidx < pre_field_pos + FDLen)
				{
					// this case ,,,,,,(fielddeilemiter = ",,")
					break;
				}

				field_end = crtidx - FDLen;
				if (!mNoTextQualifier && mTextQualifier && data[pre_field_pos] == mTextQualifier)
				{
					if (needEscapeQualifier)
					{
						mNeedEscapeQualifier[field_idx] = true;
						field_end++;
						pre_field_pos--;
					}
					mFieldsRaw[field_idx]->mFieldInfo.field_offset = pre_field_pos + sizeof(mTextQualifier);
					mFieldsRaw[field_idx]->mFieldInfo.field_data_len = field_end - pre_field_pos - sizeof(mTextQualifier) * 2;
				}

				else
				{
					mFieldsRaw[field_idx]->mFieldInfo.field_offset = pre_field_pos;
					mFieldsRaw[field_idx]->mFieldInfo.field_data_len = field_end - pre_field_pos;
				}

				field_idx++;
				if (field_idx >= mFieldsNum || !isValidField(data, field_idx-1))
				{
					OmnScreen << "invalid record" << endl;
					status = -2;
					record_len = crtidx;
					return true;
				}
				numFields++;
				pre_field_pos = crtidx;
				needEscapeQualifier = false;
				break;

			case eRecordEnd:
				numFields++;
				if (data[crtidx] == '\r')
				{
					if (crtidx + 1 < len)
					{
						if (data[crtidx + 1] != '\n')
						{
							OmnScreen << "invalid record" << endl;
							status = -2;
							crtidx += 2;
							record_len = crtidx;
							return true;
						}
						rcd_len = 2;
					}
					else
					{
						status = -1;
						record_len = -1;
						return true;
					}
				}
				if (numFields == mFieldsNum)
				{
					field_end = crtidx;
					if (!mNoTextQualifier && mTextQualifier && data[pre_field_pos] == mTextQualifier)
					{
						if (needEscapeQualifier)
						{
							mNeedEscapeQualifier[field_idx] = true;
							field_end++;
							pre_field_pos--;
						}
						mFieldsRaw[field_idx]->mFieldInfo.field_offset = pre_field_pos + sizeof(mTextQualifier);
						mFieldsRaw[field_idx]->mFieldInfo.field_data_len = field_end - pre_field_pos - sizeof(mTextQualifier);
					}
					else
					{
						mFieldsRaw[field_idx]->mFieldInfo.field_offset = pre_field_pos;
						mFieldsRaw[field_idx]->mFieldInfo.field_data_len = field_end - pre_field_pos;
					}
					if(!isValidField(data, field_idx))
					{
						OmnScreen << "invalid record" << endl;
						status = -2;
						crtidx += rcd_len;
						record_len = crtidx;
						return true;
					}
					crtidx += rcd_len;
					status = 0;
					record_len = crtidx;
					crtidx = len;
					needEscapeQualifier = false;
					return true;
				}
				else
				{
					OmnString s = "invalid record:\n";
					if (len < 2048)
						s << OmnString(data, len);
					else
						s << OmnString(data, 2048);
					OmnScreen << s << endl;
					status = -2;
					crtidx += rcd_len;
					record_len = crtidx;
					return true;
				}

			default:
				status = -2;
				crtidx += rcd_len;
				record_len = crtidx;
				return true;
		};
	}

	if (crtidx >= len)
	{
		if (crtidx == len && numFields+1 == mFieldsNum && mIsEOF)
		{
			//this means this is the last line of file, allow to missing mRowDelimiterChar
			record_len = crtidx;
			status = 0;
			mIsLastRecordMissRowDelimiter = true;
			return true;
		}
		record_len = -1;
		status = -1;
		return true;
	}
	return true;
}


AosXmlTagPtr
AosRecordCSV::serializeToXmlDoc(
		const char *data,
		const int data_len,
		AosRundata* rdata)
{
	int status;
	bool rslt = setData((char*)data, data_len, 0, status);
	aos_assert_r(rslt, nullptr);

	OmnString docstr = "<record>";

	OmnString subtagname, valuestr;
	for (u32 i=0; i<mNumFields; i++)
	{
		if (mFieldsRaw[i]->isIgnoreSerialize()) continue;

		subtagname = mFieldsRaw[i]->getName();
		aos_assert_r(subtagname != "", nullptr);

		AosValueRslt value;
		rslt = getFieldValue(i, value, false, rdata);
		aos_assert_r(rslt, nullptr);

		valuestr = value.getStr();
		valuestr.normalizeWhiteSpace(true, true);

		docstr << "<" << subtagname
			   << "><![CDATA[" << valuestr
			   << "]]></" << subtagname << ">";
	}

	docstr << "</record>";

	AosXmlTagPtr doc = AosXmlParser::parse(docstr AosMemoryCheckerArgs);
	aos_assert_r(doc, nullptr);

	return doc;
}


void
AosRecordCSV::flush(const bool clean_memory)
{
	aos_assert(mCrtFieldIdx == (int)mFieldsNum);
	//aos_assert(mCrtFieldIdx == (int)mNumFields);

}


bool
AosRecordCSV::appendField(
		AosRundata *rdata,
		const OmnString &name,
		const AosDataType::E type,
		const AosStrValueInfo &info)
{
	OmnString datatype = AosDataType::getTypeStr(type);
	if (datatype == "string") datatype = "str";
	if (name == "docid") datatype = "docid";

	OmnString str = "<datafield zky_name=\"";
	str << name << "\" type=\"" << datatype << "\"/>";

	AosXmlTagPtr xml = AosStr2Xml(str AosMemoryCheckerArgs);
	aos_assert_r(xml, false);

	AosDataFieldObjPtr field = AosDataFieldObj::createDataFieldStatic(xml, this, rdata);
	aos_assert_r(field, false);

	bool rslt = AosDataRecord::appendField(rdata, field);
	aos_assert_r(rslt, false);

	OmnDelete [] mNeedEscapeQualifier;
	bool *needEscapeQualifier = OmnNew bool[mNumFields];
	memset(needEscapeQualifier, 0, mNumFields);
	mNeedEscapeQualifier = needEscapeQualifier;

	return true;
}


bool
AosRecordCSV::appendField(
		AosRundata *rdata,
		const AosDataFieldObjPtr &field)
{
	aos_assert_r(field, false);

	bool rslt = AosDataRecord::appendField(rdata, field);
	aos_assert_r(rslt, false);

	OmnDelete [] mNeedEscapeQualifier;
	bool *needEscapeQualifier = OmnNew bool[mNumFields];
	memset(needEscapeQualifier, 0, mNumFields);
	mNeedEscapeQualifier = needEscapeQualifier;

	return true;
}


bool
AosRecordCSV::removeFields()
{
	AosDataRecord::removeFields();

	delete [] mNeedEscapeQualifier;
	mNeedEscapeQualifier = 0;

	return true;
}

bool
AosRecordCSV::isValidField(
		const char * data,
		const int index)
{
	return true;
	if (mFieldsRaw[index]->getType() == AosDataFieldType::eDateTime)
	{
		if (mFieldsRaw[index]->isValidField(data))
		{
			return true;
		}
		OmnScreen << "invalid field record_name:" << mName << endl;
		return false;
	}
	return true;
}
