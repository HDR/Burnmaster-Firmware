#ifndef __FLASH_PARAM_H__
#define __FLASH_PARAM_H__
 
#include <stdint.h>
 
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
 
 
#define FMC_PAGE_SIZE           ((uint16_t)0x400U)
#define FMC_WRITE_START_ADDR    ((uint32_t)0x0803FC00U)
#define FMC_WRITE_END_ADDR	((uint32_t)0x0803FFFFU)


void save_dword(uint32_t data);
uint32_t load_dword();


 
/* 需要保存的参数
typedef struct _CONFIG_PARAMS
{
		unsigned char data_valid_flag;				//数据有效标志  0xF1:有效  其它:无效
		unsigned char sleep_flag;             //是否进入休眠标志  0:不进入 1:进入
		unsigned char open_direction;					//开门方向   		0:左开门 1:右开门
		int16_t x_value;											//地磁X轴方向值
		int16_t y_value;											//地磁Y轴方向值
		int16_t z_value;											//地磁Z轴方向值
		uint32_t tmp_password_flag;						//临时密码使用标志
		unsigned char user_flag[50];					//当前有效用户标记
		unsigned char password_table[50][7];	//用户密码数组
}CONFIG_PARAMS;
 
 
// 系统状态信息
typedef struct _SYS_STATUS
{
		unsigned char door_state;				//门磁状态    0:关门  1:开门
		unsigned char auto_anti_lock;		//自动反锁配置 0:不自动反锁    1:自动反锁
		unsigned char quiet_flag;				//自动反锁配置 0:音频正常播放  1:部分音频静音
		unsigned char opto1_state;			//光耦1状态
		unsigned char opto2_state;			//光耦2状态
		unsigned char opto3_state;			//光耦3状态
		float battery_volatage;					//电池电压		0-5V
}SYS_STATUS;
 
 
typedef union _CONFIG_PARAMS_UNION
{
		CONFIG_PARAMS config_params;  // 360字节
		uint32_t data[104];						//用于通过联合体字节对齐，方便数据存储 104*4=416字节
}CONFIG_PARAMS_UNION;
 
 
extern int load_config_params(void);
 
extern CONFIG_PARAMS *get_config_params(void);
 
extern int is_config_params_avaliable(void);
 
extern int save_config_params(CONFIG_PARAMS *params);

 */
 
 
#ifdef __cplusplus
}
#endif /* __cplusplus */
 
#endif