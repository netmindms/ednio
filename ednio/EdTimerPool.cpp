/*
 * EdTimerPool.cpp
 *
 *  Created on: Sep 18, 2016
 *      Author: netmind
 */

#define DBGTAG "TPOOL"
#define DBG_LEVEL DBG_DEBUG

#include <chrono>
#include "EdTimerPool.h"
#include "edslog.h"

using namespace std;
using namespace std::chrono;

namespace edft {

EdTimerPool::EdTimerPool() {
	_handleSeed = 0;
}

EdTimerPool::~EdTimerPool() {
	// TODO Auto-generated destructor stub
}

uint32_t EdTimerPool::setTimer(uint32_t handle, uint64_t firstms, uint64_t periodms, std::function<void(int)> lis) {
	return setTimerUs(handle, firstms*1000, periodms*1000, lis);
}

uint32_t EdTimerPool::setTimerUs(uint32_t handle, uint64_t firstus, uint64_t periodus, std::function<void(int)> lis) {
	if(handle==0) {
		if(++_handleSeed==0) ++_handleSeed;
		handle = _handleSeed;
	}
	auto &ti = _timerMap[handle];
	ti._handle = handle;
	ti._period = periodus;
	ti._lis = lis;
	auto ntp = chrono::steady_clock::now();
	ti._fireTp = ntp + std::chrono::microseconds(firstus);
	if(ti._fireTp < _nearTp) {
		_nearTp = ti._fireTp;
		auto ds = duration_cast<microseconds>(_nearTp - ntp).count();
		_mainTimer.setUsec(ds);
	}

	return ti._handle;
}

void EdTimerPool::killTimer(uint32_t id) {
	auto itr = _timerMap.find(id);
	if(itr != _timerMap.end()) {

		_timerMap.erase(itr);
	}
}

void EdTimerPool::open() {
	_nearTp = steady_clock::time_point::max();
	_mainTimer.setOnListener([this]() {
		scheduleTimer();
	});
}

void EdTimerPool::close() {
	_mainTimer.kill();
	_timerMap.clear();
}

void EdTimerPool::scheduleTimer() {
	if(_timerMap.size()<=0) {
		return;
	}
//	dbgd("schedule timer, cnt=%lu", _timerMap.size());
	_nearTp = steady_clock::time_point::max();
	auto ntp = steady_clock::now();
	int64_t minus=-1;
	_fireList.clear();
	for(auto &ti: _timerMap) {
		auto &tm = ti.second;
		auto us = duration_cast<microseconds>(ntp-tm._fireTp).count();
		if(us >=0) {
			auto cnt = us/tm._period + us%tm._period;
			tm._fireTp += microseconds(tm._period);
			_fireList.push_back({cnt, &tm});
		}
		if(tm._fireTp < _nearTp) {
			_nearTp = tm._fireTp;
			minus = us;
		}
	}

	for(auto &info: _fireList) {
		info.second->_lis(info.first);
	}

	ntp = steady_clock::now();
	auto us = duration_cast<microseconds>(_nearTp-ntp).count();
	if(us<0) us=0;
	_mainTimer.setUsec(us);
}

} /* namespace ednio */
