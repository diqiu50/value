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
// 2017/09/25 Ken Lee
////////////////////////////////////////////////////////////////////////////
#ifndef Omn_TraceMgr_TraceMgr_h
#define Omn_TraceMgr_TraceMgr_h

#include "SingletonClass/SingletonTplt.h"
#include "Thread/ThreadedObj.h"
#include "TraceMgr/TraceEntry.h"
#include "XmlUtil/Ptrs.h"

#include <log4cplus/logger.h>
#include <log4cplus/helpers/fileinfo.h>


OmnDefineSingletonClass(OmnTraceMgrSingleton,
						OmnTraceMgr,
						OmnTraceMgrSelf,
                		OmnSingletonObjId::eTraceMgr,
						"OmnTraceMgr");

class OmnTraceMgr : virtual public OmnThreadedObj
{
	OmnDefineRCObject;

private:
	static __thread OmnTraceEntry * stTraceEntry;

	log4cplus::Logger				mLogger;
	log4cplus::helpers::FileInfo	mLastFileInfo;

	std::string						mPropertyFilename = "";
	bool							mWatchPropertyChange = false;
	int								mWaitSec = 10;
	OmnThreadPtr					mThread;

public:
	OmnTraceMgr();
	~OmnTraceMgr();

	static OmnTraceMgr *	getSelf();
	virtual bool			start();
	virtual bool			stop();
	virtual bool			config(const AosXmlTagPtr &conf);

	// ThreadedObj Interface
	virtual bool	threadFunc(OmnThrdStatus &state, const OmnThreadPtr &thread);
    virtual bool	checkThread(OmnString &errmsg, const int tid) {return true;}
	virtual bool	signal(const int threadLogicId);

private:
	bool			initProperty();
	bool			checkForFileModification();
	void			updateLastModInfo();

public:
	static OmnTraceEntry & getEntry(
						const char *file,
						const int line,
						const OmnTraceEntry::logLevel level);

	void			writeTraceEntry(
						const string ss,
						const OmnTraceEntry::logLevel level);
};

#endif

