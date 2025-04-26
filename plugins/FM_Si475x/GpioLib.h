#ifndef DEV_GPIO
#define DEV_GPIO

#ifdef __cplusplus
extern "C" {
#endif


/*
//pe16(32*4+16 = 144)
echo 144 > /sys/class/gpio/export
echo 144 > /sys/class/gpio/unexport
echo out > /sys/class/gpio/gpio144/directoin
echo in  > /sys/class/gpio/gpio144/directoin
echo 0   > /sys/class/gpio/gpio144/value
echo 1   > /sys/class/gpio/gpio144/value
*/

/** ExportGpioToUserSpace \功能 注册引脚
 * 
 * \参数 [输入] gpio      引脚号
 * 
 * \返回值 0       成功
 *        -1    失败
 *
 */
int ExportGpioToUserSpace(int gpio);

/** UnexportFromUserSpace \功能 注销引脚
 * 
 * \参数 [输入] gpio      引脚号
 * 
 * \返回值 0       成功
 *        -1    失败
 *
 */
int UnexportFromUserSpace(int gpio);

/** SetGPIODirection \功能 设置引脚方向
 * 
 * \参数 [输入] gpio      引脚号
 * \参数 [输入] dir       引脚方向 0：输入 1：输出
 * 
 * \返回值 0       成功
 *        -1    失败
 *
 */
int SetGPIODirection(int gpio, int dir);

/** SetGpioValue \功能      设置引脚值
 * 
 * \参数 [输入] gpio      引脚号
 * \参数 [输入] value     引脚值
 * 
 * \返回值 0       成功
 *        -1    失败
 *
 */
int SetGpioValue(int gpio, int value);

/** GetGpioValue \功能      获取引脚值
 * 
 * \参数 [输入] gpio      引脚号
 * \参数 [输入/输出] value  输入：缓冲区，输出：引脚值
 * 
 * \返回值 0       成功
 *        -1    失败
 *
 */
int GetGpioValue(int gpio, int *value);

#ifdef __cplusplus
}
#endif
#endif /* __DEV_GPIO__ */