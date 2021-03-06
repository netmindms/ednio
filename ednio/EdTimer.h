/*
 * EdTimer.h
 *
 *  Created on: 2014. 02. 10.
 *      Author: khkim
 */

#ifndef EDTIMER_H_
#define EDTIMER_H_
#include "ednio_config.h"

#include <functional>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include "EdType.h"
#include "EdEvent.h"

namespace edft {

class EdTimer;

class EdTimer : public EdEvent
{
public:
typedef std::function<void ()> Lis;
	EdTimer();
	virtual ~EdTimer();

public:
	/**
	 * @brief Start timer with specified interval mtime.
	 * @param mtime Expiration interval(milisec)
	 * @param first_msec Initial expiration time interval.
	 */
	void set(u32 mtime, u32 first_msec=0, Lis=nullptr);

	void setUsec(u64 usec, u64 first_usec=0, Lis=nullptr);

	/**
	 * @brief Reset timer.
	 */
	void reset(void);

	/**
	 * @brief Stop timer.
	 */
	void kill(void);

	/**
	 * @brief Set timer callback interface called when is expired.
	 * @param itimer ITimerCb interface instance.
	 */
	void setOnListener(Lis lis);

	/**
	 * @brief Test whether timer is running.
	 * @return
	 */
	bool isActive(void);

	/**
	 * @brief Return timer hit count.
	 * @return
	 */
	u64 getHitCount() { return mHitCount; };

	/**
	 * @brief Stop timer temporarily.
	 */
	void pause(void);

	/**
	 * @brief Restart timer paused by pause().
	 */
	void resume(void);

public:
	/**
	 * @brief Called when timer is expired.
	 * @remark To run your specific codes, override this virtual function when expired.\n
	 * You can use interface callback instead of overriding this function to know timer expiration.
	 */
	virtual void OnTimer(void);
	virtual void OnEventRead(void) override;
private:
	Lis mLis;
	uint64_t mHitCount;
	struct itimerspec mTimerSpec;


};


}

#endif /* EDTIMER_H_ */

