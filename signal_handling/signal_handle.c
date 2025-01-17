#include "signal_handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>


// Implementation - link_signal(SIGINT, sigint_handler, true);


extern volatile sig_atomic_t shutdown_flag;

void sigint_handler(int sig)
{
	(void)sig;
	shutdown_flag = 1;
}

void link_signal(int sig, signal_f handler, bool flag)
{
	struct sigaction sa;
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	if (flag == false) {
		sa.sa_flags = 0;
	}

	sigemptyset(&sa.sa_mask);

	if (sigaction(sig, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
}