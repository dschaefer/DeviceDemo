#if defined(__QNXNTO__)

#include "DeviceDemo.h"
#include <pthread.h>
#include <time.h>

int main(int argc, char **argv) {
	demo_start();
	while (true) {
		sleep(1);
	}
	return 0;
}

uint32_t system_get_time() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1000000 + time.tv_usec;
}

void mqtt_start(task mqtt_task) {
	pthread_create(NULL, NULL, mqtt_task, NULL);
}

void range_finder_start(task range_finder_task) {
	pthread_t thread;
	pthread_create(&thread, NULL, range_finder_task, NULL);
	pthread_setschedprio(thread, 240);
}

#endif
