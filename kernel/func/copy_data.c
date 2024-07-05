#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include "ioctl/miscdev.h"

#include "copy_data.h"

enum {
	IMPORT_DATA_CMD = 1,
	EXPORT_DATA_CMD,
};

struct general_policy
{
	int data_len;
	const char *data;
};

unsigned int g_buf_len = 0;
char *g_buf = NULL;

static long ioctl_import_data(unsigned long param)
{
	int ret = -1;
	struct general_policy gp;

	if (g_buf) {
		printk("kernel buf hava data, export fisrt!\n");
		goto err_out;
	}

	ret = copy_from_user(&gp, (void *)param, sizeof(gp));
	if (ret || !gp.data_len) {
		printk("%s copy_from_user error! ret[%d] data_len[%d]\n",
		       __func__, ret, gp.data_len);
		goto err_out;
	}

	g_buf = vmalloc(gp.data_len);
	if (!g_buf) {
		printk("%s vmalloc error! length[%d]\n", __func__, gp.data_len);
		goto err_out;
	}

	ret = copy_from_user(g_buf, gp.data, gp.data_len);
	if (ret) {
		printk("%s copy_from_user data failed!\n", __func__);
		g_buf_len = 0;
		vfree(g_buf);
		g_buf = NULL;

		goto err_out;
	}

	printk("kernel recv data:%s\n", g_buf);
	g_buf_len = gp.data_len;
err_out:
	return ret;
}

static long ioctl_export_data(unsigned long param)
{
	int ret = -1;
	char *p = NULL;

	if (!g_buf) {
		printk("No data in kernel memory, please import first\n");
		goto out;
	}

	printk("kernel send data:%s\n", g_buf);
	p = (char *)param;
	ret = copy_to_user((char __user *)p, (void *)g_buf, g_buf_len);
	if (ret) {
		printk("%s copy_to_user error!\n", __func__);
		goto out;
	}

	//若数据成功导出，则清空内核中的buf
	vfree(g_buf);
	g_buf = NULL;
	g_buf_len = 0;
out:
	return ret;
}

int copy_data_func_init(void)
{
	int ret = 0;

	//注册导入数据函数
	ret = io_command_register(IMPORT_DATA_CMD, (io_command_func)ioctl_import_data);
	if (ret) {
		printk("ioctl commond[%d] register error!\n", IMPORT_DATA_CMD);
		goto out;
	}

	ret = io_command_register(EXPORT_DATA_CMD, (io_command_func)ioctl_export_data);
	if (ret)
		printk("ioctl commond[%d] register error!\n", EXPORT_DATA_CMD);

out:
	return ret;
}

void copy_data_func_exit(void)
{
	io_command_unregister(IMPORT_DATA_CMD, (io_command_func)ioctl_import_data);
	io_command_unregister(EXPORT_DATA_CMD, (io_command_func)ioctl_export_data);
	printk("copy_data_func exited.\n");
}
