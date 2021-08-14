#include <stdio.h>
#include <errno.h>

/******************************
其他函数
******************************/
static void dest_mkdir(char *dir)
{
	// 创建文件夹
	int ret;
	ret = mkdir(dir, 0777);
	if (ret && errno != EEXIST) {
		fprintf(stderr, "failed to mkdir error %s\n", strerror(errno));
		exit(errno);
	}
}

static char * join_path(char *dest_dir, char *filename)
{
	/* alloc 2 extra bytes for '/' and '\0' */
	char *path = malloc(strlen(dest_dir) + strlen(filename) + 2);
	sprintf(path, "%s%s%s", dest_dir, "/", filename);

	return path;
}