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
//
// Modification History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef Omn_Alarm_AlarmMgr_h
#define Omn_Alarm_AlarmMgr_h

#include "Alarm/AlarmEntry.h"
#include "Debug/ErrId.h"
#include "Util/HashUtil.h"
#include "XmlUtil/Ptrs.h"


class OmnAlarmMgr
{
public:
	struct Entry
	{
		int tick;
		int num;

		Entry()	: tick(0),num(0) {}
		Entry(int t, int n)	: tick(t),num(n) {}
	};

	typedef hash_map<const OmnString, int, Omn_Str_hash, compare_str> igmap_t;
	typedef hash_map<const OmnString, int, Omn_Str_hash, compare_str>::iterator igitr_t;
	typedef hash_map<const OmnString, Entry, Omn_Str_hash, compare_str> map_t;
	typedef hash_map<const OmnString, Entry, Omn_Str_hash, compare_str>::iterator mapitr_t;

private:
	static __thread OmnAlarmEntry * stAlarmEntry;

	static igmap_t				smIgnoredAlarms;
	static map_t				smAlarmTracker;
	static bool					smPauseOnAlarm;
	static bool					smPrintFullBacktrace;
	static int					smMaxAlarms;
	static OmnString			smModuleName;

	OmnAlarmMgr();
	~OmnAlarmMgr();

public:
	static bool init();
	static bool config(const AosXmlTagPtr &conf);

	static OmnAlarmEntry & getEntry(
					const OmnString &file,
					const int line,
					const OmnErrId::E alarmId);
	static OmnAlarmEntry & setAlarm(
					OmnAlarmEntry &alarm,
					const OmnString &file,
					const int line,
					const OmnErrId::E errId);
	static bool	closeEntry(const OmnAlarmEntry &entry);

	static void pauseOnAlarm();

	static void	setMaxAlarms(const int num) {smMaxAlarms = num;}
	static OmnString getCurTime();
	static OmnString getModuleName() {return smModuleName;}

	static bool ignoreAlarm(const OmnString &fname, const int line);
	static bool setIgnoredAlarm(const OmnString &fname, const int line);
	static bool removeIgnoredAlarm(const OmnString &fname, const int line);

	static bool setIgnoredAlarms(const OmnString &alarms);
	static OmnString getIgnoredAlarms();

	static int runCmd(char * cmd, char * resp, int resp_len);
	static void getBacktrace(std::vector<string> &traces);
	static bool isPrintFullBacktrace() {return smPrintFullBacktrace;}

	static void	raiseAlarmFromAssert(
					const OmnString &file,
					const int line,
					const OmnString &msg);
};

#endif

