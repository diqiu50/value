////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005
// Packet Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification is not permitted unless authorized in writing by a duly
// appointed officer of Packet Engineering, Inc. or its derivatives
//
// File Name: Alarm.cpp
// Description:
//
//
// Modification History:
////////////////////////////////////////////////////////////////////////////
#include "Alarm/AlarmEntry.h"

#include "Alarm/AlarmMgr.h"
#include "AppMgr/App.h"
#include "Debug/Debug.h"
#include "Porting/ThreadDef.h"
#include "Porting/Sleep.h"
#include "Util1/Time.h"

#include <execinfo.h>


OmnAlarmEntry &
OmnAlarmEntry::operator << (const OmnEndError)
{
	OmnAlarmMgr::closeEntry(*this);
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const OmnString &errMsg)
{
	mErrMsg += errMsg;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const std::string &errMsg)
{
	mErrMsg += errMsg;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const char *errmsg)
{
	mErrMsg += errmsg;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const u8 *msg)
{
	return *this << (const char *)msg;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const int value)
{
	mErrMsg << value;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const u8 value)
{
	mErrMsg << value;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const u32 value)
{
	mErrMsg << value;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const u64 value)
{
	mErrMsg << value;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const int64_t value)
{
	mErrMsg << value;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (const double value)
{
	mErrMsg << value;
	return *this;
}


OmnAlarmEntry &
OmnAlarmEntry::operator << (void *ptr)
{
	char buf[100];
	sprintf(buf, "%ld", (unsigned long)ptr);
	mErrMsg += buf;
	return *this;
}


bool
OmnAlarmEntry::toString(
		const int num_alarms,
		char *data,
		const int length) const
{
	if (mErrorId == OmnErrId::eSynErr)
	{
		return toString_SynErr(num_alarms, data, length);
	}
	return toString_Alarm(num_alarms, data, length);
}


bool
OmnAlarmEntry::toString_Alarm(
		const int num_alarms,
		char *data,
		const int length) const
{
	//
	// Write the alarm in the following form:
	//
	//	"\n\n<"
	//	<< OmnTime::getSecTick()
	//	<< "> ******* Alarm Entry **********"
	//	"\nFile:              " << mFile <<
	//	"\nLine:              " << mLine <<
	//  "\nAlarm ID:          " << mErrorId <<
	//	"\nSeqno:             " << mAlarmSeqno <<
	//	"\nTrigger Thread ID: " << OmnGetCurrentThreadId() <<
	//	"\nTrigger Time:      " << mTime <<
	//	"\nError Message:     " << mErrMsg <<
	//	"\n*************************\n";
	//
	// Note that we did not use OmnString in order not to generate any alarms
	// while converting this entry into a string.
	//

	OmnString addrs;
	if (!OmnAlarmMgr::isPrintFullBacktrace())
	{
		void *buff[100];
		int sizes = backtrace(buff, 100);
		char addrStr[100];
		for(int i=0; i<sizes; i++)
		{
			sprintf(addrStr, "0x%lx", (u64)buff[i]);
			if (i == sizes - 1)
				addrs << addrStr << "\n";
			else
				addrs << addrStr << " ";
		}
	}
	else
	{
		void *buff[100];
		int sizes = backtrace(buff, 100);

		std::vector<string> tmps;
		OmnAlarmMgr::getBacktrace(tmps);
		if (sizes + 1 != tmps.size() || sizes <= 3) return false;

		char addrStr[100];
		for (size_t i=3; i<tmps.size(); i++)
		{
			sprintf(addrStr, "0x%lx", (u64)buff[i-1]);
			addrs << "[" << (i - 3) << "] "
				  << "[" << addrStr << "] "
				  << tmps[i] << "\n";
		}
	}
	addrs << "--------------------";

	int index = 0;
	if (!addChar(data, length, index, "\n<")) return false;
	if (!addInt(data, length, index, OmnTime::getSecTick())) return false;

	OmnString aa = "> ***** Alarm Entry ";
	aa << mFile << ":" << mLine << ":" << num_alarms << " ******";
	if (!addChar(data, length, index, aa.data())) return false;

	if (!addChar(data, length, index, "\nError Id:            ")) return false;
	if (!addInt(data, length, index, mErrorId)) return false;

	if (!addChar(data, length, index, "\nSeqno:               ")) return false;
	if (!addInt(data, length, index, mAlarmSeqno)) return false;

	if (!addChar(data, length, index, "\nTrigger Time:        ")) return false;
	if (!addChar(data, length, index, mTime.data())) return false;

	if (!addChar(data, length, index, "\nStackAddr:-----------\n")) return false;
	if (!addChar(data, length, index, addrs.data())) return false;

	if (!addChar(data, length, index, "\nError Message:       ")) return false;
	if (!addChar(data, length, index, mErrMsg.data())) return false;

	if (!addChar(data, length, index, "\n*************************\n")) return false;
	return true;
}


bool
OmnAlarmEntry::toString_SynErr(
		const int num_alarms,
		char *data,
		const int length) const
{
	//
	// Write the alarm in the following form:
	//
	//	"\n\n<"
	//	<< OmnTime::getSecTick()
	//	<< "> [Syntax Error][File:Line]"<<
	//	"\nTrigger Thread ID: " << OmnGetCurrentThreadId() <<
	//	"\nTrigger Time:      " << mTime <<
	//	"\nError Message:     " << mErrMsg <<
	//	"\n\n";
	//
	// Note that we did not use OmnString in order not to generate any alarms
	// while converting this entry into a string.
	//
	int index = 0;
	if (!addChar(data, length, index, "\n\n<")) return false;
	if (!addInt(data, length, index, OmnTime::getSecTick())) return false;

	OmnString aa = "> [Syntax Error][";
	aa << mFile << ":" << mLine << ":" << num_alarms << "]";
	if (!addChar(data, length, index, aa.data())) return false;

	if (!addChar(data, length, index, "\nThread ID:           ")) return false;
	if (!addIntHex(data, length, index, OmnGetCurrentThreadId())) return false;

	if (!addChar(data, length, index, "\nTrigger Time:        ")) return false;
	if (!addChar(data, length, index, mTime.data())) return false;

	if (!addChar(data, length, index, "\nError Message:       ")) return false;
	if (!addChar(data, length, index, mErrMsg.data())) return false;

	if (!addChar(data, length, index, "\n\n")) return false;

	return true;
}

