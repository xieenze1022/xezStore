#ifndef __MISCDEV_H__
#define __MISCDEV_H__

int miscdev_init(void);
void miscdev_exit(void);

typedef long (*io_command_func)(unsigned long param);
int io_command_register(int nr, io_command_func func);
int io_command_unregister(int nr, io_command_func func);

#endif