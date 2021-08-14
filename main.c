#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include <asm/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <limits.h>
#include <float.h>
#include <signal.h>

// #include "plot.h"
#include "blkparse.h"
// #include "misc.h"
#include "list.h"
#include "tracers.h"
// #include "mpstat.h"
// #include "fio.h"

/******************************
全局变量定义
******************************/
static char line[1024];
static int line_len = 1024;
static char *blktrace_devices[MAX_DEVICES_PER_TRACE];
static char *blktrace_outfile = "trace";
static char *blktrace_dest_dir = ".";
static int num_blktrace_devices = 0;


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


enum {
	HELP_LONG_OPT = 1,
};


/******************************
参数解析函数
******************************/
char *option_string = "d:D:";
static struct option long_options[] = {
	{"device", required_argument, 0, 'd'},
	{"blktrace-destination", required_argument, 0, 'D'},
	{0, 0, 0, 0}
};

static void print_usage(void)
{
	fprintf(stderr, "iowatcher usage:\n"
		"\t-d (--device): device for blktrace to trace\n"
		"\t-D (--blktrace-destination): destination for blktrace\n"
	       );
	exit(1);
}

static int parse_options(int ac, char **av)
{
	int c;

	while (1) {
		int option_index = 0;

		c = getopt_long(ac, av, option_string,
				long_options, &option_index);

		if (c == -1)
			break;

		switch(c) {
		case 'd':
			if (num_blktrace_devices == MAX_DEVICES_PER_TRACE - 1) {
				fprintf(stderr, "Too many blktrace devices provided\n");
				exit(1);
			}
			blktrace_devices[num_blktrace_devices++] = strdup(optarg);
			break;
		case 'D':
			blktrace_dest_dir = strdup(optarg);
			if (!strcmp(blktrace_dest_dir, "")) {
				fprintf(stderr, "Need a directory\n");
				print_usage();
			}
			break;
		case '?':
		case HELP_LONG_OPT:
			print_usage();
			break;
		default:
			break;
		}
	}

	return 0;
}



/******************************
主函数
******************************/
int main(int ac, char **av)
{
	int ret;

	parse_options(ac, av);

	if (num_blktrace_devices) {
		char *path = join_path(blktrace_dest_dir, blktrace_outfile);
		dest_mkdir(blktrace_dest_dir);
		dest_mkdir(path);

		snprintf(line, line_len, "%s.dump", path); 
		unlink(line);

		ret = start_blktrace(blktrace_devices, num_blktrace_devices,
				     blktrace_outfile, blktrace_dest_dir);
		if (ret) {
			perror("Exiting due to blktrace failure"); // 把一个描述性错误消息输出到标准错误 stderr。
			exit(ret);
		}

		printf("Tracing until interrupted...\n");
		wait_for_tracers(0);
		
		free(path);
	}

	
	return 0;
}



