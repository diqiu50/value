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
#ifndef Omn_TraceMgr_TraceMacros_h
#define Omn_TraceMgr_TraceMacros_h

#include "TraceMgr/TraceMgr.h"


#ifndef OmnScreenTrace
#define OmnScreenTrace OmnTraceMgr::getEntry(__FILE__, __LINE__, OmnTraceEntry::eTraceLevel)
#endif

#ifndef OmnScreenDebug
#define OmnScreenDebug OmnTraceMgr::getEntry(__FILE__, __LINE__, OmnTraceEntry::eDebugLevel)
#endif

#ifndef OmnScreenInfo
#define OmnScreenInfo OmnTraceMgr::getEntry(__FILE__, __LINE__, OmnTraceEntry::eInfoLevel)
#endif

#ifndef OmnScreenWarn
#define OmnScreenWarn OmnTraceMgr::getEntry(__FILE__, __LINE__, OmnTraceEntry::eWarnLevel)
#endif

#ifndef OmnScreenError
#define OmnScreenError OmnTraceMgr::getEntry(__FILE__, __LINE__, OmnTraceEntry::eErrorLevel)
#endif

#ifndef OmnScreenFatal
#define OmnScreenFatal OmnTraceMgr::getEntry(__FILE__, __LINE__, OmnTraceEntry::eFatalLevel)
#endif

#ifndef OmnScreen
#define OmnScreen OmnScreenInfo
#endif

#ifndef OmnMark
#define OmnMark OmnScreenDebug << "Mark" << endl;
#endif

#ifndef OmnTagFuncInfo
#define OmnTagFuncInfo if (false) \
			OmnScreenDebug << "[" << __FUNCTION__ << "]"
#endif

#endif

