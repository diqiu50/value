////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005
// Packet Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification is not permitted unless authorized in writing by a duly
// appointed officer of Packet Engineering, Inc. or its derivatives
//
// File Name: AlarmMgr.cpp
// Description:
//  This is a singleton class. It provides alarm management for this
//  module.
//
//  When a function triggers an alarm, it should create an alarm and
//  add the alarm to this class. This class maintains all the alarms
//  it collected. For every alarm it receives, it sends the alarm to
//  the PNMS Alarm Manager, which provides a system-wide alarm
//  management.
//
//	ErrId is used to identify alarms.
//
// Modification History:
//
////////////////////////////////////////////////////////////////////////////
#include "Alarm/AlarmMgr.h"

#include "API/AosApiG.h"
#include "Debug/Debug.h"
#include "Porting/Sleep.h"
#include "Porting/GetTime.h"
#include "Porting/Mutex.h"
#include "Thread/Mutex.h"
#include "Util1/Time.h"
#include "XmlUtil/XmlTag.h"

#include <stdarg.h>
#include <execinfo.h>
#include <stdlib.h>

__thread OmnAlarmEntry * OmnAlarmMgr::stAlarmEntry = nullptr;


static u32 sgAlarmId = 101;
static OmnMutexType	sgLock = OmnInitMutex(sgLock);
OmnAlarmMgr::igmap_t	OmnAlarmMgr::smIgnoredAlarms;
OmnAlarmMgr::map_t		OmnAlarmMgr::smAlarmTracker;
bool					OmnAlarmMgr::smPauseOnAlarm = false;
bool					OmnAlarmMgr::smPrintFullBacktrace = true;
int						OmnAlarmMgr::smMaxAlarms = 5;
OmnString				OmnAlarmMgr::smModuleName = "";


OmnAlarmMgr::OmnAlarmMgr()
{
}


OmnAlarmMgr::~OmnAlarmMgr()
{
}


bool
OmnAlarmMgr::init()
{
	config(AosGetConfig());
	return true;
}


bool
OmnAlarmMgr::config(const AosXmlTagPtr &conf)
{
	if (!conf) return true;

	smModuleName = conf->getAttrStr("module_name");

	AosXmlTagPtr tag = conf->getFirstChild("AlarmMgr");
	if (!tag) return true;

	smPauseOnAlarm = tag->getAttrBool("pause_on_alarm", false);
	smPrintFullBacktrace = tag->getAttrBool("print_full_backtrace", true);
	return true;
}


OmnAlarmEntry &
OmnAlarmMgr::getEntry(
		const OmnString &file,
		const int line,
		const OmnErrId::E errId)
{
	if (stAlarmEntry == nullptr) stAlarmEntry = OmnNew OmnAlarmEntry();

	// Construct the time string
	OmnString time_str = OmnGetTime(AosLocale::eChina);

	stAlarmEntry->setSeqno(sgAlarmId++);
	stAlarmEntry->setErrId(errId);
	stAlarmEntry->setFile(file);
	stAlarmEntry->setLine(line);
	stAlarmEntry->setTime(time_str);
	stAlarmEntry->setErrMsg("");

	return *stAlarmEntry;
}


bool
OmnAlarmMgr::closeEntry(const OmnAlarmEntry &theEntry)
{
	// It is the time to close the event entry. If 'entryId' is not correct,
	// it is a serious program error. Generate a special alarm.
	// Check whether we need to raise the alarm. Some alarms are controlled
	// on the frequency at which the alarms can be raised.
	//
	OmnErrId::E errId = theEntry.getErrId();
	if (!OmnErrId::isValid(errId))
	{
		// This is a serious problem. Should never happen.
		// We will change it to OmnErrId::eIncorrect.
		errId = OmnErrId::eIncorrect;
	}

	int tick = OmnTime::getSecTick();
	OmnString key = theEntry.getFilename();
	key << "_" << theEntry.getLine();

	OmnMutexLock(sgLock);
	mapitr_t itr = smAlarmTracker.find(key);
	bool needToRaise = false;
	int num_alarms = 1;
	if (itr == smAlarmTracker.end())
	{
		smAlarmTracker[key] = Entry(tick, 1);
		needToRaise = true;
	}
	else
	{
		if (itr->second.tick != tick)
		{
			itr->second.tick = tick;
			itr->second.num = 0;
		}
		num_alarms = ++(itr->second.num);
		if (num_alarms < smMaxAlarms)
		{
			needToRaise = true;
		}
	}
	OmnMutexUnlock(sgLock);

	if (needToRaise)
	{
		char buff[10000];
		bool rslt = theEntry.toString(num_alarms, buff, 10000);
		if (rslt)
		{
			OmnScreenError << buff << endl;
		}
		else
		{
			OmnScreenError << "alarm error" << endl;
		}
	}

	if (!ignoreAlarm(theEntry.getFilename(), theEntry.getLine()))
	{
		pauseOnAlarm();
	}

	return false;
}


bool
OmnAlarmMgr::ignoreAlarm(
		const OmnString &fname,
		const int line)
{
	OmnString key = fname;
	key << "_" << line;
	OmnMutexLock(sgLock);
	igitr_t itr = smIgnoredAlarms.find(key);
	if (itr == smIgnoredAlarms.end())
	{
		OmnMutexUnlock(sgLock);
		return false;
	}

	OmnMutexUnlock(sgLock);
	return true;
}


bool
OmnAlarmMgr::setIgnoredAlarm(
		const OmnString &fname,
		const int line)
{
	OmnString key = fname;
	key << "_" << line;
	OmnMutexLock(sgLock);
	smIgnoredAlarms[key] = line;
	OmnMutexUnlock(sgLock);
	return true;
}


bool
OmnAlarmMgr::removeIgnoredAlarm(
		const OmnString &fname,
		const int line)
{
	OmnString key = fname;
	key << "_" << line;
	OmnMutexLock(sgLock);
	smIgnoredAlarms.erase(key);
	OmnMutexUnlock(sgLock);
	return true;
}


bool
OmnAlarmMgr::setIgnoredAlarms(const OmnString &ignored_alarms)
{
	// 'alarms' is in the form:
	// 	"fname:line,fname:line,..."

	const char *alarms = ignored_alarms.data();
	int len = ignored_alarms.length();
	int idx = 0;
	while (idx < len)
	{
		int name_start = idx;
		while (idx < len && alarms[idx] != ':') idx++;

		if (alarms[idx] != ':') break;

		int name_end = idx-1;
		if (name_end <= name_start)
		{
			cout << __FILE__ << ":" << __LINE__ << "************"
				<< name_start << ":" << name_end;
			return false;
		}

		idx++;
		int line_start = idx;
		while (idx < len && alarms[idx] != ',') idx++;

		int line_end = idx - 1;
		if (line_end <= line_start)
		{
			cout << __FILE__ << ":" << __LINE__ << "************"
				<< name_start << ":" << name_end;
			return false;
		}

		OmnString key(&alarms[name_start], name_end - name_start+1);
		int mm = atoi(&alarms[line_start]);
		key << "_" << mm;

		OmnMutexLock(sgLock);
		smIgnoredAlarms[key] = mm;
		OmnMutexUnlock(sgLock);

		OmnScreen << "Add ignored alarm: " << key << endl;
		idx++;
	}

	return true;
}


OmnString
OmnAlarmMgr::getIgnoredAlarms()
{
	OmnString results = "<Contents>";

	OmnMutexLock(sgLock);
	igitr_t itr = smIgnoredAlarms.begin();
	for (; itr != smIgnoredAlarms.end(); itr++)
	{
		results << "<entry filename=\"" << itr->first << "\" line=\""
			<< itr->second << "\"/>";
	}
	OmnMutexUnlock(sgLock);
	results << "</Contents>";
	return results;
}


void
OmnAlarmMgr::pauseOnAlarm()
{
	if (!smPauseOnAlarm) return;

	bool pause_on_alarm = true;
	while (pause_on_alarm)
	{
		OmnScreen << "alarm sleep!" << endl;
		OmnSleep(5);
	}
}


OmnString
OmnAlarmMgr::getCurTime()
{
	time_t rawtime;
	time(&rawtime);
	struct tm* ptm = gmtime(&rawtime);

	char time_str[128];
	sprintf(time_str, "%02d%02d%02d%02d%02d", ptm->tm_mon+1,
		ptm->tm_mday, (ptm->tm_hour+8)%24, ptm->tm_min, ptm->tm_sec);

	return time_str;
}


void
OmnAlarmMgr::raiseAlarmFromAssert(
		const OmnString &file,
		const int line,
		const OmnString &msg)
{
	OmnAlarmEntry entry = OmnAlarmMgr::getEntry(file, line, OmnErrId::eAssertFail);
	entry.setModuleName(OmnAlarmMgr::getModuleName());
	entry << msg << enderr;
}


int
OmnAlarmMgr::runCmd(char * cmd, char * resp, int resp_len)
{
	FILE *fp = popen(cmd, "r");
	if (fp == nullptr) return -1;
	fgets(resp, resp_len, fp);
	pclose(fp);
	return 0;
}


void
OmnAlarmMgr::getBacktrace(std::vector<string> &traces)
{
	traces.clear();
	void *buff[100];
	int sizes = backtrace(buff, 100);
	char **strings = backtrace_symbols(buff, sizes);

	char exe_path[1024] = {0};
	readlink("/proc/self/exe", exe_path, sizeof(exe_path));
	if (exe_path[strlen(exe_path)-1]=='\n')
	{
		exe_path[strlen(exe_path)-1]='\0';
	}

	char addrStr[100];
	OmnString cmd = "addr2line -e ";
	cmd << exe_path;
	for (int i=0; i<sizes; i++)
	{
		sprintf(addrStr, "0x%lx", (u64)buff[i]);
		cmd << " " << addrStr;
	}

	FILE *pp = popen(cmd.data(), "r"); //建立管道
	if (pp)
	{
		char tmp[1024] = {0}; //设置一个合适的长度，以存储每一行输出
		while (fgets(tmp, sizeof(tmp), pp) != nullptr)
		{
			if (tmp[strlen(tmp) - 1] == '\n')
			{
				tmp[strlen(tmp) - 1] = '\0'; //去除换行符
			}

			if (0 == strcmp(tmp, "??:0"))
			{
				if (strstr(strings[traces.size()], "Runtime_lib") == nullptr)
				{
					traces.push_back(strings[traces.size()]);
				}
				else
				{
					char *begin_name = 0;
					char *begin_offset = 0;
					char *end_offset = 0;
					for (char *p = strings[traces.size()]; *p; ++p) { // 利用了符号信息的格式
						if (*p == '(') { // 左括号
							begin_name = p;
						}
						else if (*p == '+' && begin_name) { // 地址偏移符号
							begin_offset = p;
						}
						else if (*p == ')' && begin_offset) { // 右括号
							end_offset = p;
							break;
						}
					}

					if (begin_name && begin_offset && end_offset ) {
						*begin_name++ = '\0';
						*begin_offset++ = '\0';
						*end_offset = '\0';

						OmnString cmd;
						cmd << "nm " << strings[traces.size()] << " | grep " << begin_name << " | cut -d ' ' -f 1";
						char offset[1024] = {0};
						OmnAlarmMgr::runCmd(cmd.getBuffer(), offset, sizeof(offset));

						cmd = "python -c 'print hex(0x";
						cmd << offset << "+" << begin_offset << ")' | xargs -I {} addr2line -e " << strings[traces.size()] << " {}";
						char trace[1024] = {0};
						OmnAlarmMgr::runCmd(cmd.getBuffer(), trace, sizeof(trace));

						if (trace[strlen(trace) - 1] == '\n')
						{
							trace[strlen(trace) - 1] = '\0'; //去除换行符
						}
						traces.push_back(trace);
					}
					else
					{
						traces.push_back(strings[traces.size()]);
					}
				}
			}
			else
			{
				traces.push_back(tmp);
			}
		}
		pclose(pp);
	}
	free(strings);
}


void raiseAlarmFromAssert(
		const char *file,
		const int line,
		int level,
		int module,
		int id,
		const char *msg)
{
	OmnAlarmEntry entry = OmnAlarmMgr::getEntry(file, line, OmnErrId::eAssertFail);
	entry.setModuleName(OmnAlarmMgr::getModuleName());
	OmnAlarmMgr::closeEntry(entry);
}


char * aos_alarm_get_errmsg(const char *fmt, ...)
{
	static int  slBufIndex = 0;
	static char slBuffer[10][1010];
	unsigned int index = (slBufIndex++) & 0x07;

	va_list args;

	va_start(args, fmt);
	vsprintf(slBuffer[index], fmt, args);
	va_end(args);

	return slBuffer[index];
}

