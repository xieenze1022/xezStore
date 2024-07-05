#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define MISC_DEV "/dev/test"
#define IOCTL_DEVICE_TYPE  0xAF

enum {
	IMPORT_DATA_CMD = 1,
	EXPORT_DATA_CMD,
};

struct general_policy
{
	int data_len;
	const char *data;
};

#define IMPORT_DATA _IOWR(IOCTL_DEVICE_TYPE, IMPORT_DATA_CMD, unsigned long)
#define EXPORT_DATA _IOWR(IOCTL_DEVICE_TYPE, EXPORT_DATA_CMD, unsigned long)

int ioctl_user(unsigned long cmd, unsigned long param)
{
	int ret = 0;
	int fd;

	fd = open(MISC_DEV, O_RDWR);
	if (fd < 0) {
		printf("Open dev %s failed,ret=%d\n", MISC_DEV, fd);
		return fd;
	}

	//通过ioctl发送命令
	ret = ioctl(fd, cmd, param);
	if (ret < 0) {
		printf("ioctl ret[%d] cmd[%ld]\n", ret, _IOC_NR(cmd));
	}

	close(fd);
	return ret;
}

int import_data_user_api(const char *data ,int length)
{
	struct general_policy gp;
	gp.data_len= length;
	gp.data = data;

	printf("app send data:%s\n", data);
	return ioctl_user(IMPORT_DATA, (unsigned long)&gp);
}

int export_data_user_api(char *buf)
{
	int ret;
	ret = ioctl_user(EXPORT_DATA, (unsigned long)buf);
	if (ret < 0)
		return ret;

	printf("app recv kernel buf:%s\n", buf);

	//buf = (char *)buffer;
	return 0;
}

#define USER_DATA_FILE "user_data.txt"
#define KERNEL_DATA_FILE "kernel_data.txt"

int main()
{
	int ret;
	int fd;
	struct stat st;

	long buf_len;
	char *buf = NULL;

	fd = open(USER_DATA_FILE, O_RDONLY);
	if (fd == -1) {
		printf("Failed to open file\n");
		return -1;
	}

	ret = stat(USER_DATA_FILE, &st);
	if (ret) {
		perror("stat");
		return -1;
	}

	buf_len = st.st_size;
	if (buf_len <= 0) {
		printf("file size error\n");
		return -1;
	}

	buf = (char *)malloc(buf_len + 1);
	if (!buf) {
		printf("malloc error\n");
		return -1;
	}

	read(fd, buf, buf_len);
	close(fd);

	import_data_user_api(buf, buf_len);

	memset(buf, 0x00, buf_len);
	export_data_user_api(buf);

	fd = open(KERNEL_DATA_FILE, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Failed to open file\n");
		return -1;
	}

	write(fd, buf, buf_len);
	close(fd);

	free(buf);

	return 0;
}
