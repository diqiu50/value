////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005
// Packet Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification is not permitted unless authorized in writing by a duly
// appointed officer of Packet Engineering, Inc. or its derivatives
//
// File Name: TraceEntry.h
// Description:
//   
//
// Modification History:
// 
////////////////////////////////////////////////////////////////////////////
#ifndef Omn_TraceMgr_TraceEntry_h
#define Omn_TraceMgr_TraceEntry_h

#include "Util/String.h"
#include <sstream>

class OmnTraceEntry
{
public:
	enum logLevel
	{
		eAllLevel = 0,
		eTraceLevel = 0,
		eDebugLevel = 10000,
		eInfoLevel = 20000,
		eWarnLevel = 30000,
		eErrorLevel = 40000,
		eFatalLevel = 50000,
		eOffLevel = 60000
	};

private:
	u64 			mThreadId;
	const char *	mFile;
	int				mLine = 0;
	logLevel		mLevel;

	stringstream	mStream;
	string			mHead;

public:
	OmnTraceEntry();
	~OmnTraceEntry();

	void		init(
					const char *file,
					const int line,
					const OmnTraceEntry::logLevel level); 
	void		initEntryHead();
	void		refresh();

	OmnTraceEntry & operator << (ostream & (*f)(ostream &outs));

	OmnTraceEntry & operator << (const char *v);
	OmnTraceEntry & operator << (const std::string &v);
	OmnTraceEntry & operator << (const OmnString &v) { return operator << (v.data()); }

	OmnTraceEntry & operator << (const char v);
	OmnTraceEntry & operator << (const signed char v);
	OmnTraceEntry & operator << (const unsigned char v);
	OmnTraceEntry & operator << (const short int v) { return operator << ((long long int) v); }
	OmnTraceEntry & operator << (const int v) { return operator << ((long long int) v); }
	OmnTraceEntry & operator << (const long int v) { return operator << ((long long int) v); }
	OmnTraceEntry & operator << (const long long int v);
	OmnTraceEntry & operator << (const unsigned short int v) { return operator << ((unsigned long long int) v); }
	OmnTraceEntry & operator << (const unsigned int v) { return operator << ((unsigned long long int) v); }
	OmnTraceEntry & operator << (const unsigned long int v) { return operator << ((unsigned long long int) v); }
	OmnTraceEntry & operator << (const unsigned long long int v);
	OmnTraceEntry & operator << (const float v) { return operator << ((double) v); }
	OmnTraceEntry & operator << (const double v);
	OmnTraceEntry & operator << (const bool v);

	OmnTraceEntry & operator << (const void * const v);
	
//	template <class T>
//	OmnTraceEntry & operator << (T) = delete;
};

#endif

