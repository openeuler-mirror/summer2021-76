
#ifndef BLKPARSE_H
#define BLKPARSE_H

int is_pipe(const char *str);
int resize_devices(char *name);
int do_fifo(void);
void show_stats(void);
void handle_sigint(__attribute__((__unused__)) int sig);

#endif
