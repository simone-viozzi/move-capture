
#include "pch.h"
#include "real_time.h"


void real_time::start()
{
	timer.start();
	t = timer.elapsed();
}


float real_time::stop()
{
	timer.stop();
	boost::timer::cpu_times end = timer.elapsed();
	return (((end.system + end.user + end.wall) - (t.system + t.user + t.wall)) / 1000) / 1000;
}


uint64_t real_time::get_curr_time()
{
	auto now = boost::chrono::steady_clock::now().time_since_epoch();
	return boost::chrono::duration_cast<boost::chrono::milliseconds>(now).count();
}