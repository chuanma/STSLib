#include "timeval.h"
//#include <sys/time.h>

class timer {
public:
	timer() 
	{ 
		gettimeofday(&_tv, NULL);	
	}

	void set_start_time(void) 
	{ 
		gettimeofday(&_tv, NULL);	
	}

	float diff_time(void)   
	{ 
		timeval tv2;
		gettimeofday(&tv2, NULL);
		double end   = tv2.tv_sec + tv2.tv_usec/1000000.;
		double start = _tv.tv_sec + _tv.tv_usec/1000000.;
		return (end-start);
	}
private:
	timeval _tv;	
};
	
//	clock_t ck = clock();
//      int gettimeofday(struct timeval *tv, struct timezone *tz);
//      int settimeofday(const struct timeval *tv , const struct timezone *tz);


