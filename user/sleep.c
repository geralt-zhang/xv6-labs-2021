/*sleep (easy)

Implement the UNIX program sleep for xv6;
your sleep should pause for a user-specified number of ticks.
A tick is a notion of time defined by the xv6 kernel,
namely the time between two interrupts from the timer chip.
Your solution should be in the file user/sleep.c.

Some hints:

    Before you start coding, read Chapter 1 of the xv6 book.
    Look at some of the other programs in user/ (
    e.g., user/echo.c, user/grep.c, and user/rm.c)
    to see how you can obtain the command-line arguments passed to a program.

    If the user forgets to pass an argument, sleep should print an error
    message. The command-line argument is passed as a string;
    you can convert it  to an integer using atoi (see user/ulib.c).
    Use the system call sleep.

    See kernel/sysproc.c for the xv6 kernel code
    that implements the sleep system call (look for sys_sleep),
    user/user.h for the C definition of sleep callable from a user program,
    and user/usys.S for the assembler code that jumps from user code into the
kernel for sleep. Make sure main calls exit() in order to exit your program.

    Add your sleep program to UPROGS in Makefile;
    once you've done that, make qemu will compile your program
    and you'll be able to run it from the xv6 shell.

    Look at Kernighan and Ritchie's book
    The C programming language (second edition) (K&R) to learn about C.

Run the program from the xv6 shell:

      $ make qemu
      ...
      init: starting sh
      $ sleep 10
      (nothing happens for a little while)
      $


Your solution is correct if your program pauses when run as shown above.
Run make grade to see if you indeed pass the sleep tests.

Note that make grade runs all tests, including the ones for the assignments
below. If you want to run the grade tests for one assignment, type:

     $ ./grade-lab-util sleep


This will run the grade tests that match "sleep". Or, you can type:

     $ make GRADEFLAGS=sleep grade


which does the same. t

Q: how sleep impl? 
A: call sys_sleep()，把当前进程挂到 chan 上状态设为 SLEEPING
*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(2, "Usage: sleep <ticks>\n");
		exit(1);
	}

    int time = atoi(argv[1]);

	if (time < 0) {
		fprintf(2, "Error: <ticks> should > 0\n");
		exit(1);
	}

	sleep(time);

	exit(0);
}
