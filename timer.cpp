#include "timer.h"

#ifndef _WIN32
#include <time.h>

#define NS_PER_SEC 1000000000LL
void QueryPerformanceFrequency(LARGE_INTEGER *freq)
{
	freq->QuadPart=NS_PER_SEC;
}

void QueryPerformanceCounter(LARGE_INTEGER *counter)
{
	int64_t ticks;
#ifdef __linux__
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	ticks = now.tv_sec;
	ticks *= NS_PER_SEC;
	ticks += now.tv_nsec;
#elif __APPLE__
	ticks = mach_absolute_time();
#else
	ticks = 0;
#endif
	counter->QuadPart=ticks;
}
#endif

Timer::Timer()
	{
	QueryPerformanceFrequency(&timerFrequency);
	tfreq=(double)timerFrequency.QuadPart;	
	// calibration
	starttimer();
	stoptimer();
	tcorrection=(double)(endCount.QuadPart-startCount.QuadPart);
	tbegin=tstart;
	}

Timer::~Timer()
	{}

void Timer::starttimer(void)
	{
	time=0.0;
	QueryPerformanceCounter(&startCount);
	tstart=(double)startCount.QuadPart;
	}

void Timer::stoptimer(void)
	{
	QueryPerformanceCounter(&endCount);
	tstop=(double)endCount.QuadPart;
	}

void Timer::update(void)
	{
	stoptimer();
	dt=(float)((tstop-tstart-tcorrection)/(tfreq));
	elapsed=(float)((tstop-tbegin)/tfreq);
	starttimer();
	}
