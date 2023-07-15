#include "test_support.h"

#include <stdio.h>

void test_mu_macros(void);
void test_mu_mqueue(void);
void test_mu_sched(void);
void test_mu_spsc(void);
void test_mu_str(void);
void test_mu_task(void);
void test_mu_time(void);
void test_mu_timer(void);

void test_mulib_core(void) {
	printf("\nStarting test_mulib_core...");
	test_mu_macros();
	test_mu_mqueue();
	test_mu_sched();
	test_mu_spsc();
	test_mu_str();
	test_mu_task();
	test_mu_time();
	test_mu_timer();
	printf("\nCompleted test_mulib_core\n");
}

int main(void) {
	test_mulib_core();
	return 0;
}

