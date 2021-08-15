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
#include "locale.h"
#include "list.h"
#include "tracers.h"
#include "rbtree.h"
#include "blktrace.h"
#include "blkparse.h"
#include "blkparse_fmt.h"
#include "blktrace_api.h"
/******************************
全局变量定义
******************************/
static char line[1024];
#define MAX_DEVICES_PER_TRACE 64
static int line_len = 1024;
#define LC_NUMERIC        __LC_NUMERIC
#define is_done()	(*(volatile int *)(&done))
static volatile int done;

static char *blktrace_devices[MAX_DEVICES_PER_TRACE];
static char *blktrace_outfile = "trace";
static char *blktrace_dest_dir = ".";
static int num_blktrace_devices = 0;
static unsigned long read_sequence;

static char *output_name;

// static unsigned long long last_allowed_time;
// static unsigned int act_mask = -1U;

// // rbtree 
// #define RB_BATCH_DEFAULT	(512)
// static unsigned int rb_batch = RB_BATCH_DEFAULT;

static struct rb_root rb_sort_root;
// static unsigned long rb_sort_entries;

// // time setting
// static unsigned long long genesis_time;
// static unsigned long long last_allowed_time;
// static unsigned long long stopwatch_start;	/* 起始时间 */
// static unsigned long long stopwatch_end = -1ULL;	/* 终止时间(默认无限) */
// static unsigned long read_sequence;

// struct timespec		abs_start_time;
// static unsigned long long start_timestamp;

// // allocation cache
// static struct blk_io_trace *bit_alloc_list;
// static struct trace *t_alloc_list;

// static unsigned int t_alloc_cache;
// static unsigned int bit_alloc_cache;

// // dump output
// static FILE *dump_fp;
// static char *dump_binary;

// // trace struct
// struct trace {
// 	struct blk_io_trace *bit;
// 	struct rb_node rb_node;
// 	struct trace *next;
// 	unsigned long read_sequence;
// };
// static struct trace *trace_list;

// // dev info 
// static int ndevices;
// static struct per_dev_info *devices;

static int pipeline;
static char *pipename;

// static int have_drv_data = 0;

static int text_output = 1;

// static int bin_output_msgs = 1;

// int data_is_native = -1;

// FILE *ofp = NULL;

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


// static int is_pipe(const char *str)
// {
// 	struct stat st;

// 	if (!strcmp(str, "-"))
// 		return 1;
// 	if (!stat(str, &st) && S_ISFIFO(st.st_mode))
// 		return 1;

// 	return 0;
// }

// /******************************
// 其它函数
// ******************************/
// static inline void bit_free(struct blk_io_trace *bit)
// {
// 	/* 释放bit */
// 	if (bit_alloc_cache < 1024 && !bit->pdu_len) {
// 		/*
// 		 * abuse a 64-bit field for a next pointer for the free item
// 		 */
// 		bit->time = (__u64) (unsigned long) bit_alloc_list;
// 		bit_alloc_list = (struct blk_io_trace *) bit;
// 		bit_alloc_cache++;
// 	} else
// 		free(bit);
// }

// static inline struct blk_io_trace *bit_alloc(void)
// {
// 	/* 分配bit内存 */
// 	struct blk_io_trace *bit = bit_alloc_list;

// 	if (bit) {
// 		bit_alloc_list = (struct blk_io_trace *) (unsigned long) \
// 				 bit->time;
// 		bit_alloc_cache--;
// 		return bit;
// 	}

// 	return malloc(sizeof(*bit));
// }

// static int resize_devices(char *name)
// {
// 	int size = (ndevices + 1) * sizeof(struct per_dev_info);

// 	devices = realloc(devices, size);
// 	if (!devices) {
// 		fprintf(stderr, "Out of memory, device %s (%d)\n", name, size);
// 		return 1;
// 	}
// 	memset(&devices[ndevices], 0, sizeof(struct per_dev_info));
// 	devices[ndevices].name = name;
// 	ndevices++;
// 	return 0;
// }

// static int read_data(int fd, void *buffer, int bytes, int block, int *fdblock)
// {
// 	int ret, bytes_left, fl;
// 	void *p;

// 	if (block != *fdblock) {
// 		fl = fcntl(fd, F_GETFL);

// 		if (!block) {
// 			*fdblock = 0;
// 			fcntl(fd, F_SETFL, fl | O_NONBLOCK);
// 		} else {
// 			*fdblock = 1;
// 			fcntl(fd, F_SETFL, fl & ~O_NONBLOCK);
// 		}
// 	}

// 	bytes_left = bytes;
// 	p = buffer;
// 	while (bytes_left > 0) {
// 		ret = read(fd, p, bytes_left);
// 		if (!ret)
// 			return 1;
// 		else if (ret < 0) {
// 			if (errno != EAGAIN) {
// 				perror("read");
// 				return -1;
// 			}

// 			/*
// 			 * never do partial reads. we can return if we
// 			 * didn't read anything and we should not block,
// 			 * otherwise wait for data
// 			 */
// 			if ((bytes_left == bytes) && !block)
// 				return 1;

// 			usleep(10);
// 			continue;
// 		} else {
// 			p += ret;
// 			bytes_left -= ret;
// 		}
// 	}

// 	return 0;
// }

// static inline __u16 get_pdulen(struct blk_io_trace *bit)
// {
// 	if (data_is_native)
// 		return bit->pdu_len;

// 	return __bswap_16(bit->pdu_len);
// }

// static inline __u32 get_magic(struct blk_io_trace *bit)
// {
// 	if (data_is_native)
// 		return bit->magic;

// 	return __bswap_32(bit->magic);
// }

// static void output_binary(void *buf, int len)
// {
// 	/* 输出二进制文件 */
// 	if (dump_binary) {
// 		size_t n = fwrite(buf, len, 1, dump_fp);
// 		if (n != 1) {
// 			perror(dump_binary);
// 			fclose(dump_fp);
// 			dump_binary = NULL;
// 		}
// 	}
// }




// /******************************
// 管道符
// ******************************/
// static struct per_dev_info *get_dev_info(dev_t dev)
// {
// 	/* 获得设备信息 */
// 	struct per_dev_info *pdi;
// 	int i;

// 	for (i = 0; i < ndevices; i++) {
// 		if (!devices[i].dev)
// 			devices[i].dev = dev;
// 		if (devices[i].dev == dev)
// 			return &devices[i];
// 	}

// 	if (resize_devices(NULL))
// 		return NULL;

// 	pdi = &devices[ndevices - 1];
// 	pdi->dev = dev;
// 	pdi->first_reported_time = 0;
// 	pdi->last_read_time = 0;

// 	return pdi;
// }

// static char *get_dev_name(struct per_dev_info *pdi, char *buffer, int size)
// {
// 	if (pdi->name)
// 		snprintf(buffer, size, "%s", pdi->name);
// 	else
// 		snprintf(buffer, size, "%d,%d",MAJOR(pdi->dev),MINOR(pdi->dev));
// 	return buffer;
// }

// static void resize_cpu_info(struct per_dev_info *pdi, int cpu)
// {
// 	struct per_cpu_info *cpus = pdi->cpus;
// 	int ncpus = pdi->ncpus;
// 	int new_count = cpu + 1;
// 	int new_space, size;
// 	char *new_start;

// 	size = new_count * sizeof(struct per_cpu_info);
// 	cpus = realloc(cpus, size);
// 	if (!cpus) {
// 		char name[20];
// 		fprintf(stderr, "Out of memory, CPU info for device %s (%d)\n",
// 			get_dev_name(pdi, name, sizeof(name)), size);
// 		exit(1);
// 	}

// 	new_start = (char *)cpus + (ncpus * sizeof(struct per_cpu_info));
// 	new_space = (new_count - ncpus) * sizeof(struct per_cpu_info);
// 	memset(new_start, 0, new_space);

// 	pdi->ncpus = new_count;
// 	pdi->cpus = cpus;

// 	for (new_count = 0; new_count < pdi->ncpus; new_count++) {
// 		struct per_cpu_info *pci = &pdi->cpus[new_count];

// 		if (!pci->fd) {
// 			pci->fd = -1;
// 			memset(&pci->rb_last, 0, sizeof(pci->rb_last));
// 			pci->rb_last_entries = 0;
// 			pci->last_sequence = -1;
// 		}
// 	}
// }

// static struct per_cpu_info *get_cpu_info(struct per_dev_info *pdi, int cpu)
// {
// 	/* 获得CPU信息 */
// 	struct per_cpu_info *pci;

// 	if (cpu >= pdi->ncpus)
// 		resize_cpu_info(pdi, cpu);

// 	pci = &pdi->cpus[cpu];
// 	pci->cpu = cpu;
// 	return pci;
// }




// static inline int trace_rb_insert(struct trace *t, struct rb_root *root)
// {
// 	struct rb_node **p = &root->rb_node;
// 	struct rb_node *parent = NULL;
// 	struct trace *__t;

// 	while (*p) {
// 		parent = *p;

// 		__t = rb_entry(parent, struct trace, rb_node);

// 		if (t->bit->time < __t->bit->time)
// 			p = &(*p)->rb_left;
// 		else if (t->bit->time > __t->bit->time)
// 			p = &(*p)->rb_right;
// 		else if (t->bit->device < __t->bit->device)
// 			p = &(*p)->rb_left;
// 		else if (t->bit->device > __t->bit->device)
// 			p = &(*p)->rb_right;
// 		else if (t->bit->sequence < __t->bit->sequence)
// 			p = &(*p)->rb_left;
// 		else	/* >= sequence */
// 			p = &(*p)->rb_right;
// 	}

// 	rb_link_node(&t->rb_node, parent, p);
// 	rb_insert_color(&t->rb_node, root);
// 	return 0;
// }

// static inline int trace_rb_insert_sort(struct trace *t)
// {
// 	if (!trace_rb_insert(t, &rb_sort_root)) {
// 		rb_sort_entries++;
// 		return 0;
// 	}

// 	return 1;
// }






// static void check_time(struct per_dev_info *pdi, struct blk_io_trace *bit)
// {
// 	unsigned long long this = bit->time;
// 	unsigned long long last = pdi->last_reported_time;

// 	pdi->backwards = (this < last) ? 'B' : ' ';
// 	pdi->last_reported_time = this;
// }





// /*
//  * struct trace and blktrace allocation cache, we do potentially
//  * millions of mallocs for these structures while only using at most
//  * a few thousand at the time
//  */
// static inline void t_free(struct trace *t)
// {
// 	if (t_alloc_cache < 1024) {
// 		t->next = t_alloc_list;
// 		t_alloc_list = t;
// 		t_alloc_cache++;
// 	} else
// 		free(t);
// }

// static inline struct trace *t_alloc(void)
// {
// 	struct trace *t = t_alloc_list;

// 	if (t) {
// 		t_alloc_list = t->next;
// 		t_alloc_cache--;
// 		return t;
// 	}

// 	return malloc(sizeof(*t));
// }

// static inline void __put_trace_last(struct per_dev_info *pdi, struct trace *t)
// {
// 	struct per_cpu_info *pci = get_cpu_info(pdi, t->bit->cpu);

// 	rb_erase(&t->rb_node, &pci->rb_last);
// 	pci->rb_last_entries--;

// 	bit_free(t->bit);
// 	t_free(t);
// }

// static int trace_rb_insert_last(struct per_dev_info *pdi, struct trace *t)
// {
// 	struct per_cpu_info *pci = get_cpu_info(pdi, t->bit->cpu);

// 	if (trace_rb_insert(t, &pci->rb_last))
// 		return 1;

// 	pci->rb_last_entries++;

// 	if (pci->rb_last_entries > rb_batch * pdi->nfiles) {
// 		struct rb_node *n = rb_first(&pci->rb_last);

// 		t = rb_entry(n, struct trace, rb_node);
// 		__put_trace_last(pdi, t);
// 	}

// 	return 0;
// }

// static void put_trace(struct per_dev_info *pdi, struct trace *t)
// {
// 	rb_erase(&t->rb_node, &rb_sort_root);
// 	rb_sort_entries--;

// 	trace_rb_insert_last(pdi, t);
// }

// static void dump_trace_pc(struct blk_io_trace *t, struct per_dev_info *pdi,
// 			  struct per_cpu_info *pci)
// {
// 	int w = (t->action & BLK_TC_ACT(BLK_TC_WRITE)) != 0;
// 	int act = (t->action & 0xffff) & ~__BLK_TA_CGROUP;

// 	switch (act) {
// 		case __BLK_TA_QUEUE:
// 			log_generic(pci, t, "Q");
// 			account_pc_queue(t, pci, w);
// 			break;
// 		case __BLK_TA_GETRQ:
// 			log_generic(pci, t, "G");
// 			break;
// 		case __BLK_TA_SLEEPRQ:
// 			log_generic(pci, t, "S");
// 			break;
// 		case __BLK_TA_REQUEUE:
// 			/*
// 			 * can happen if we miss traces, don't let it go
// 			 * below zero
// 			 */
// 			if (pdi->cur_depth[w])
// 				pdi->cur_depth[w]--;
// 			account_pc_requeue(t, pci, w);
// 			log_generic(pci, t, "R");
// 			break;
// 		case __BLK_TA_ISSUE:
// 			account_pc_issue(t, pci, w);
// 			pdi->cur_depth[w]++;
// 			if (pdi->cur_depth[w] > pdi->max_depth[w])
// 				pdi->max_depth[w] = pdi->cur_depth[w];
// 			log_pc(pci, t, "D");
// 			break;
// 		case __BLK_TA_COMPLETE:
// 			if (pdi->cur_depth[w])
// 				pdi->cur_depth[w]--;
// 			log_pc(pci, t, "C");
// 			account_pc_c(t, pci, w);
// 			break;
// 		case __BLK_TA_INSERT:
// 			log_pc(pci, t, "I");
// 			break;
// 		default:
// 			fprintf(stderr, "Bad pc action %x\n", act);
// 			break;
// 	}
// }

// static void handle_notify(struct blk_io_trace *bit)
// {
// 	void	*payload = (caddr_t) bit + sizeof(*bit);
// 	__u32	two32[2];

// 	switch (bit->action & ~__BLK_TN_CGROUP) {
// 	case BLK_TN_PROCESS:
// 		add_ppm_hash(bit->pid, payload);
// 		break;

// 	case BLK_TN_TIMESTAMP:
// 		if (bit->pdu_len != sizeof(two32))
// 			return;
// 		memcpy(two32, payload, sizeof(two32));
// 		if (!data_is_native) {
// 			two32[0] = be32_to_cpu(two32[0]);
// 			two32[1] = be32_to_cpu(two32[1]);
// 		}
// 		start_timestamp = bit->time;
// 		abs_start_time.tv_sec  = two32[0];
// 		abs_start_time.tv_nsec = two32[1];
// 		if (abs_start_time.tv_nsec < 0) {
// 			abs_start_time.tv_sec--;
// 			abs_start_time.tv_nsec += 1000000000;
// 		}

// 		break;

// 	case BLK_TN_MESSAGE:
// 		if (bit->pdu_len > 0) {
// 			char msg[bit->pdu_len+1];
// 			int len = bit->pdu_len;
// 			char cgidstr[24];

// 			cgidstr[0] = 0;
// 			if (bit->action & __BLK_TN_CGROUP) {
// 				struct blk_io_cgroup_payload *cgid = payload;

// 				sprintf(cgidstr, "%x,%x ", cgid->ino,
// 					cgid->gen);
// 				payload += sizeof(struct blk_io_cgroup_payload);
// 				len -= sizeof(struct blk_io_cgroup_payload);
// 			}
// 			memcpy(msg, (char *)payload, len);
// 			msg[len] = '\0';

// 			fprintf(ofp,
// 				"%3d,%-3d %2d %8s %5d.%09lu %5u %s%2s %3s %s\n",
// 				MAJOR(bit->device), MINOR(bit->device),
// 				bit->cpu, "0", (int)SECONDS(bit->time),
// 				(unsigned long)NANO_SECONDS(bit->time),
// 				bit->pid, cgidstr, "m", "N", msg);
// 		}
// 		break;

// 	default:
// 		/* Ignore unknown notify events */
// 		;
// 	}
// }

// static void dump_trace_fs(struct blk_io_trace *t, struct per_dev_info *pdi,
// 			  struct per_cpu_info *pci)
// {
// 	int w = (t->action & BLK_TC_ACT(BLK_TC_WRITE)) != 0;
// 	int act = (t->action & 0xffff) & ~__BLK_TA_CGROUP;

// 	switch (act) {
// 		case __BLK_TA_QUEUE:
// 			// log_track_queue(pdi, t);
// 			account_queue(t, pci, w);
// 			log_queue(pci, t, "Q");
// 			break;
// 		case __BLK_TA_INSERT:
// 			log_insert(pdi, pci, t, "I");
// 			break;
// 		case __BLK_TA_BACKMERGE:
// 			account_m(t, pci, w);
// 			log_merge(pdi, pci, t, "M");
// 			break;
// 		case __BLK_TA_FRONTMERGE:
// 			account_m(t, pci, w);
// 			log_merge(pdi, pci, t, "F");
// 			break;
// 		case __BLK_TA_GETRQ:
// 			log_track_getrq(pdi, t);
// 			log_generic(pci, t, "G");
// 			break;
// 		case __BLK_TA_SLEEPRQ:
// 			log_generic(pci, t, "S");
// 			break;
// 		case __BLK_TA_REQUEUE:
// 			/*
// 			 * can happen if we miss traces, don't let it go
// 			 * below zero
// 			 */
// 			if (pdi->cur_depth[w])
// 				pdi->cur_depth[w]--;
// 			account_requeue(t, pci, w);
// 			log_queue(pci, t, "R");
// 			break;
// 		case __BLK_TA_ISSUE:
// 			account_issue(t, pci, w);
// 			pdi->cur_depth[w]++;
// 			if (pdi->cur_depth[w] > pdi->max_depth[w])
// 				pdi->max_depth[w] = pdi->cur_depth[w];
// 			log_issue(pdi, pci, t, "D");
// 			break;
// 		case __BLK_TA_COMPLETE:
// 			if (pdi->cur_depth[w])
// 				pdi->cur_depth[w]--;
// 			fixup_complete(pdi, t);
// 			account_c(t, pci, w, t->bytes);
// 			log_complete(pdi, pci, t, "C");
// 			break;
// 		case __BLK_TA_PLUG:
// 			log_action(pci, t, "P");
// 			break;
// 		case __BLK_TA_UNPLUG_IO:
// 			account_unplug(t, pci, 0);
// 			log_unplug(pci, t, "U");
// 			break;
// 		case __BLK_TA_UNPLUG_TIMER:
// 			account_unplug(t, pci, 1);
// 			log_unplug(pci, t, "UT");
// 			break;
// 		case __BLK_TA_SPLIT:
// 			log_track_split(pdi, t);
// 			log_split(pci, t, "X");
// 			break;
// 		case __BLK_TA_BOUNCE:
// 			log_generic(pci, t, "B");
// 			break;
// 		case __BLK_TA_REMAP:
// 			log_generic(pci, t, "A");
// 			break;
// 		case __BLK_TA_DRV_DATA:
// 			have_drv_data = 1;
// 			/* dump to binary file only */
// 			break;
// 		default:
// 			fprintf(stderr, "Bad fs action %x\n", t->action);
// 			break;
// 	}
// }

// static void dump_trace(struct blk_io_trace *t, struct per_cpu_info *pci,
// 		       struct per_dev_info *pdi)
// {
// 	if (text_output) {
// 		if ((t->action & ~__BLK_TN_CGROUP) == BLK_TN_MESSAGE)
// 			handle_notify(t);
// 		else if (t->action & BLK_TC_ACT(BLK_TC_PC))
// 			dump_trace_pc(t, pdi, pci);
// 		else
// 			dump_trace_fs(t, pdi, pci);
// 	}

// 	if (!pdi->events)
// 		pdi->first_reported_time = t->time;

// 	pdi->events++;

// 	if (bin_output_msgs ||
// 			    !(t->action & BLK_TC_ACT(BLK_TC_NOTIFY) &&
// 			      (t->action & ~__BLK_TN_CGROUP) == BLK_TN_MESSAGE))
// 		output_binary(t, sizeof(*t) + t->pdu_len);
// }


// static int check_sequence(struct per_dev_info *pdi, struct trace *t, int force)
// {
// 	struct blk_io_trace *bit = t->bit;
// 	unsigned long expected_sequence;
// 	struct per_cpu_info *pci;
// 	struct trace *__t;

// 	pci = get_cpu_info(pdi, bit->cpu);
// 	expected_sequence = pci->last_sequence + 1;

// 	if (!expected_sequence) {
// 		/*
// 		 * 1 should be the first entry, just allow it
// 		 */
// 		if (bit->sequence == 1)
// 			return 0;
// 		if (bit->sequence == pci->smallest_seq_read)
// 			return 0;

// 		return check_cpu_map(pdi);
// 	}

// 	if (bit->sequence == expected_sequence)
// 		return 0;

// 	/*
// 	 * we may not have seen that sequence yet. if we are not doing
// 	 * the final run, break and wait for more entries.
// 	 */
// 	if (expected_sequence < pci->smallest_seq_read) {
// 		__t = trace_rb_find_last(pdi, pci, expected_sequence);
// 		if (!__t)
// 			goto skip;

// 		__put_trace_last(pdi, __t);
// 		return 0;
// 	} else if (!force) {
// 		return 1;
// 	} else {
// skip:
// 		if (check_current_skips(pci, bit->sequence))
// 			return 0;

// 		if (expected_sequence < bit->sequence)
// 			insert_skip(pci, expected_sequence, bit->sequence - 1);
// 		return 0;
// 	}
// }

// static void show_entries_rb(int force)
// {
// 	struct per_dev_info *pdi = NULL;
// 	struct per_cpu_info *pci = NULL;
// 	struct blk_io_trace *bit;
// 	struct rb_node *n;
// 	struct trace *t;

// 	while ((n = rb_first(&rb_sort_root)) != NULL) { 
// 		if (is_done() && !force && !pipeline)
// 			break;

// 		t = rb_entry(n, struct trace, rb_node);
// 		bit = t->bit;

// 		if (read_sequence - t->read_sequence < 1 && !force)
// 			break;

// 		if (!pdi || pdi->dev != bit->device) {
// 			pdi = get_dev_info(bit->device);
// 			pci = NULL;
// 		}

// 		if (!pdi) {
// 			fprintf(stderr, "Unknown device ID? (%d,%d)\n",
// 				MAJOR(bit->device), MINOR(bit->device));
// 			break;
// 		}

// 		if (!((bit->action & ~__BLK_TN_CGROUP) == BLK_TN_MESSAGE) &&
// 		    check_sequence(pdi, t, force))
// 			break;

// 		if (!force && bit->time > last_allowed_time)
// 			break;

// 		check_time(pdi, bit);

// 		if (!pci || pci->cpu != bit->cpu)
// 			pci = get_cpu_info(pdi, bit->cpu);

// 		if (!((bit->action & ~__BLK_TN_CGROUP) == BLK_TN_MESSAGE))
// 			pci->last_sequence = bit->sequence;

// 		pci->nelems++;

// 		if (bit->action & (act_mask << BLK_TC_SHIFT))
// 			dump_trace(bit, pci, pdi);

// 		put_trace(pdi, t);
// 	}
// }



// static void correct_abs_start_time(void)
// {
// 	/* 更新最初时间 */
// 	long delta = genesis_time - start_timestamp;

// 	abs_start_time.tv_sec  += SECONDS(delta);
// 	abs_start_time.tv_nsec += NANO_SECONDS(delta);
// 	if (abs_start_time.tv_nsec < 0) {
// 		abs_start_time.tv_nsec += 1000000000;
// 		abs_start_time.tv_sec -= 1;
// 	} else
// 	if (abs_start_time.tv_nsec > 1000000000) {
// 		abs_start_time.tv_nsec -= 1000000000;
// 		abs_start_time.tv_sec += 1;
// 	}
// }

// static inline int check_stopwatch(struct blk_io_trace *bit)
// {
// 	/* 根据时间判断是否停止监听 */
// 	if (bit->time < stopwatch_end &&
// 	    bit->time >= stopwatch_start)
// 		return 0;

// 	return 1;
// }


// static void find_genesis(void)
// {
// 	/* 找到最初的记录 */
// 	struct trace *t = trace_list;

// 	genesis_time = -1ULL;
// 	while (t != NULL) {
// 		if (t->bit->time < genesis_time)
// 			genesis_time = t->bit->time;

// 		t = t->next;
// 	}

// 	/* The time stamp record will usually be the first
// 	 * record in the trace, but not always.
// 	 */
// 	if (start_timestamp
// 	 && start_timestamp != genesis_time) {
// 		correct_abs_start_time();
// 	}
// }

// static int sort_entries(unsigned long long *youngest)
// {
// 	struct per_dev_info *pdi = NULL;
// 	struct per_cpu_info *pci = NULL;
// 	struct trace *t;

// 	if (!genesis_time)
// 		find_genesis();

// 	*youngest = 0;
// 	while ((t = trace_list) != NULL) {
// 		struct blk_io_trace *bit = t->bit;

// 		trace_list = t->next;

// 		bit->time -= genesis_time;

// 		if (bit->time < *youngest || !*youngest)
// 			*youngest = bit->time;

// 		if (!pdi || pdi->dev != bit->device) {
// 			pdi = get_dev_info(bit->device);
// 			pci = NULL;
// 		}

// 		if (!pci || pci->cpu != bit->cpu)
// 			pci = get_cpu_info(pdi, bit->cpu);

// 		if (bit->sequence < pci->smallest_seq_read)
// 			pci->smallest_seq_read = bit->sequence;

// 		if (check_stopwatch(bit)) {
// 			bit_free(bit);
// 			t_free(t);
// 			continue;
// 		}

// 		if (trace_rb_insert_sort(t))
// 			return -1;
// 	}

// 	return 0;
// }

// static int read_events(int fd, int always_block, int *fdblock)
// {
// 	struct per_dev_info *pdi = NULL;
// 	unsigned int events = 0;

// 	while (!is_done() && events < rb_batch) {
// 		struct blk_io_trace *bit;
// 		struct trace *t;
// 		int pdu_len, should_block, ret;
// 		__u32 magic;

// 		bit = bit_alloc();

// 		should_block = !events || always_block;

// 		ret = read_data(fd, bit, sizeof(*bit), should_block, fdblock);
// 		if (ret) {
// 			bit_free(bit);
// 			if (!events && ret < 0)
// 				events = ret;
// 			break;
// 		}

// 		/*
// 		 * look at first trace to check whether we need to convert
// 		 * data in the future
// 		 */
// 		if (data_is_native == -1 && check_data_endianness(bit->magic))
// 			break;

// 		magic = get_magic(bit);
// 		if ((magic & 0xffffff00) != BLK_IO_TRACE_MAGIC) {
// 			fprintf(stderr, "Bad magic %x\n", magic);
// 			break;
// 		}

// 		pdu_len = get_pdulen(bit);
// 		if (pdu_len) {
// 			void *ptr = realloc(bit, sizeof(*bit) + pdu_len);

// 			if (read_data(fd, ptr + sizeof(*bit), pdu_len, 1, fdblock)) {
// 				bit_free(ptr);
// 				break;
// 			}

// 			bit = ptr;
// 		}

// 		trace_to_cpu(bit);

// 		if (verify_trace(bit)) {
// 			bit_free(bit);
// 			continue;
// 		}

// 		/*
// 		 * not a real trace, so grab and handle it here
// 		 */
// 		if (bit->action & BLK_TC_ACT(BLK_TC_NOTIFY) && (bit->action & ~__BLK_TN_CGROUP) != BLK_TN_MESSAGE) {
// 			handle_notify(bit);
// 			output_binary(bit, sizeof(*bit) + bit->pdu_len);
// 			continue;
// 		}

// 		t = t_alloc();
// 		memset(t, 0, sizeof(*t));
// 		t->bit = bit;
// 		t->read_sequence = read_sequence;

// 		t->next = trace_list;
// 		trace_list = t;

// 		if (!pdi || pdi->dev != bit->device)
// 			pdi = get_dev_info(bit->device);

// 		if (bit->time > pdi->last_read_time)
// 			pdi->last_read_time = bit->time;

// 		events++;
// 	}

// 	return events;
// }

// static void do_pipe(int fd)
// {
// 	unsigned long long youngest;
// 	int events, fdblock;

// 	last_allowed_time = -1ULL;
// 	fdblock = -1;
// 	while ((events = read_events(fd, 0, &fdblock)) > 0) {
// 		read_sequence++;

// 		if (sort_entries(&youngest))
// 			break;
		
// 		if (youngest > stopwatch_end)
// 			break;
		
// 		show_entries_rb(0);
// 	}

// 	if (rb_sort_entries)
// 		show_entries_rb(1);

// }

// static int do_fifo(void)
// {
// 	int fd;

// 	if (!strcmp(pipename, "-"))
// 		fd = dup(STDIN_FILENO);
// 	else
// 		fd = open(pipename, O_RDONLY);

// 	if (fd == -1) {
// 		perror("dup stdin");
// 		return -1;
// 	}

// 	do_pipe(fd);
// 	close(fd);
// 	return 0;
// }


/******************************
参数解析函数
******************************/
char *option_string = "d:D:i:";
static struct option long_options[] = {
	{"device", required_argument, 0, 'd'},
	{"blktrace-destination", required_argument, 0, 'D'},
	{"input", required_argument, 0, 'i'},
	{0, 0, 0, 0}
};

static void print_usage(void)
{
	fprintf(stderr, "iowatcher usage:\n"
		"\t-i <file>           | --input=<file>\n" \
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
		case 'i':
			if (is_pipe(optarg) && !pipeline) {
				pipeline = 1;
				pipename = strdup(optarg);
			} else if (resize_devices(optarg) != 0)
				return 1;
			break;
		case '?':
		case HELP_LONG_OPT:
			print_usage();
			break;
		default:
			break;
		}

		while (option_index < ac) {
		if (is_pipe(av[option_index]) && !pipeline) {
			pipeline = 1;
			pipename = strdup(av[option_index]);
		} else if (resize_devices(av[option_index]) != 0)
			return 1;
		option_index++;
	}
	}
	
	return 0;
}



/******************************
主函数
******************************/
int main(int ac, char **av)
{
	int ret, mode;
	char *ofp_buffer = NULL;
	parse_options(ac, av);

	// if (num_blktrace_devices) {
	// 	char *path = join_path(blktrace_dest_dir, blktrace_outfile);
	// 	dest_mkdir(blktrace_dest_dir);
	// 	dest_mkdir(path);

	// 	snprintf(line, line_len, "%s.dump", path); 
	// 	unlink(line);

	// 	ret = start_blktrace(blktrace_devices, num_blktrace_devices,
	// 			     blktrace_outfile, blktrace_dest_dir);
	// 	if (ret) {
	// 		perror("Exiting due to blktrace failure"); // 把一个描述性错误消息输出到标准错误 stderr。
	// 		exit(ret);
	// 	}

	// 	printf("Tracing until interrupted...\n");
	// 	wait_for_tracers(0);
		
	// 	free(path);
	// }
	setlocale(LC_NUMERIC, "en_US");

	memset(&rb_sort_root, 0, sizeof(rb_sort_root));

	signal(SIGINT, handle_sigint);
	signal(SIGHUP, handle_sigint);
	signal(SIGTERM, handle_sigint);

	if (text_output) {
		if (!output_name) {
			ofp = fdopen(STDOUT_FILENO, "w");
			mode = _IOLBF;
		} else {
			char ofname[PATH_MAX];

			snprintf(ofname, sizeof(ofname) - 1, "%s", output_name);
			ofp = fopen(ofname, "w");
			mode = _IOFBF;
		}

		if (!ofp) {
			perror("fopen");
			return 1;
		}

		ofp_buffer = malloc(4096);
		if (setvbuf(ofp, ofp_buffer, mode, 4096)) {
			perror("setvbuf");
			return 1;
		}
	}

	// if (pipeline)
	ret = do_fifo();

	if (!ret)
		show_stats();

	return 0;
}



