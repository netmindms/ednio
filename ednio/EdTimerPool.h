/*
 * EdTimerPool.h
 *
 *  Created on: Sep 18, 2016
 *      Author: netmind
 */

#ifndef EDTIMERPOOL_H_
#define EDTIMERPOOL_H_

#include <chrono>
#include <functional>
#include <list>
#include <unordered_map>
#include <vector>
#include "EdTimer.h"

namespace edft {

class EdTimerPool {
public:
	struct TimerInfo {
		uint32_t _handle;
		std::chrono::steady_clock::time_point _fireTp;
		uint64_t _period;
		std::function<void(int)> _lis;

	};
	EdTimerPool();
	virtual ~EdTimerPool();
	void open();
	void close();
	uint32_t setTimerUs(uint32_t handle, uint64_t firstus, uint64_t periodus, std::function<void(int)> lis);
	uint32_t setTimer(uint32_t handle, uint64_t firstms, uint64_t periodms, std::function<void(int)> lis);
	void killTimer(uint32_t id);
private:
	EdTimer _mainTimer;
	std::chrono::steady_clock::time_point _nearTp;
	std::unordered_map<uint32_t, TimerInfo> _timerMap;
	uint32_t _handleSeed;
	std::vector<std::pair<int, TimerInfo*>> _fireList;
	void scheduleTimer();
};

} /* namespace ednio */

#endif /* EDTIMERPOOL_H_ */
