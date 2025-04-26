
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "icm42680.h"
#include "mahony_filter.h"
#include "log.h"
#include <vector>
#include <numeric>

#define DEV_I2C_BUS "/dev/i2c-0"

static unsigned char SLAVE_ADDR  = 0xD0 >>1;

static int dev_fd = -1;


#define WHOAMI        0X75


// 前后 看 x/z  主要看x
// 左右 看 y/z  主要看y

//--------------------------------

#define PWR_MGMT0     0x1F


#define TEMP_DATA 0x09

// 加速度参数
#define ACCEL_CONFIG0 0x21

#define ACCEL_DATA_X 0x0B 
#define ACCEL_DATA_Y 0x0D
#define ACCEL_DATA_Z 0x0F

// 角速度参数
#define GYRO_CONFIG0 0x20

#define GYRO_DATA_X  0x11
#define GYRO_DATA_Y  0x13
#define GYRO_DATA_Z  0x15





static float acc_scale = 16; //默认加速度缩放
static float gyro_scale = 2000;   //默认角速度缩放

static Axis3f AXIS_X = {1,0,0};  // x 坐标轴
static Axis3f AXIS_Y = {0,1,0};  // y 坐标轴
static Axis3f AXIS_Z = {0,0,1};  // z 坐标轴

static Point3f init_ang = {0,0,0};
int i2c_write_data(unsigned char reg_addr, __uint8_t data);
__uint16_t i2c_read_data(unsigned char addr);

int get_satus()
{
    return dev_fd; 
}

int open_dev(void)
{
    //Initialization work before power on RDAFM
    //Add your code here such as Initialization work of I2C. 
	dev_fd = open(DEV_I2C_BUS, O_RDWR);
    if (dev_fd < 0) {
       // LOGD("open dev filed \n");
        close(dev_fd);
        dev_fd = -1;
        return -1;
    }

      // 10011111  0x9F
    i2c_write_data(PWR_MGMT0,0x8F);
    usleep(1000 * 100);

    __uint16_t data;

   // LOGD("open dev success \n");
    data = i2c_read_data(WHOAMI);
    LOGD("WHOAMI: %0X \n",data >> 8);

    
    data = i2c_read_data(PWR_MGMT0);
    LOGD("PWR_MGMT0: %0X \n",data >>8);


    data = i2c_read_data( ACCEL_CONFIG0);
    LOGD("ACCEL_CONFIG0 %02X", data >> 8);

    data = i2c_read_data( GYRO_CONFIG0);
    LOGD("GYRO_CONFIG0 %02X", data >> 8);

    // __uint16_t data = i2c_write_data( ACCEL_CONFIG0,9);
    // LOGD("ACCEL_CONFIG0 %0X", data >> 8);

    // data = i2c_read_data( GYRO_CONFIG0);
    // LOGD("GYRO_CONFIG0 %0X", data >> 8);



    return dev_fd; 

}
float sumarr(std::vector<float>& arr )
{
    float sum = 0;
    for(size_t i = 0;i < arr.size();i++)
    {
        sum+= arr[i];
    }
    return sum;
}
// 姿态矫正 
void attitude_correction(int times,int alg)
{
    if(dev_fd < 0)
        return ;

    Axis3f acc;


    std::vector<float> arr_x;
    std::vector<float> arr_y;
    std::vector<float> arr_z;
     __int16_t data;
    while (times--) {

        data = i2c_read_data(ACCEL_DATA_X);
        acc.x  = data*acc_scale /32768;
       
        data = i2c_read_data(ACCEL_DATA_Y);
        acc.y  = data*acc_scale /32768;
     
        data = i2c_read_data(ACCEL_DATA_Z);
        acc.z  = data*acc_scale /32768;

        arr_x.push_back(acc.x);
        arr_y.push_back(acc.y);
        arr_z.push_back(acc.z);
     //  LOGD(" ax: %f, ay: %f, az: %f \n",  get_angle3d(acc,AXIS_X),  get_angle3d(acc,AXIS_Y), get_angle3d(acc,AXIS_Z));
    //    LOGD(" ")
        usleep(1000 * 100 );
    }

  //  LOGD(" arr_x size  = %d; sum %f ",arr_x.size(),sumarr(arr_x));
    acc.x = sumarr(arr_x) / (float)arr_x.size();
    acc.y = sumarr(arr_y) / (float)arr_y.size();
    acc.z = sumarr(arr_z) / (float)arr_z.size();

  //  LOGD(" ax: %f, ay: %f, az: %f \n", acc.x,   acc.y,  acc.z);
   
    init_ang.x = get_angle3d(acc,AXIS_X);
    init_ang.y = get_angle3d(acc,AXIS_Y);
    init_ang.z = get_angle3d(acc,AXIS_Z);
    LOGD("init_ang (%f,%f,%f)\n",   init_ang.x, init_ang.y, init_ang.z);
}

Point3f getCurAngle(){
     if(dev_fd < 0)
        return init_ang;
    // 加速度
    Axis3f   acc{0,0,0};
    __int16_t data ;
    int times  = 5;
    while (times--)
    {
        data = i2c_read_data(ACCEL_DATA_X);
        acc.x += data*acc_scale /32768;

        data = i2c_read_data(ACCEL_DATA_Y);
        acc.y  += data*acc_scale /32768;

        data = i2c_read_data(ACCEL_DATA_Z);
        acc.z  += data*acc_scale /32768;

    }
    
    acc.x /= 5;
    acc.y /= 5;
    acc.z /= 5;

    Point3f angs;
    angs.x = get_angle3d(acc,AXIS_X);
    angs.y = get_angle3d(acc,AXIS_Y);
    angs.z = get_angle3d(acc,AXIS_Z);



    //LOGD(" getCurAngle :angs (%f,%f,%f)",angs.x,angs.y,angs.z);

    return angs;
}

Point3f getInitAngle()
{
    return init_ang;
}


void  setInitAngle(Point3f const  &ang )
{
    init_ang = ang;
}
// 每次值操作一个寄存器，故数据最长为 short
// addr 寄存器地址
// data 数据
int i2c_write_data(unsigned char reg_addr, __uint8_t data) {
    //TODO
     if(dev_fd < 0)
        return  0 ;
    struct i2c_rdwr_ioctl_data rwdata;
   
	ioctl(dev_fd, I2C_TIMEOUT, 1);
    ioctl(dev_fd, I2C_RETRIES, 2);

  

    struct i2c_msg msgs[1];
    unsigned char buf1[2] ={};
    buf1[0]  = reg_addr;
    buf1[1] = data;
	//先设定一下地址
    msgs[0].addr =  SLAVE_ADDR;
	msgs[0].len = 2;
	msgs[0].flags = 0;//write
	msgs[0].buf = buf1;
	
	//然后从刚才设定的地址处读

    // unsigned char buf[1] = {data};
	// msgs[1].addr = SLAVE_ADDR;
    // msgs[1].flags = 0;
    // msgs[1].len = 1;
	// msgs[1].buf  = buf;
	
    rwdata.nmsgs  = 1;
	rwdata.msgs  = msgs;

    if (ioctl(dev_fd, I2C_RDWR, &rwdata) < 0) {
       // printf("write reg %0X failed \n", reg_addr);
        return -1;
    }
    usleep(200);
    
    return 0;
}

//S, id, address, delay, S, id, *buf, NACK,*last, P

__uint16_t i2c_read_data(unsigned char addr) {
     if(dev_fd < 0)
        return 0;
    struct i2c_rdwr_ioctl_data rwdata;

    ioctl(dev_fd, I2C_TIMEOUT, 1);
    ioctl(dev_fd, I2C_RETRIES, 2);

    struct i2c_msg msgs[2];
    unsigned char buf1[1] = {0};
	//先设定一下地址
    msgs[0].addr =  SLAVE_ADDR;
	msgs[0].len = 1;
	msgs[0].flags = 0;//write
	msgs[0].buf = buf1;
	msgs[0].buf[0] = addr;
	//然后从刚才设定的地址处读

    unsigned char buf[16] = {0};
	msgs[1].addr = SLAVE_ADDR;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = 2;
	msgs[1].buf  = buf;
	
    rwdata.nmsgs  = 2;
	rwdata.msgs  = msgs;

	int ret = ioctl(dev_fd, I2C_RDWR, (unsigned long)&rwdata);
    usleep(500);
	if(ret < 0){
		//printf("error during I2C_RDWR ioctl with error code %0X \n", ret);
        return -1;
    }
    __uint16_t tmp = buf[0] << 8;
    tmp |= buf[1];
    return tmp;
 
}

void delayms(unsigned short int ms)
{
	usleep(ms*1000);
}

// 量程4g例如：8000*4g/32768=0.9765625 默认 2000dps 16g
// 
void test()
{
    
    int id = 0;
    __int16_t  data = 0;

    id = i2c_read_data(WHOAMI);
    printf("id WHOAMI: %0X \n",id);

    // 10001111  0x8F
    data = i2c_write_data(PWR_MGMT0,0x8F);
    printf(" i2c_write_data PWR_MGMT0: %0X\n",0x8F);

    data = i2c_read_data(PWR_MGMT0);
    printf("i2c_read_data PWR_MGMT0: %0X\n",data);
  
   
     // 00100000 4g
    data = i2c_read_data( ACCEL_CONFIG0);
 //   printf(" i2c_write_data PWR_MGMT0: %0X\n",0x8F);
   
 
    // 00100000
    data = i2c_write_data( GYRO_CONFIG0,0x20);
 //   printf(" i2c_write_data PWR_MGMT0: %0X\n",0x8F);
    Axis3f gyro;
    Axis3f acc;
    int times = 100 ;
    while (times--)
    {
        printf("============================== \n");
        data = i2c_read_data(TEMP_DATA);
        printf("TEMP_DATA: %0X  t %d \n",data,data/128  +25);

        // 加速度
        data = i2c_read_data(ACCEL_DATA_X);
        acc.x  = data*acc_scale /32768;
       
        data = i2c_read_data(ACCEL_DATA_Y);
        acc.y  = data*acc_scale /32768;
     
        data = i2c_read_data(ACCEL_DATA_Z);
        acc.z  = data*acc_scale /32768;
      
        // 角速度
        data = i2c_read_data(GYRO_DATA_X);
        gyro.x = data*gyro_scale /32768;

        data = i2c_read_data(GYRO_DATA_Y);
        gyro.y = data*gyro_scale /32768; 

        data = i2c_read_data(GYRO_DATA_Z);
        gyro.z = data*gyro_scale /32768; 
       

        printf("acc:(%f,%f,%f); gyro:(%f,%f,%f)\n", acc.x, acc.y, acc.z, gyro.x, gyro.y, gyro.z);
        // mahony_filter.mahony_input(&mahony_filter,gyro,acc);
        // mahony_filter.mahony_update(&mahony_filter);
        // mahony_filter.mahony_output(&mahony_filter);

        // //  float pitch, roll, yaw;         // 姿态角：俯仰角，滚转角，偏航角
        // printf(" MODE : %f",sqrt(acc.x * acc.x +acc.y * acc.y +acc.z * acc.z ) );
        // printf("pitch, roll, yaw:(%f,%f,%f);\n\n\n", mahony_filter.pitch, mahony_filter.roll, mahony_filter.yaw);

       // printf(" MODE : %f",sqrt(acc.x * acc.x +acc.y * acc.y +acc.z * acc.z ) );

        Axis3f a_x = {1,0,0};
        Axis3f a_y = {0,1,0};
        Axis3f a_z = {0,0,1};
        get_angle3d(acc,a_x);

        printf(" ax: %f, ay: %f, az: %f \n",  get_angle3d(acc,a_x),  get_angle3d(acc,a_y), get_angle3d(acc,a_z));
        usleep(1000 * 1000);
    }

  

}
int main()
{
    printf("test gyro_icm \n");
    open_dev();
    test();
    
    return 0;
}


