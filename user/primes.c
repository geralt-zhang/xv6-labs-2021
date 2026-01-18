/*
Be careful to close file descriptors that a process doesn't need,
because otherwise your program will run xv6 out of resources
before the first process reaches 35.

Once the first process reaches 35,
it should wait until the entire pipeline terminates,
including all children, grandchildren, &c.

Thus the main primes process should
only exit after all the output has been printed,
and after all the other primes processes have exited.

Hint: read returns zero when the write-side of a pipe is closed.
It's simplest to directly write 32-bit (4-byte) ints to the pipes,
rather than using formatted ASCII I/O.

You should create the processes in the pipeline only as they are needed.
Add the program to UPROGS in Makefile.

Your solution is correct if it implements a pipe-based sieve
and produces the following output:

  $ make qemu
  ...
  init: starting sh
  $ primes
  prime 2
  prime 3
  prime 5
  prime 7
  prime 11
  prime 13
  prime 17
  prime 19
  prime 23
  prime 29
  prime 31
  $

  pid1：2~35 -- pid2:(2~35/2) -- pid3：(2~35/2/3)...

  父进程（parent）                          子进程（child）
────────────────────── fork ──────────────────────▶

父进程文件描述符表                子进程文件描述符表
┌──────────────┐              ┌──────────────┐
│ fd=0 stdin   │──┐           │ fd=0 stdin   │──┐
│ fd=1 stdout  │──┤           │ fd=1 stdout  │──┤
│ fd=2 stderr  │──┤           │ fd=2 stderr  │──┤
│ fd=3 = p[0]──┼────────────┐ │ fd=3 = p[0]──┼────────────┐
│ fd=4 = p[1]──┼────────────┘ │ fd=4 = p[1]──┼────────────┘
└──────────────┘              └──────────────┘
    │                                │
    ▼                                ▼
  ┌────────────────────────────────────┐
  │        pipe 对象（内核）             │
  │────────────────────────────────────│
  │ [读端] <── 数据缓冲区 ─── [写端]      │
  │  ^                       ^         │
  │  │                       │         │
  │  │ (p[0])                │ (p[1])  │
  └────────────────────────────────────┘

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve(int left[2])
{
	close(left[1]); // 这一层不需要写端，只读

	int prime;
	// 读取本层第一个数字（一定是质数）
	if (read(left[0], &prime, sizeof(prime)) == 0) {
		// 如果读不到，说明没有数字了
		close(left[0]);
		exit(0);
	}

	printf("prime %d\n", prime); // 输出本层质数

	// 为下一层准备管道
	int right[2];
	pipe(right);

	int pid = fork();
	if (pid == 0) {
		// 子进程 → 下一层筛选
		close(right[1]); // 只读
		sieve(right);
		exit(0);
	}

	// 父进程 → 过滤不能被当前 prime 整除的数，传给右边
	close(right[0]); // 只写

	int num;
	while (read(left[0], &num, sizeof(num)) != 0) {
		if (num % prime != 0) {
			write(right[1], &num, sizeof(num));
		}
	}

	// 读完，关闭管道，收尸子进程
	close(left[0]);
	close(right[1]);
	wait(0);
	exit(0);
}

int main()
{
	int p[2];
	pipe(p);

	if (fork() == 0) {
		sieve(p);
		exit(0);
	}

	// 主进程：写入 2~35
	close(p[0]);
	for (int i = 2; i <= 35; i++) {
		write(p[1], &i, sizeof(i));
	}
	close(p[1]);

	wait(0); // 等待整个筛选结束
	exit(0);
}
