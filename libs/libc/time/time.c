#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include "../internal/syscall.h"

time_t time(time_t* out) {
    time_t result = (time_t)syscall_invoke(syscall_gettime,0,0,0,0);
    if(out != NULL) {
        *out = result;
    }
    return result;
}

// 时间对比函数

double difftime(time_t t1, time_t t0)
{
	return t1-t0;
}


// 时间转换类函数

char * ctime(const time_t * t)
{
	return asctime((const struct tm *)localtime(t));
}

struct tm * localtime(const time_t * t)
{
	return gmtime(t);
}

static const char * week_days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char * month_days[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

char * asctime(const struct tm * tm)
{
	static char ascbuf[26];

	if(!tm) {
        return NULL;
    }

	snprintf(ascbuf, 26, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
			week_days[tm->tm_wday], month_days[tm->tm_mon], tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec, 1900 + tm->tm_year);
    
	return ascbuf;
}

time_t mktime(struct tm* tm_now)
{
    const unsigned int year0 = tm_now->tm_year+1900;
    const unsigned int mon0 = tm_now->tm_mon+1;
    const unsigned int day = tm_now->tm_mday;
    const unsigned int hour = tm_now->tm_hour;
    const unsigned int min = tm_now->tm_min;
    const unsigned int sec = tm_now->tm_sec;
	unsigned int mon = mon0, year = year0;
 
	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}
 
	return ((((unsigned int)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}