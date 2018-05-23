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
// 2016/04/06 Creared by Andy Zhang
////////////////////////////////////////////////////////////////////////////
#include "TimerNew/TimerMgrNew.h"

#include "Alarm/AlarmMacros.h"
#include "Thread/Thread.h"
#include "SingletonClass/SingletonImpl.cpp"
#include "Util/OmnNew.h"
#include "XmlUtil/XmlTag.h"


AosTimerMgrNew::AosTimerMgrNew()
:
AosJimoDbModule(module_id::eAosTimerMgrNew),
mCrtTimerId(0)
{
}


AosTimerMgrNew::~AosTimerMgrNew()
{
	mTimers.clear();
}


bool
AosTimerMgrNew::threadFunc(OmnThrdStatus &state, const OmnThreadPtr &thread)
{
	 while (state == OmnThrdStatus::eActive)
	 {
		 boost::asio::io_service::work work(mIoService);
		 mIoService.run();
		 OmnAlarm << "AosTimerMgrNew error." << enderr;
	 }
	 return true;
}

bool
AosTimerMgrNew::signal(const int threadLogicId)
{
	return true;
}

bool
AosTimerMgrNew::checkThread(OmnString &err, const int thrdLogicId)
{
	return true;
}

bool
AosTimerMgrNew::config(const AosXmlTagPtr &def)
{
	return true;
}

bool
AosTimerMgrNew::start()
{
	OmnThreadedObjPtr thisPtr(this, false);
	mThread = OmnNew OmnThread(thisPtr, "AosTimerMgrNew", 0);
	mThread->start();
	return true;
}

bool
AosTimerMgrNew::stop()
{
	mThread->stop();
	mIoService.stop();
	return true;
}

bool
AosTimerMgrNew::remove(const int time_id)
{
	mLock.lock();
	auto itr = mTimers.find(time_id);
	aos_assert_rl(itr != mTimers.end(), &mLock, false);
	timerptr_t t = itr->second;
	mTimers.erase(itr);
	mLock.unlock();

	t->cancel();
	return true;
}


bool
AosTimerMgrNew::cancel(const int time_id)
{
	mLock.lock();
	auto itr = mTimers.find(time_id);
	aos_assert_rl(itr != mTimers.end(), &mLock, false);
	timerptr_t t = itr->second;
	mLock.unlock();
	t->cancel();
	return true;
}

boost::asio::io_service&
AosTimerMgrNew::getIoServer()
{
	return mIoService;
}


