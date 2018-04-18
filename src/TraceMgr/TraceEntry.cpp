////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005
// Packet Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification is not permitted unless authorized in writing by a duly
// appointed officer of Packet Engineering, Inc. or its derivatives
//
// File Name: TraceEntry.cpp
// Description:
//   
//
// Modification History:
////////////////////////////////////////////////////////////////////////////
#include "TraceMgr/TraceEntry.h"

#include "TraceMgr/TraceMgr.h"
#include "Porting/ThreadDef.h"

#include <iostream>
#include <iomanip>


OmnTraceEntry::OmnTraceEntry()
:
mThreadId(OmnGetCurrentThreadId())
{
}


OmnTraceEntry::~OmnTraceEntry()
{
}


void
OmnTraceEntry::init(
		const char * file,
		const int line,
		const OmnTraceEntry::logLevel level)
{
	refresh();
	mFile = file;
	mLine = line;
	mLevel = level;
	initEntryHead();
}


void
OmnTraceEntry::initEntryHead()
{
	mStream.str("");
	mStream << '[';
	mStream << "0x" << std::hex << std::setw(8) << std::setfill('0') << mThreadId;
	mStream << ":";
	mStream << mFile;
	mStream << ":";
	mStream << std::dec << mLine;
	mStream << "] ";

	mHead = mStream.str();
	mStream.str("");
}


void
OmnTraceEntry::refresh()
{
	OmnTraceMgr * mgr = OmnTraceMgr::getSelf();
	if (mgr && !mStream.str().empty())
	{
		string ss = mHead;
		ss += mStream.str();
		mgr->writeTraceEntry(ss, mLevel);
	}
	mStream.str("");
}


OmnTraceEntry & 
OmnTraceEntry::operator << (ostream & (*f)(ostream &outs))
{
	refresh();
	return *this;
}


OmnTraceEntry & 
OmnTraceEntry::operator << (const std::string &v)
{
	mStream << v;
	return *this;
}


OmnTraceEntry & 
OmnTraceEntry::operator << (const char *v)
{
	mStream << v;
	return *this;
}


OmnTraceEntry & 
OmnTraceEntry::operator << (const char v)
{
	mStream << v;
	return *this;
}


OmnTraceEntry & 
OmnTraceEntry::operator << (const signed char v)
{
	mStream << v;
	return *this;
}


OmnTraceEntry & 
OmnTraceEntry::operator << (const unsigned char v)
{
	mStream << v;
	return *this;
}


OmnTraceEntry & 
OmnTraceEntry::operator << (const long long int v)
{
	mStream << std::dec << v;
	return *this;
}



OmnTraceEntry & 
OmnTraceEntry::operator << (const unsigned long long int v)
{
	mStream << std::dec << v;
	return *this;
}


OmnTraceEntry &
OmnTraceEntry::operator << (const double v)
{
	mStream << std::dec << v;
	return *this;
}


OmnTraceEntry &
OmnTraceEntry::operator << (const bool v)
{
	if (v) mStream << "True";
	else mStream << "False";
	return *this;
}



OmnTraceEntry & 
OmnTraceEntry::operator << (const void * const v)
{
	mStream << "0x" << std::hex << (unsigned long long int)v;
	return *this;
}

