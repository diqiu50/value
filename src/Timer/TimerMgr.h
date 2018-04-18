#ifndef TimerNew_TimerMgrNew_h
#define TimerNew_TimerMgrNew_h

#include "Alarm/AlarmMacros.h"
#include "SingletonClass/SingletonObjId.h"
#include "SingletonClass/SingletonTplt.h"
#include "Thread/Mutex.h"
#include "Thread/LockGuard.h"
#include "Util/TypeDef.h"
#include "SEInterfaces/JimoDbModuleObj.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <map>

OmnDefineSingletonClass(AosTimerMgrNewSingleton,
			AosTimerMgrNew,
			AosTimerMgrNewSelf,
			OmnSingletonObjId::eTimerMgrNew,
			"AosTimerMgrNew");

class AosTimerMgrNew : public OmnThreadedObj,
					   virtual public AosJimoDbModule
{
	OmnDefineRCObject;
private:
	typedef std::chrono::high_resolution_clock::time_point				timer_at_t;
	typedef boost::shared_ptr<boost::asio::high_resolution_timer>		timerptr_t;

private:
	std::map<int, timerptr_t>			mTimers;
	boost::asio::io_service				mIoService;
	int									mCrtTimerId = 0;
	OmnMutex							mLock;
	OmnThreadPtr						mThread;

public:
	AosTimerMgrNew();
	~AosTimerMgrNew();

	// OmnThreadedObj Interface
	virtual bool    threadFunc(OmnThrdStatus &state, const OmnThreadPtr &thread);
	virtual bool    signal(const int threadLogicId);
	virtual bool    checkThread(OmnString &err, const int thrdLogicId);

	// Singleton class interface
	static AosTimerMgrNew* 	    getSelf();
	virtual bool 				start();
	virtual bool 				stop();
	virtual OmnString   		getSysObjName() const {return "AosTimerMgrNew";}
	virtual bool        		config(const AosXmlTagPtr &def);

	template<typename WaitHandler>
	bool			 addTimer(
						int 			&time_id,
						WaitHandler 	&&handler,
						const i64		seconds,
						const i64 		milliseconds = 0)
	{
		OmnLockGuard<OmnMutex> g(&mLock);
		timerptr_t t;
		if (time_id == -1)
		{
			time_id = mCrtTimerId;
			aos_assert_r(mTimers.count(time_id) == 0, false);
			t = boost::make_shared<boost::asio::high_resolution_timer>(mIoService);
			mTimers[time_id] = t;
			mCrtTimerId++;
		}
		else
		{
			auto itr = mTimers.find(time_id);
			aos_assert_r(itr != mTimers.end(), false);
			t = itr->second;
		}

		if (seconds > 0) t->expires_from_now(std::chrono::seconds(seconds));
		else t->expires_from_now(std::chrono::milliseconds(milliseconds));

		t->async_wait(handler);
		return true;
	}

	template<typename WaitHandler>
	bool			addTimer(
						int 			&time_id,
						WaitHandler 	&&handler,
						timer_at_t		&timer_at)
	{
		OmnLockGuard<OmnMutex> g(&mLock);
		timerptr_t t;
		if (time_id == -1)
		{
			time_id = mCrtTimerId;
			aos_assert_r(mTimers.count(time_id) == 0, false);
			t = boost::make_shared<boost::asio::high_resolution_timer>(mIoService);
			mTimers[time_id] = t;
			mCrtTimerId++;
		}
		else
		{
			auto itr = mTimers.find(time_id);
			aos_assert_r(itr != mTimers.end(), false);
			t = itr->second;
		}
		t->expires_at(timer_at);
		t->async_wait(handler);
		return true;
	}

	bool						cancel(const int time_id);
	bool 						remove(const int time_id);
	boost::asio::io_service&	getIoServer();

	virtual bool module_exit() final
	{
		return stop();
	}

	virtual bool module_start() final
	{
		return start();
	}

	virtual bool module_config(const AosXmlTagPtr &app_config) final
	{
		return config(app_config);
	}
};

#endif
