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

// 获取路径的文件名
char *fmtname(char *path)
{
	static char buf[DIRSIZ + 1];
	char *p;

	// 从路径末尾开始寻找最后一个 '/' 的位置
	for (p = path + strlen(path); p >= path && *p != '/'; p--)
		;
	p++; // 跳过 '/'

	// 如果文件名过长，则返回文件名本身
	if (strlen(p) >= DIRSIZ)
		return p;

	// 填充空格以保证返回的文件名长度一致
	memmove(buf, p, strlen(p));
	memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
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

	// 根据文件类型处理
	switch (st.type) {
	case T_FILE:
		// 匹配文件名
		if (strcmp(fmtname(path), word) == 0) {
			// 找到匹配的文件
			printf("%s %d %d %l\n", fmtname(path), st.type, st.ino,
			       st.size);
		}
		break;

	case T_DIR:
		// 确保路径不会太长
		if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
			printf("find: path too long for directory %s\n", path);
			break;
		}

		// 拼接新的路径
		strcpy(buf, path);
		p = buf + strlen(buf);
		*p++ = '/';

		// 读取目录项
		while (read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (de.inum == 0)
				continue;

			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;

			// 跳过当前目录和父目录
			if (strcmp(de.name, ".") == 0 ||
			    strcmp(de.name, "..") == 0)
				continue;

			// 获取目录项的状态
			if (stat(buf, &st) < 0) {
				printf("find: cannot stat %s\n", buf);
				continue;
			}

			// 递归查找子目录
			find(word, buf);
		}
		break;
	}

	close(fd);
}

int main(int argc, char *argv[])
{
	if (argc == 2) {
		// 如果只有一个参数，从当前目录开始查找
		find(argv[1], ".");
		exit(0);
	}

	// 如果有两个参数，按照指定的目录查找
	find(argv[2], argv[1]);
	exit(0);
}
