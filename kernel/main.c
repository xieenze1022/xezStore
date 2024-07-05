#include <linux/module.h>

#include "ioctl/miscdev.h"
#include "func/copy_data.h"

static int __init test_module_init(void)
{
	int ret = 0;

	ret = miscdev_init();
	if (ret)
		goto out;

	ret = copy_data_func_init();
	if (ret)
		goto copy_out;

	printk("test module init success.\n");
	goto out;

copy_out:
	miscdev_exit();
out:
	return ret;

}

static void test_module_exit(void)
{
	copy_data_func_exit();
	miscdev_exit();

	printk("test module exit.\n");
}

MODULE_AUTHOR("XEZ");
MODULE_DESCRIPTION("TEST");
MODULE_LICENSE("GPL");

module_init(test_module_init);
module_exit(test_module_exit);
