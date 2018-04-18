////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005
// Packet Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification is not permitted unless authorized in writing by a duly
// appointed officer of Packet Engineering, Inc. or its derivatives
//
// File Name: TraceMgr.cpp
// Description:
//
// Modification History:
//
////////////////////////////////////////////////////////////////////////////
#include "TraceMgr/TraceMgr.h"

#include "Porting/Sleep.h"
#include "SingletonClass/SingletonImpl.cpp"
#include "Thread/Thread.h"
#include "Util/OmnNew.h"
#include "XmlUtil/XmlTag.h"

#include <sys/stat.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>


__thread OmnTraceEntry * OmnTraceMgr::stTraceEntry = nullptr;


OmnSingletonImpl(OmnTraceMgrSingleton,
                 OmnTraceMgr,
                 OmnTraceMgrSelf,
                "OmnTraceMgr");

OmnTraceMgr::OmnTraceMgr()
{
	mLogger = log4cplus::Logger::getInstance("TraceMgr");
}


OmnTraceMgr::~OmnTraceMgr()
{
}


bool
OmnTraceMgr::start()
{
	return true;
}


bool
OmnTraceMgr::stop()
{
	return true;
}


bool
OmnTraceMgr::threadFunc(
		OmnThrdStatus &state,
		const OmnThreadPtr &thread)
{
	while (state == OmnThrdStatus::eActive)
	{
		OmnSleep(mWaitSec);
		bool modified = checkForFileModification();
		if (modified)
		{
			initProperty();
		}
	}

	return true;
}


bool
OmnTraceMgr::signal(const int threadLogicId)
{
	return true;
}


bool
OmnTraceMgr::config(const AosXmlTagPtr &config)
{
	OmnString base_dir = OmnApp::getAppBaseDir();
	OmnString dir_name = OmnApp::getDirName(base_dir);
	OmnString log_dir = OmnApp::getAppLogDir();
	if (log_dir == "")
	{
		log_dir << "./";
	}
	else
	{
		log_dir << "/";
	}
	OmnApp::createDir(log_dir);

	OmnString processname = OmnApp::getComposeProcessName();
	int idx = processname.find('_', false);
	OmnString appname = processname.substr(0, idx - 1);

	string fname = "log.properties/";
	fname += appname.data();
	fname += "Log.properties";
	mPropertyFilename = fname;

	initProperty();

	AosXmlTagPtr tag = config->getFirstChild("TraceMgr");
	if (tag)
	{
		mWatchPropertyChange = tag->getAttrBool("watch_property_change", false);
		mWaitSec = tag->getAttrInt("wait_sec", 10);

		if (mWatchPropertyChange)
		{
			OmnThreadedObjPtr thisptr(this, false);
			mThread = OmnNew OmnThread(thisptr, "TraceMgrThread", 1, true, true);
			mThread->start();
		}
	}

	return true;
}


bool
OmnTraceMgr::checkForFileModification()
{
	log4cplus::helpers::FileInfo fi;
	if (log4cplus::helpers::getFileInfo(&fi, mPropertyFilename) != 0)
		return false;

	bool modified = fi.mtime > mLastFileInfo.mtime || fi.size != mLastFileInfo.size;
	if (!modified && fi.is_link)
	{
		struct stat fileStatus;
		if (lstat(mPropertyFilename.c_str(), &fileStatus) == -1)
			return false;

		log4cplus::helpers::Time linkModTime(log4cplus::helpers::from_time_t(fileStatus.st_mtime));
		modified = (linkModTime > fi.mtime);
	}

	return modified;
}


void
OmnTraceMgr::updateLastModInfo()
{
	log4cplus::helpers::FileInfo fi;
	if (log4cplus::helpers::getFileInfo (&fi, mPropertyFilename) == 0)
		mLastFileInfo = fi;
}


bool
OmnTraceMgr::initProperty()
{
	OmnString processname = OmnApp::getComposeProcessName();
	int idx = processname.find('_', false);
	OmnString appname = processname.substr(0, idx - 1);

	try
	{
		log4cplus::helpers::Properties properties(mPropertyFilename);
		std::vector<log4cplus::tstring> propertyNames = properties.propertyNames();

		for (size_t i=0; i<propertyNames.size(); i++)
		{
			if (propertyNames[i].length() > 5 &&
				propertyNames[i].compare(propertyNames[i].length() - 5, 5, ".File") == 0)
			{
				log4cplus::tstring val = properties.getProperty(propertyNames[i]);
				val = val.replace(val.find(appname.data()), appname.length(), processname.data());
				properties.setProperty(propertyNames[i], val);
			}
		}

		log4cplus::PropertyConfigurator tmp(properties, log4cplus::Logger::getDefaultHierarchy(), 0);
		tmp.configure();

		updateLastModInfo();
	}
	catch(...)
	{
		cout << "error" << endl;
	}

	return true;
}


OmnTraceEntry &
OmnTraceMgr::getEntry(
		const char *file,
		const int line,
		const OmnTraceEntry::logLevel level)
{
	if (stTraceEntry == nullptr) stTraceEntry = OmnNew OmnTraceEntry();
	stTraceEntry->init(file, line, level);
	return *stTraceEntry;
}


void
OmnTraceMgr::writeTraceEntry(
		const string ss,
		const OmnTraceEntry::logLevel level)
{
	switch(level)
	{
		case OmnTraceEntry::eTraceLevel:
			 LOG4CPLUS_TRACE_STR(mLogger, ss);
			 break;

		case OmnTraceEntry::eDebugLevel:
			 LOG4CPLUS_DEBUG_STR(mLogger, ss);
			 break;

		case OmnTraceEntry::eInfoLevel:
			 LOG4CPLUS_INFO_STR(mLogger, ss);
			 break;

		case OmnTraceEntry::eWarnLevel:
			 LOG4CPLUS_WARN_STR(mLogger, ss);
			 break;

		case OmnTraceEntry::eErrorLevel:
			 LOG4CPLUS_ERROR_STR(mLogger, ss);
			 break;

		case OmnTraceEntry::eFatalLevel:
			 LOG4CPLUS_FATAL_STR(mLogger, ss);
			 break;

		default:
			 break;
	}
}

