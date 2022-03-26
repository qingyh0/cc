#include "stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

int get_tem_hum(float *tem, float *hum)
{
	char str[10]={0};
	int fd=0;
	int ret=0;
	fd = open("/dev/si7006", O_RDONLY);
	if(fd <0 )
	{
		perror("open error");
		return fd;
	}

	ret = read(fd,str, sizeof(str));
	if(ret < 0)
	{
		perror("read error");
		return ret;
	}

	//湿度 - 得到的是大端字节序
	unsigned short datatemp = 0;
	datatemp = (*(char*)str << 8) +(*((char*)str+1));
	*hum = 125* datatemp /65536.0 -6;

	datatemp = (*((char*)str+2) << 8) +(*((char*)str+3));
	*tem = 175.72 *datatemp /65536 -46.85;

	// printf("[hum]=%.2f\n[tem]=%.2f\n", *hum, *tem);
	close(fd);
	return 0;
}
