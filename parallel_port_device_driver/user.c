#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
int fd;

//--------------------------------------------------------------------------
char get_data() {
    char ch;
    system("clear");
    printf("\n\n                 << enter the data to sent to the device >>\n");
    printf("\n\n\nenter a char : ");
    getchar();
    ch = getchar();

    return ch;
}

//--------------------------------------------------------------------------
void show_data(char ch) {
    system("clear");
    printf("\n\n                      << this data was read from device >>\n");
    printf("\n\n\ndata is : %c", ch);

    return;
}

//--------------------------------------------------------------------------
int open_device() {
    return open("/dev/test", O_RDWR);
}

//--------------------------------------------------------------------------
void read_from_device() {
    char input_buf[2];
    read(fd, input_buf, 1);
    input_buf[1]='\0';
    printf("\ndata = %x\n", input_buf[0]);

    return;
}

//--------------------------------------------------------------------------
void write_to_device() {
    char output_buf[2];
    output_buf[0] = get_data();
    write(fd, output_buf, 1);

    return;
}

//--------------------------------------------------------------------------
main() {
    int job, flag = 0;
    if ((fd = open_device()) == -1) {
	printf("\ncan not open device ...\n");
        exit(1);
    }

    while (1) {
	system ("clear");
	printf("\n\n             << This is an application to use device driver >>\n");
	printf("\n\n\n1) read data\n");
	printf("2) write data\n");
	printf("3) exit\n");
	printf("\n\nselect (1-3) : ");
	scanf("%d", &job);

	switch (job) {
	    case 1:
		read_from_device();
		break;
	    case 2:
		write_to_device();
		break;
	    case 3:
		flag = 1;
		break;
	    default:
		printf("\nwrong number ...\n");
	}

	if (flag == 1) break;
	printf("\n\n\npress ENTER to continue ...\n");
	getchar();
	getchar();
    }

    close (fd);
}
