#include "timer.h"

struct timespec
timerStart(void)
{
        struct timespec start_time;
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
                return start_time;
}

long
timerEnd(struct timespec start_time)
{
        struct timespec end_time;
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
                return timeDelta(start_time, end_time);
}

long
timeDelta(Timestamp start, Timestamp end)
{
        long diffInNanos = ((end.tv_sec - start.tv_sec) * 10000000000L) + (end.tv_nsec - start.tv_nsec);
            return diffInNanos;
}

