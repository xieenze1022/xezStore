#include <linux/miscdevice.h>

#include "miscdev.h"

#define MAX_CMD_NUM 256
#define MISC_DEVICE_TYPE 0xAF

io_command_func cmd_array[MAX_CMD_NUM];
static atomic_t cmd_ref_nums[MAX_CMD_NUM];
static DEFINE_SPINLOCK(command_lock);

atomic_t num_miscdev_opens;

int io_command_register(int nr, io_command_func func)
{
	int ret = 0;

	spin_lock(&command_lock);
	//判断nr号是否合法
	if (nr < 0 || nr >= MAX_CMD_NUM || cmd_array[nr]) {
		printk("Rregister fail\n");
		ret = -EPERM;
	} else {
		cmd_array[nr] = func;
	}
	spin_unlock(&command_lock);

	return ret;
}
EXPORT_SYMBOL(io_command_register);

int io_command_unregister(int nr, io_command_func func)
{
	int r = 0;

	spin_lock(&command_lock);
	if (func == cmd_array[nr]) {
		cmd_array[nr] = 0;
	} else {
		r = -EPERM;
		printk("Unregister fail\n");
	}
	spin_unlock(&command_lock);

	if (!r) {
		//等待占用释放后再返回
		while (atomic_read(cmd_ref_nums + nr) > 0) {
			printk("Waiting for IOCTL %d complete\n", nr);
			schedule_timeout_uninterruptible(HZ / 10);
		}
	}
	return r;
}
EXPORT_SYMBOL(io_command_unregister);

static long miscdev_ioctl(struct file *filp, unsigned int cmd,
		   unsigned long param)
{
	unsigned long ret = -ENOTTY;
	io_command_func func;
	int nr = _IOC_NR(cmd);

	if (_IOC_TYPE(cmd) != MISC_DEVICE_TYPE)
		return ret;

	if (nr < 0 || nr >= MAX_CMD_NUM)
		return ret;

	atomic_inc(cmd_ref_nums + nr);
	//防止编译器优化
	mb();
	func = cmd_array[nr];
	if (func) {
		ret = func(param);
	}
	atomic_dec(cmd_ref_nums + nr);

	return ret;
}

static int miscdev_open(struct inode *inode, struct file *file)
{
	int ret;
	ret = try_module_get(THIS_MODULE);

	if (ret == 0) {
		ret = -EIO;
	} else {
		ret = 0;
		atomic_inc(&num_miscdev_opens);
	}

	return ret;
}

static int miscdev_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	atomic_dec(&num_miscdev_opens);
	return 0;
}

long miscdev_ioctl(struct file *filp, unsigned int cmd,
		   unsigned long param);

static const struct file_operations miscdev_fops = {
	.open = miscdev_open,
	.release = miscdev_release,
	.unlocked_ioctl = miscdev_ioctl
};

static struct miscdevice test_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "test",
	.fops = &miscdev_fops
};

int miscdev_init(void)
{
	int ret;

	atomic_set(&num_miscdev_opens, 0);
	ret = misc_register(&test_miscdev);
	if (ret) {
		printk("Failed to register miscdevic ret=[%d]\n", ret);
	}

	return ret;
}

void miscdev_exit(void)
{
	BUG_ON(atomic_read(&num_miscdev_opens) != 0);
	misc_deregister(&test_miscdev);
}
