////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005
// Packet Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification is not permitted unless authorized in writing by a duly
// appointed officer of Packet Engineering, Inc. or its derivatives
//
// File Name: Alarm.h
// Description:
//   
//
// Modification History:
// 
////////////////////////////////////////////////////////////////////////////
#ifndef Omn_Alarm_AlarmEntry_h
#define Omn_Alarm_AlarmEntry_h

#include "Debug/ErrId.h"
#include "TraceMgr/TraceMacros.h"
#include "Util/String.h"


#ifndef enderr
#define enderr OmnEndError()
#endif

class OmnEndError
{
};


class OmnAlarmEntry
{
public:
	enum
	{
		eFile,
		eLine,
		eAlarmCat,
		eAlarmId,
		eAlarmSeqno,
		eEntityId,
		eThreadId,
		eTime,
		eErrMsg,

		eMaxFileNameLength = 200,
		eTimeStrLength = 101,
		eErrMsgMaxLength = 1010
	};

private:
	OmnString			mFile;
	int					mLine = 0;
	OmnErrId::E			mErrorId = OmnErrId::eUnknown;
	int					mAlarmSeqno;
	OmnString			mTime;
	OmnString			mErrMsg;
	OmnString			mModuleName;

	bool				mNeedToRaise = true;

public:
	OmnAlarmEntry()	{}
	~OmnAlarmEntry() {}

	OmnAlarmEntry & operator << (const OmnEndError er);
	OmnAlarmEntry & operator << (const OmnString &errMsg);
	OmnAlarmEntry & operator << (const std::string &errMsg);
	OmnAlarmEntry & operator << (const char *errmsg);
	OmnAlarmEntry & operator << (const u8 *msg);
	OmnAlarmEntry & operator << (const int value);
	OmnAlarmEntry & operator << (const u8 value);
	OmnAlarmEntry & operator << (const u32 value);
	OmnAlarmEntry & operator << (void *ptr);
	OmnAlarmEntry & operator << (const u64 value);
	OmnAlarmEntry & operator << (const int64_t value);
	OmnAlarmEntry & operator << (const double value);

	OmnErrId::E		getErrId() const {return mErrorId;} 
	void			setErrId(const OmnErrId::E id) {mErrorId = id;}

	void			setFile(const OmnString file) {mFile = file;}
	OmnString		getFilename() const {return mFile;}

	void			setLine(const int line) {mLine = line;}
	int				getLine() const {return mLine;}

	void			setSeqno(const int sno) {mAlarmSeqno = sno;}
	int				getSeqno() const {return mAlarmSeqno;}

	void			setModuleName(const OmnString &name) {mModuleName = name;}
	void			setTime(const OmnString &time) {mTime = time;}

	void			setErrMsg(const OmnString &msg) {mErrMsg = msg;}
	OmnString		getErrMsg() const {return mErrMsg;}

	bool			toString(const int nn, char *data, const int length) const;
	bool			toString_Alarm(const int nn, char *data, const int length) const;
	bool			toString_SynErr(const int nn, char *data, const int length) const;


	static bool	addChar(char *to, 
					const int length, 
					int &pos,
					const char * const from)
	{
		int fLen = strlen(from);
		if (pos + fLen >= length)
		{
			return false;
		}

		strncpy(&to[pos], from, fLen);
		pos += fLen;
		to[pos] = 0;
		return true;
	}

	static bool addInt(char *to, 
				const int length,
				int &pos, 
				const int value)
	{
		char buff[30];
		sprintf(buff, "%d", value);
		int len = strlen(buff);
		if (pos + len >= length)
		{
			return false;
		}
		strncpy(&to[pos], buff, len);
		pos += len;
		to[pos] = 0;
		return true;
	}

	static bool addIntHex(char *to, 
				const int length,
				int &pos, 
				const int value)
	{
		char buff[30];
		sprintf(buff, "0x%x", value);
		int len = strlen(buff);
		if (pos + len >= length)
		{
			return false;
		}
		strncpy(&to[pos], buff, len);
		pos += len;
		to[pos] = 0;
		return true;
	}

};

#endif

