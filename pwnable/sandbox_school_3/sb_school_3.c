// gcc sb_school_3.c -o sb3
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <linux/prctl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>

#include "seccomp-bpf.h"

static int install_syscall_filter(void)
{
	struct sock_filter filter[] = {
		VALIDATE_ARCHITECTURE,
		EXAMINE_SYSCALL,
		ALLOW_SYSCALL(open),
		ALLOW_SYSCALL(read),
		ALLOW_SYSCALL(write),
		ALLOW_SYSCALL(exit),
		KILL_PROCESS
	};
	struct sock_fprog prog = {
		.len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
		.filter = filter,
	};

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
		perror("prctl(NO_NEW_PRIVS)");
		goto failed;
	}
	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
		perror("prctl(SECCOMP)");
		goto failed;
	}
	return 0;

failed:
	if (errno == EINVAL)
		fprintf(stderr, "SECCOMP_FILTER is not available. :(\n");
	return 1;
}

int main()
{
	void (*sc)();
	unsigned char *shellcode;

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

	shellcode = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (shellcode == MAP_FAILED) {
		printf("[err] Please, let me know this issue (hackability@naver.com)\n");
		return -1;
	}

	printf("[*] Welcome to sandbox school for beginner!\n");
	printf("[*] Put your shellcode as binary stream. I'll ready for your input as read(0, shellcode, 1024)\n");
	printf("[*] Lv   : Orc\n");
	printf("[*] Desc : You can't go futher.\n");
	printf("> ");

	alarm(10);

	read(0, shellcode, 1024);

	for(int i=0 ; i<1024-1; i++) {
		if (shellcode[i] == 0x0f && shellcode[i+1] == 0x05) {
			printf("[*] blocked !\n");
			return -1;
		}
	}

	install_syscall_filter();

	sc = (void *)shellcode;
	sc();	
	
	return 0;
}
