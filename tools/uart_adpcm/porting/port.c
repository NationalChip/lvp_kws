#include "port.h"
#include "../message.h"

//porting
static int fd = -1;

#ifdef WINDOWS
static HANDLE hmu;
#endif

static bool Config(int speed, int databits, int stopBits, char parity)
{
#ifdef WINDOWS
	char comSetStr[30];
	void *filehandle = (void *)fd;
	sprintf(comSetStr, "%d,%c,%d,%d", speed, parity, databits, stopBits);
         printf("%s\n", comSetStr);
	DCB dcb;
         COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 100;
	TimeOuts.ReadTotalTimeoutMultiplier = 50;
	TimeOuts.ReadTotalTimeoutConstant = 500;
	//�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;

	FillMemory(&dcb, sizeof(dcb), 0);
	dcb.DCBlength = sizeof(dcb);
	// try to build the DCB
	if (!BuildCommDCB(comSetStr, &dcb)) {
		fprintf(stderr, "Serial config BuildCommDCB error.\n");
		return false;
	}
	// set the state of filehandle to be dcb
	if(!SetCommState(filehandle, &dcb)) {
		fprintf(stderr, "Serial config SetCommState error.\n");
		return false;
	}

	// set the buffers to be size 1024 of filehandle
	if(!SetupComm(filehandle, 1024, 1024)) {
		fprintf(stderr, "Serial config SetupComm error.\n");
		return false;
	}
        if(!SetCommTimeouts(filehandle, &TimeOuts)) {
		fprintf(stderr, "Serial config setTimeouts error.\n");
		return false;
	}

         hmu = CreateMutex(NULL, FALSE, NULL);
	return true;
    
#else
	//printf("speed = %d\n", speed);
	int speed_arr[] = {B3000000, B2000000, B1000000, B460800, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
	int name_arr[]  = {3000000, 2000000, 1000000, 460800, 115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200,  300};
	struct termios opt;

	/* waits until all output written to the object referred to by fd has been transmitted */
	tcdrain(fd);

	/* flushes both data received but not read, and data written but not transmitted. */
	tcflush(fd, TCIOFLUSH);

	if (tcgetattr(fd, &opt) != 0) {
		fprintf(stderr, "111Serial config tcgetattr error.\n");
		return false;
	}

	// set speed
	for (int i= 0; i < (int)(sizeof(speed_arr) / sizeof(int)); i++) {
				if  (speed == name_arr[i]) {
					if (cfsetispeed(&opt, speed_arr[i]) != 0)
						fprintf(stderr, "Serial config cfsetispeed error\n");
					if (cfsetospeed(&opt, speed_arr[i]) != 0)
						fprintf(stderr, "Serial config cfsetospeed error\n");
					break;
				}
	}

	opt.c_cflag &= ~CSIZE;
	switch (databits) { /*设置数据位数*/
		case 7:
			opt.c_cflag |= CS7;
			break;
		case 8:
			opt.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return false;
	}
	switch (parity) {
		case 'n':
		case 'N':
					opt.c_cflag &= ~PARENB;           /* Clear parity enable */
					opt.c_iflag &= ~INPCK;            /* Enable parity checking */
					break;
		case 'o':
		case 'O':
					opt.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
					opt.c_iflag |= INPCK;             /* Disnable parity checking */
					break;
		case 'e':
		case 'E':
					opt.c_cflag |= PARENB;            /* Enable parity */
					opt.c_cflag &= ~PARODD;           /* 转换为偶效验*/
					opt.c_iflag |= INPCK;             /* Disnable parity checking */
					break;
		case 'S':
		case 's':  /*as no parity*/
					opt.c_cflag &= ~PARENB;
					opt.c_cflag &= ~CSTOPB;break;
		default:
					fprintf(stderr,"Unsupported parity\n");
					return false;
	}

			/* 设置停止位*/
	switch (stopBits) {
		case 1:
					opt.c_cflag &= ~CSTOPB;
					break;
		case 2:
					opt.c_cflag |= CSTOPB;
					break;
		default:
					fprintf(stderr,"Unsupported stop bits\n");
					return false;
	}
			/* Set input parity option */
	if (parity != 'n')
		opt.c_iflag |= INPCK;

	opt.c_cc[VTIME] = 150; /* 设置超时 15 seconds          */
	opt.c_cc[VMIN] = 0;    /* Update the opt and do it NOW */

	opt.c_cflag &= ~CRTSCTS;
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Input  */
	opt.c_oflag &= ~OPOST;                           /* Output */
	opt.c_iflag &= ~(INLCR | ICRNL | IGNCR | IXON);

	if (tcsetattr(fd, TCSANOW, &opt) != 0) {
		fprintf(stderr, "Serial 222config tcsetattr error.\n");
		return false;
	}

	return true;
#endif
}

void dump(unsigned char* buffer, int len)
{
#if 0
	printf("\n********************\n");
	int i = 0;
	for (i = 0; i < len; i++)
	{
		printf("0x%02x, ", buffer[i]);
	}
	printf("\n********************\n");
#endif
}

static int Read(unsigned char *buf, int size)
{
#ifdef WINDOWS
    DWORD read = 0;
    WaitForSingleObject(hmu, INFINITE);
    ReadFile((void *)fd, buf, size, &read, NULL);
    ReleaseMutex(hmu);
    if(read > 0)
    {
        dump(buf, read);
    }
    return read;
#else
    int ret = read(fd, buf, size);
    if(ret > 0)
    {
        dump(buf, ret);
    }
    return ret;
#endif
}

static int Write(const void *buf, int size)
{
#ifdef WINDOWS
    DWORD write = 0;
    WaitForSingleObject(hmu, INFINITE);
    WriteFile((void *)fd, buf, size, &write, NULL);
    ReleaseMutex(hmu);
    return write;
#else
    return write(fd, buf, size);
#endif
}

static int StreamIsEmpty(void)
{
    return 0;
}

static int StreamRead(unsigned char *buf, int len)
{
    //printf("StreamRead = %d\n", len);
    int real_len = 0;
    while(real_len < len)
	real_len += Read(buf+real_len, len - real_len);

    if(real_len != len)
        printf("read error\n");

    return real_len;
}

static int StreamWrite(const unsigned char *buf, int len)
{
   /*for (int i=0; i<len; i++)
			printf("%02x,", buf[i]);*/

#ifdef WINDOWS
        return Write(buf, len);
#else
	return write(fd, buf, len);
#endif
}

int uart_config(char *com, int baudrate, int databits, int stopbits, char parity)
{
#ifdef WINDOWS
    static char serial_name[32];
    void *filehandle;
    char devname[32];
    if (com != NULL) {
		int PortNum = -1;
		if(sscanf(com,"COM%d",&PortNum) == 0){
			printf("error: open serial com name is %s\n", com);
			return false;
		}
		if(PortNum < 10){
			sprintf(devname, "%s", com);
		}else{
			sprintf(devname, "\\\\.\\%s", com);
			strcpy(serial_name, devname);
		}
        filehandle = CreateFile(devname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
        fd = (int)filehandle;
		if(filehandle == INVALID_HANDLE_VALUE){
			printf("open serial %s failed, please check.\n", com);
			return false;
		}
	}
    Config(baudrate, databits, stopbits, parity);
#else
    if (fd >= 0 )
        close(fd);
    fd = -1;
    fd = open(com, O_RDWR | O_NOCTTY);
    Config(baudrate, databits, stopbits, parity);
#endif
}

int sys_init(void)
{
	MessageInit((STREAM_READ)StreamRead, (STREAM_WRITE)StreamWrite, (STREAM_EMPTY)StreamIsEmpty);
	return 0;
}

uint64_t get_time_ms(void)
{
#ifdef WINDOWS
    Sleep(1);
#else
    usleep(1000*10);
#endif

    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

#if 0
int uart_read_nonblock(uint8_t *ch, int timeout_ms)
{
    uint64_t start_time, current_time;

    start_time = get_time_ms();
    current_time = start_time;
    while (current_time < start_time + timeout_ms) {
        if (uart_read(ch, 1) == 1){
            //printf("ch = %c\n", *ch);
            return 1;
        }
        else
            current_time = get_time_ms();
    }
    return -1;
}

int uart_try_read(uint8_t *buf, int size)
{
#ifdef WINDOWS
	void *filehandle = (void *)fd;
	DWORD read = 0;
	COMMTIMEOUTS cmt;
	// ��������������
	cmt.ReadIntervalTimeout         = MAXDWORD;
	cmt.ReadTotalTimeoutMultiplier  = 0;
	cmt.ReadTotalTimeoutConstant    = 0;
	cmt.WriteTotalTimeoutMultiplier = 500;
	cmt.WriteTotalTimeoutConstant   = 2000;

	if(!SetCommTimeouts(filehandle, &cmt)) {
		return -1;
	}
	ReadFile(filehandle, buf, size, &read, NULL);
	return read;
#else
	struct termios opt;

	if (tcgetattr(fd, &opt) == -1) {
        return false;
	}

	opt.c_cc[VTIME] = 0;
	opt.c_cc[VMIN]  = 0;
	if (tcsetattr(fd, TCSANOW, &opt) != 0) {
	return false;
	}
	return read(fd, buf, size);
#endif
}
#endif


