#ifndef TIMER_H
#define TIMER_H

#ifndef _WIN32
#include <stdint.h>
typedef union _LARGE_INTEGER {
  struct {
    uint32_t LowPart;
    int32_t  HighPart;
  } DUMMYSTRUCTNAME;
  struct {
    uint32_t LowPart;
    int32_t  HighPart;
  } u;
  int64_t QuadPart;
} LARGE_INTEGER;
#endif

class Timer	// high resolution timer
	{
	protected:
		LARGE_INTEGER timerFrequency;
		LARGE_INTEGER startCount;
		LARGE_INTEGER endCount;
		LARGE_INTEGER beginCount;
		LARGE_INTEGER currentCount;
		double tfreq;
		double time;
		double tstart;
		double tstop;
		double tbegin;
		double tcurrent;
	public:
		Timer(void);
		~Timer(void);
		double tcorrection;
		void starttimer(void);
		void stoptimer(void);
		void update(void);
		float elapsed;
		float dt;
	};

#endif
