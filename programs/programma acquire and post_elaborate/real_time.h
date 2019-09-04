#pragma once


#ifndef _REAL_TIME_H_
#define _REAL_TIME_H_

#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>



class real_time
{
private:
	boost::timer::cpu_timer timer;
	boost::timer::cpu_times t;

public:

	void start();
	

	/// <summary>
	/// return enlapsed time in millisecond
	/// </summary>
	/// <returns></returns>
	float stop();

	/// <summary>
	/// get the time in millisecond
	/// </summary>
	/// <returns></returns>
	uint64_t get_curr_time();

};

#endif // #ifndef _REAL_TIME_H_