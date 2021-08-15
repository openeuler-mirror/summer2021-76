#include <stdio.h>
#include <limits.h>
#include <byteswap.h>
#include <endian.h>
#include <sys/types.h>



extern void set_all_format_specs(char *);
extern int add_format_spec(char *);
extern void process_fmt(char *, struct per_cpu_info *, struct blk_io_trace *,
			unsigned long long, int, unsigned char *);
extern int valid_act_opt(int);
extern int find_mask_map(char *);
extern char *find_process_name(pid_t);