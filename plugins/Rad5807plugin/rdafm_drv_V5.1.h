
#ifndef _RDAFM_DRV_H_
#define _RDAFM_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

void  RDAFM_test(void);
// 打开IIC总线
unsigned char RDAFM_init(void);
//电源开
void RDAFM_power_on(void);
void RDAFM_power_0ff(void);


/**
 * @brief 音量设置 
 * @It has better use the system volume operation to replace this function
 * @author sky
 * @date 2024-06-15
 * @param level: 输入 0 ～ 0xf 之间的值
 */

void RDAFM_set_volume(unsigned char level);


/**
 * @brief // 设置频率，频率单位为10K，即87.5MHZ频率，传入参数为8750
 * @author sky
 * @date 2024-06-15
 * @param curf: 设置的频率
 */
void RDAFM_set_freq(unsigned short int curf);

// 设置频点
void RDAFM_set_tune(unsigned short int curf);
// 读取频点
int RDAFM_read_tune();

void RDAFM_set_mute(unsigned char mute);

int RDAFM_valid_stop(unsigned short int freq);

unsigned char RDAFM_get_rssi(void);

int  RDAFM_write_data(unsigned char addr, unsigned short int data);
int  RDAFM_read_data(unsigned char addr, unsigned short int *pdata);
unsigned short int RDAFM_freq_to_chan(unsigned short int frequency);
unsigned short int RDAFM_chan_to_freq(unsigned short int chan);

int RDAFM_auto_seek();
#ifdef __cplusplus
}
#endif

#endif