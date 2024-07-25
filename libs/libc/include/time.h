#ifndef _TIME_H_
#define _TIME_H_

typedef unsigned long int  time_t;
typedef unsigned long int  clock_t;

struct timespec {
    time_t  tv_sec;     /**> Seconds */
    long    tv_nsec;    /**> Nanoseconds */
};

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
	long __tm_gmtoff;
	const char *__tm_zone;
};

// Clock

#define CLOCKS_PER_SEC ((clock_t) 100)

clock_t clock(void);

// Time functions

time_t time(time_t* out);


struct tm *gmtime (const time_t *);
struct tm *localtime (const time_t *);
char *asctime (const struct tm *);
char *ctime (const time_t *);

char * asctime(const struct tm * tm);

#endif /* _TIME_H_ */
