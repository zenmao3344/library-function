/* For testing HMC5893 magnetic IC data, 900 Project Team
    Created by Davion     17-1-2
*/
#include<stdio.h>
#include "wiringPi.h"
#include<math.h>
#include "HMC5893.h"
#include "errno.h"
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <wiringSerial.h>
#include<stdlib.h>
#include <pthread.h>
#include"udpserver.h"
#include"hexutil.h"
#define RDY_MASK 0x01
#define RDY_SET 0x01

#define RDY_RESET 0x00

#define LOCK_MASK 0x02
#define LOCK_SET 0x02
#define LOCK_RESET 0x00


#define ReadAddress 0x1F
#define WriteAddress 0x1E
static int fd;
unsigned char   g_quit = 0 ;//�˳�״̬
static HMC5893_DATA test_data = {SAMPLE_AVERAGE_1, OUTPUT_RATE_15, MESURA_NORMAL, GAIN_1_3_1090, MODE_IDLE, 
                                    0, 0, 0, 0, 0};

void set_sample_average(AVERAGE average)
{
    unsigned char reg;
    reg = wiringPiI2CReadReg8(fd, HMC5883L_REG_CONFIG_A);
    reg &= (~SAMPLE_AVERAGE_MASK);
    reg |= (average << 5);
    
    wiringPiI2CWriteReg8(fd, HMC5883L_REG_CONFIG_A, reg);
}

void set_rate(RATE rate)
{
    unsigned char reg;
    reg = wiringPiI2CReadReg8(fd, HMC5883L_REG_CONFIG_A);
    reg &= (~OUTPUT_RATE_MASK);
    reg |= (rate << 2);
    wiringPiI2CWriteReg8(fd, HMC5883L_REG_CONFIG_A, reg);
}


void set_mode(MODE mode)
{
    unsigned char reg;
    reg = wiringPiI2CReadReg8(fd, HMC5883L_REG_MODE);
    reg &= (~MODE_MASK);
    reg |= mode;
    wiringPiI2CWriteReg8(fd, HMC5883L_REG_MODE, reg);
}


void set_gain(GAIN gain)
{   
    unsigned char reg;
    reg = wiringPiI2CReadReg8(fd, HMC5883L_REG_CONFIG_B);
    reg &= (~GAIN_MASK);
    reg |= (gain << 5);
    wiringPiI2CWriteReg8(fd, HMC5883L_REG_CONFIG_B, reg);
}


void set_mesure(MESURA mesure)
{
    unsigned char reg;
    reg = wiringPiI2CReadReg8(fd, HMC5883L_REG_CONFIG_A);
    reg &= (~MESURA_MASK);
    reg |= mesure;
    wiringPiI2CWriteReg8(fd, HMC5883L_REG_CONFIG_A, reg);
}

void hmc5893_init(void)
{   
    int status;
    status = wiringPiSetup();
    if(status != 0)
        {
        printf("wiringPi start error: %s\n", strerror(errno));
        
        }
    fd = wiringPiI2CSetup(WriteAddress);
    if(fd != 0)
        {
        printf("connect I2C device\n");
        }
    else
        {
        printf("cann't connect I2C device\n");
        
        }
    
    hmc5893_para_init(&test_data);   //����������ģʽ
    
}

void hmc5893_para_init(HMC5893_DATA* para)
{
    /*0x7F��ʾbit7����Ϊ0�ſ�������*/
    wiringPiI2CWriteReg8(fd, HMC5883L_REG_CONFIG_A, (para->sample_average << 5) | (para->output_rate << 2) | (para->mesura) & 0x7F);
    /*0xE0��ʾbit4~bit0����Ϊ0��������*/
    wiringPiI2CWriteReg8(fd, HMC5883L_REG_CONFIG_B, (para->gain << 5) & 0xE0); 
    /*0x83��ʾbit7����Ϊ1����bit0��bit1������λ����Ϊ0*/
    wiringPiI2CWriteReg8(fd, HMC5883L_REG_MODE, para->mode & 0x83);
                                                        
}

unsigned char get_RDY(void)
{
    unsigned char status;
    status = wiringPiI2CReadReg8(fd, HMC5883L_REG_STATUS);
    status = (status & RDY_MASK);
    return status;
}

unsigned char get_LOCK(void)
{
    unsigned char status;
    status = wiringPiI2CReadReg8(fd, HMC5883L_REG_STATUS);
    status = (status & LOCK_MASK);
    return status;
}

HMC5893_DATA* simple_mesure(HMC5893_DATA* data)
{
    short receive_data[2];
    set_mode(MODE_SINGLE);
    delay(300);
    while( RDY_SET != get_RDY());
    receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_XDATA_MSB);
    receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_XDATA_LSB); 
    data->axis[AXIS_X] = (receive_data[0] << 8) | receive_data[1];
    receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_YDATA_MSB);
    receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_YDATA_LSB); 
    data->axis[AXIS_Y] = (receive_data[0] << 8) | receive_data[1];
    receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_ZDATA_MSB);
    receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_ZDATA_LSB); 
    data->axis[AXIS_Z] = (receive_data[0] << 8) | receive_data[1];
    set_mode(MODE_IDLE);
    return data;
}

/*Ĭ���������Ƶ����15Hz��Ҳ����˵�Ĵ��������ֵ������1/15s����һ��*/
void continous_mesure(HMC5893_DATA* data)
{
    int receive_data[2], i;
    set_mode(MODE_CONTINOUS);
    delay(300);
    for(i = 0; i < 100; i++)
        {
    while( RDY_SET != get_RDY());
    receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_XDATA_MSB);
    receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_XDATA_LSB); 
    data->axis[AXIS_X] = (receive_data[0] << 8) | receive_data[1];
    receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_YDATA_MSB);
    receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_YDATA_LSB); 
    data->axis[AXIS_Y] = (receive_data[0] << 8) | receive_data[1];
    receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_ZDATA_MSB);
    receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_ZDATA_LSB); 
    data->axis[AXIS_Z] = (receive_data[0] << 8) | receive_data[1];
    printf("%d Times Measure: x value: %d  y value: %d  z value: %d\n", i, data->axis[AXIS_X], data->axis[AXIS_Y], data->axis[AXIS_Z]);
    memset(receive_data, 0, sizeof(receive_data));
    delay(5000);   //��ʱ5s
        }
    
}

void magnetic_mesure(HMC5893_DATA* data,char times)
{
    int receive_data[2], i;
	int x_sum,y_sum,z_sum;
	FILE *magnetic_fd ;
	char *magnetic_data = "/home/pi/HMC5893/magnetic_data";
	int length_w;
	int x_avg,y_avg,z_avg;
	float x_data,y_data,z_data,compose_data;
	x_sum = y_sum = z_sum = 0;
	x_avg = y_avg = z_avg = 0;
	x_data = y_data = z_data = compose_data = 0.0;
	

	magnetic_fd = fopen(magnetic_data,"a+") ;
	if (NULL == magnetic_fd)
    {
		printf("File open faild!\n ");
	}
    set_mode(MODE_CONTINOUS);
    delay(300);
	fprintf(magnetic_fd,"this is the %d times mesure\n", times);
	
    for(i = 0; i < 10; i++)
    {
       while( RDY_SET != get_RDY());
	   
       receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_XDATA_MSB);
       receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_XDATA_LSB); 
       data->axis[AXIS_X] = (receive_data[0] << 8) | receive_data[1];
	   x_sum = x_sum + data->axis[AXIS_X];
	   x_data = 0.92 * data->axis[AXIS_X] ;
	   fprintf(magnetic_fd,"%f ",x_data);

	   
       receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_YDATA_MSB);
       receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_YDATA_LSB); 
       data->axis[AXIS_Y] = (receive_data[0] << 8) | receive_data[1];
	   y_sum = y_sum + data->axis[AXIS_Y] ;
	   y_data = 0.92 * data->axis[AXIS_Y] ;
	   fprintf(magnetic_fd,"%f ",y_data);
	   
       receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_ZDATA_MSB);
       receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_ZDATA_LSB); 
       data->axis[AXIS_Z] = (receive_data[0] << 8) | receive_data[1];
	   z_sum = z_sum + data->axis[AXIS_Z] ;
	   z_data = 0.92 * data->axis[AXIS_Z] ; 
	   fprintf(magnetic_fd,"%f ",z_data);
	   
	   compose_data =  sqrt((x_data*x_data)+(y_data*y_data)+(z_data*z_data));
	   fprintf(magnetic_fd,"%f\n",compose_data);
	   
	   /*printf("%d Times Measure: x value: %d  y value: %d  z value: %d  compose value is: %f\n", i, data->axis[AXIS_X], data->axis[AXIS_Y], data->axis[AXIS_Z],compose_data);*/
       printf("%d Times Measure: x value: %f  y value: %f  z value: %f  compose value is: %f\n", i, x_data, y_data, z_data,compose_data);
       memset(receive_data, 0, sizeof(receive_data));
	   delay(500);
    }
	
	x_avg = x_sum / 10 ;
	y_avg = y_sum / 10 ;
	z_avg = z_sum / 10 ;
	
	x_data = 0.92 * x_avg ;
	y_data = 0.92 * y_avg ;
	z_data = 0.92 * z_avg ;
	compose_data = sqrt((x_data*x_data)+(y_data*y_data)+(z_data*z_data));
	
    fprintf(magnetic_fd,"%f ",x_data);
	fprintf(magnetic_fd,"%f ",y_data);
	fprintf(magnetic_fd,"%f ",z_data);
	fprintf(magnetic_fd,"%f\n",compose_data);
	
	printf("x average value: %f  y average value: %f  z average value: %f  compose average value is: %f\n",  x_data, y_data, z_data,compose_data);	
	fclose(magnetic_fd);
}

void magnetic_profile_mesure(HMC5893_DATA* data)
{
	char  count_times = 0 ;
	int receive_data[2], i;
	int x_sum,y_sum,z_sum;
	int x_avg,y_avg,z_avg;
	float proportion = 0.4;
	float x_data,y_data,z_data,compose_data;
	float x_data_past,y_data_past,z_data_past,compose_data_past;
	float serial_send_buf[4];
	unsigned char start_CMD[2] = {0x03,0xFC};//����ʾ����Э��淶-��ʼ��
	unsigned char end_CMD[2] = {0xFC,0x03};//����ʾ����Э��淶-ֹͣ��
	char data_buf[100]; 
	int Serial_fd = 0 ;
	pthread_t pth;
	
	/*if (-1 == (Serial_fd = serialOpen("/dev/ttyAMA0",115200)))//��ʼ������
	{
		printf("serial Open error\n");
		exit(1);
	}*/
	
	if (0 != pthread_create(&pth, NULL, (void*)scout_input, NULL))
	{
		printf("scout thread creat faild!\n");
	}
	
    //��ʼ������
	x_sum = y_sum = z_sum = 0;
	x_avg = y_avg = z_avg = 0;
	x_data = y_data = z_data = compose_data = 0.0;
	x_data_past = y_data_past = z_data_past = compose_data_past = 0.0;
	
	while(1)
	{
       if (1 == g_quit)
	   {
		   printf(" the magnetic_profile_mesure is over!\n");
		   exit(0);
	   }
	   while( RDY_SET != get_RDY());
	   
       receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_XDATA_MSB);
       receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_XDATA_LSB); 
       data->axis[AXIS_X] = (receive_data[0] << 8) | receive_data[1];
	   x_data = 0.92 * data->axis[AXIS_X] ;
	   
       receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_YDATA_MSB);
       receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_YDATA_LSB); 
       data->axis[AXIS_Y] = (receive_data[0] << 8) | receive_data[1];
	   y_data = 0.92 * data->axis[AXIS_Y] ;
	   
       receive_data[0] = wiringPiI2CReadReg8(fd, HMC5883L_REG_ZDATA_MSB);
       receive_data[1] = wiringPiI2CReadReg8(fd, HMC5883L_REG_ZDATA_LSB); 
       data->axis[AXIS_Z] = (receive_data[0] << 8) | receive_data[1];
	   z_data = 0.92 * data->axis[AXIS_Z] ; 
	   
	   compose_data =  sqrt((x_data*x_data)+(y_data*y_data)+(z_data*z_data));
	   
	   if (20 > count_times)   //����ǰ20�����ݵ�ƽ��ֵ����Ϊ�˲�ֵ��
	   {
		   count_times++;
		   x_sum = x_sum + data->axis[AXIS_X];
		   y_sum = y_sum + data->axis[AXIS_Y] ;
		   z_sum = z_sum + data->axis[AXIS_Z] ;
	   }
	   else if (20 == count_times) 
	   {
		   count_times++;
		   	x_avg = x_sum / 20 ;
	        y_avg = y_sum / 20 ;
	        z_avg = z_sum / 20 ;
			x_data_past = 0.92 * x_avg ;
	        y_data_past = 0.92 * y_avg ;
	        z_data_past = 0.92 * z_avg ;
			compose_data_past = sqrt((x_data_past*x_data_past)+(y_data_past*y_data_past)+(z_data_past*z_data_past));
	   }
	   else
	   {
	        serial_send_buf[0] = proportion*x_data +(1.0-proportion)*x_data_past ;
	        serial_send_buf[1]  = proportion*y_data +(1.0-proportion)*y_data_past ;
	        serial_send_buf[2]  = proportion*z_data +(1.0-proportion)*z_data_past ;
	        serial_send_buf[3]  = proportion*z_data +(1.0-proportion)*compose_data_past;
	   
	        x_data_past = serial_send_buf[0] ;
	        y_data_past = serial_send_buf[1] ;
	        z_data_past = serial_send_buf[2] ;
	        compose_data_past =  serial_send_buf[3];  
			  
			sprintf(data_buf,"%f,%f,%f,%f",serial_send_buf[0],serial_send_buf[1],serial_send_buf[2],serial_send_buf[3]);
			udp_send(data_buf,sizeof(data_buf));
	   }
	
       memset(receive_data, 0, sizeof(receive_data));
	
	   delay(50);
	}
		udp_join_server();
}

void* scout_input(void)
{
	char str[]="quit";
	char para[10];
	memset(para, 0, strlen(para));
	g_quit = 0;
	while(1)
	{
	    fscanf(stdin, "%s", para);
	    if(memcmp(str, para,4) == 0)
		{
          g_quit = 1 ;
		  g_test = 0;
		  exit(0);
		}
		
	}
	
	
}

void recv_callback(char* data, int len, struct sockaddr_in* addr) {
    printf("recv data:");
	int i;
   for(i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    //udp_send(data, len);
	char data1[] = "QUIT";
	int temp1 = 0;
	
	if(data[1] == 55)
	{
		g_quit = 1;
	}
	temp1 = sizeof(data1);
//	udp_send(data1, temp1);
}

 //������������5s����һ��
int main(int argc, char *argv[])
{
	char times = 0 ;
	char Mode  = 0 ;
	
	printf("the program name is %s",argv[0]);
	if (2 > argc)
	{
		printf ("Hi!\n");
		printf("Usage: Please input the mesure times\n");
	}
	
	times = atoi(argv[2]);
	Mode  = atoi(argv[1]);
	
    hmc5893_init();
    hmc5893_para_init(&test_data);
	
    int Serial_fd = 0 ;
	unsigned char start_CMD[2] = {0x03,0xFC};
	
	/*if (-1 == (Serial_fd = serialOpen("/dev/ttyAMA0",9600)))//��ʼ������
	{
		printf("serial Open error\n");
		exit(1);
	}
	write(Serial_fd,start_CMD,2 );
	serialClose(Serial_fd);
	printf("the serial close");
	*/
    
	if (2 == Mode)//ģʽ����������̬����
    {
		udp_start(23121, recv_callback);
		while(1)
		{
			if(g_test == 1)
			{
				magnetic_profile_mesure(&test_data);
			}
		}
	}                           
	else if(1 == Mode)//ģʽһ��������̬����
	{
	    magnetic_mesure(&test_data,times);
        printf(" the magnetic_mesure is over!\n");
	}
    
    return 0;
}

/*gcc -wall -o test HMC5893_test.c -I./ -lwiringPi*/
