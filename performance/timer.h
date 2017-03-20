#include <time.h>

typedef struct timespec Timestamp;

struct timespec timerStart(void);
long timerEnd(struct timespec start_time);
long timeDelta(Timestamp start, Timestamp end);
