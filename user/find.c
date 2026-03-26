/*
Write a simple version of the UNIX find program:
find all the files in a directory tree with a specific name.

Your solution should be in the file user/find.c.

Some hints:

Look at user/ls.c to see how to read directories.
Use recursion to allow find to descend into sub-directories.
Don't recurse into "." and "..".
Changes to the file system persist across runs of qemu;
to get a clean file system run make clean and then make qemu.
You'll need to use C strings. Have a look at K&R (the C book), for example
Section 5.5. Note that == does not compare strings like in Python. Use strcmp()
instead. Add the program to UPROGS in Makefile.

Your solution is correct if produces the following output (when the file system
contains the files b and a/b):*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 返回路径中的文件名，不填充空格，保证 null 结尾
char *fmtname(char *path)
{
	static char buf[DIRSIZ + 1];
	char *p;

	// 找最后一个 '/'
	for (p = path + strlen(path); p >= path && *p != '/'; p--)
		;
	p++;

	if (strlen(p) >= DIRSIZ)
		return p;

	memmove(buf, p, strlen(p));
	buf[strlen(p)] = '\0'; // null 结尾
	return buf;
}

void find(char *word, char *path)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	// 打开目录
	if ((fd = open(path, 0)) < 0) {
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}

	switch (st.type) {
	case T_FILE:
		// 匹配文件名
		if (strcmp(fmtname(path), word) == 0) {
			printf("%s %d %d %d\n", path, st.type, st.ino, st.size);
		}
		break;

	case T_DIR:
		// 确保路径长度不溢出
		if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
			printf("find: path too long %s\n", path);
			break;
		}

		strcpy(buf, path);
		p = buf + strlen(buf);
		*p++ = '/';

		while (read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (de.inum == 0)
				continue;

			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0; // null 结尾

			// 跳过 "." 和 ".."
			if (strcmp(de.name, ".") == 0 ||
			    strcmp(de.name, "..") == 0)
				continue;

			// stat 子目录/文件
			if (stat(buf, &st) < 0) {
				printf("find: cannot stat %s\n", buf);
				continue;
			}

			// 递归查找
			find(word, buf);
		}
		break;
	}

	close(fd);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(2, "Usage: find [dir] <name>\n");
		exit(1);
	}

	if (argc == 2) {
		// 从当前目录开始查找
		find(argv[1], ".");
	} else if (argc == 3) {
		// 指定目录查找
		find(argv[2], argv[1]);
	}

	exit(0);
}
