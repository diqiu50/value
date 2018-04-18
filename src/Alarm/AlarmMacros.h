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
#ifndef Omn_Alarm_AlarmMacros_h
#define Omn_Alarm_AlarmMacros_h

#include "Alarm/AlarmMgr.h"

extern char *aos_alarm_get_errmsg(const char *fmt, ...);

#ifndef OmnNotImplementedYet
#define OmnNotImplementedYet OmnAlarm << "Not implemented yet" << enderr;
#endif

#ifndef OmnShouldNeverComeHere
#define OmnShouldNeverComeHere OmnAlarm << "Should never come to this point" << enderr;
#endif

#ifndef OmnWarn 
#define OmnWarn OmnAlarmMgr::getEntry(__FILE__, __LINE__, OmnErrId::eWarning)
#endif

#ifndef OmnWarnProgError
#define OmnWarnProgError OmnAlarmMgr::getEntry(__FILE__, __LINE__, OmnErrId::eWarning)
#endif

#ifndef OmnAlarm
#define OmnAlarm OmnAlarmMgr::getEntry(__FILE__, __LINE__, OmnErrId::eAlarm)
#endif

#ifndef OmnProgAlarm 
#define OmnProgAlarm OmnAlarmMgr::getEntry(__FILE__, __LINE__, OmnErrId::eProgError)
#endif

#ifndef OmnAlarmProgError
#define OmnAlarmProgError OmnAlarmMgr::getEntry(__FILE__, __LINE__, OmnErrId::eProgError)
#endif

#ifndef aos_alarm
#define aos_alarm(format, x...) 						\
	OmnAlarmMgr::raiseAlarmFromAssert(					\
		__FILE__, __LINE__, 							\
		aos_alarm_get_errmsg(format, ##x));
#endif

#ifndef aos_assert
#define aos_assert(cond)								\
{														\
	if (!(cond))										\
	{													\
		OmnAlarmMgr::raiseAlarmFromAssert(				\
			__FILE__, __LINE__, "");					\
		return;											\
	}													\
}
#endif

#ifndef aos_assert_l
#define aos_assert_l(cond, lock)						\
{														\
	if (!(cond))										\
	{													\
		OmnAlarmMgr::raiseAlarmFromAssert(				\
			__FILE__, __LINE__, "");					\
		AOSUNLOCK(lock);								\
		return;											\
	}													\
}
#endif

#ifndef aos_assert_r
#define aos_assert_r(cond, returncode)					\
{														\
	if (!(cond))										\
	{													\
		OmnAlarmMgr::raiseAlarmFromAssert(				\
			__FILE__, __LINE__, "");					\
		return (returncode);							\
	}													\
}
#endif

#ifndef aos_assert_rb
#define aos_assert_rb(cond, lock, returncode)			\
{														\
	if (!(cond))										\
	{													\
		OmnAlarmMgr::raiseAlarmFromAssert(				\
			__FILE__, __LINE__, "");					\
		AOSUNLOCK(lock);								\
		return (returncode);							\
	}													\
}
#endif

#ifndef aos_assert_rg
#define aos_assert_rg(cond, rdata, returncode, errmsg)	\
{														\
	if (!(cond))										\
	{													\
		rdata->setError((errmsg).getErrmsg(),			\
			__FILE__, __LINE__);						\
		OmnAlarmMgr::raiseAlarmFromAssert(				\
			__FILE__, __LINE__,							\
			(errmsg).getErrmsg());						\
		return (returncode);							\
	}													\
}
#endif

#ifndef aos_assert_rl
#define aos_assert_rl(cond, lock, returncode)			\
{														\
	if (!(cond))										\
	{													\
		OmnAlarmMgr::raiseAlarmFromAssert(              \
			__FILE__, __LINE__, "");					\
		AOSUNLOCK(lock);								\
		return (returncode);							\
	}													\
}
#endif

#ifndef aos_assert_rm
#define aos_assert_rm(cond, returncode, format, x...)	\
{														\
	if (!(cond))										\
	{													\
		aos_alarm(format, ##x);							\
		return (returncode);							\
	}													\
}
#endif

#ifndef aos_assert_rr
#define aos_assert_rr(cond, rdata, returncode)			\
{														\
	if (!(cond))										\
	{													\
		if (rdata) rdata->setError(						\
			"assert_error", __FILE__, __LINE__);		\
		OmnAlarmMgr::raiseAlarmFromAssert(				\
			__FILE__, __LINE__, "assert_error");		\
		return (returncode);							\
	}													\
}
#endif

#endif

