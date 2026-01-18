/*
 * HINTS:
 * - Use pipe to create a pipe.
 * - Use fork to create a child.
 * - Use read to read from the pipe, and write to write to the pipe.
 * - Use getpid to find the process ID of the calling process.
 * - Add the program to UPROGS in Makefile.
 * - User programs on xv6 have a limited set of
 * 	 library functions available to them.
 * - You can see the list in user/user.h; the source (other than for system
 * 	 calls) is in user/ulib.c, user/printf.c, and user/umalloc.c.
 *
 * EXPECTED OUTPUT:
 * $ make qemu
 * ...
 * init: starting sh
 * $ pingpong
 * 4: received ping
 * 3: received pong
 * $
 *
 * SOLUTION VERIFICATION:
 * Your solution is correct if your program exchanges a byte between two
 * processes and produces output as shown above.
 *
 * PIPE DIAGRAM:
 * write(fd[1]) ---> [ kernel buffer ] ---> read(fd[0])
 *
 *   parent                 child
 *     |                     |
 *     | write(p1[1])        | read(p1[0])
 *     |-------- p1 -------->|
 *     | read(p2[0])         | write(p2[1])
 *     |<------- p2 ---------|
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void)
{
	int p1[2], p2[2]; // Two pipes: parent<->child communication
	pipe(p1);
	pipe(p2);

	char buf[2];
	if (fork() == 0) {
		// Child process
		close(p1[1]); // Close write end (not using)
		close(p2[0]); // Close read end (not using)

		read(p1[0], buf, 1);
		fprintf(1, "%d: received ping\n", getpid());
		write(p2[1], "!", 1);

		close(p1[0]);
		close(p2[1]);
		exit(0);
	} else {
		// Parent process
		close(p1[0]); // Close read end (not using)
		close(p2[1]); // Close write end (not using)

		write(p1[1], "!", 1);
		read(p2[0], buf, 1);
		fprintf(1, "%d: received pong\n", getpid());

		close(p1[1]);
		close(p2[0]);
		exit(0);
	}
}
