#include <stdio.h>
#include <string.h>
#include <gd32f10x.h>
#include "flashparam.h"
 


void save_dword(uint32_t data)
{
  //
  fmc_unlock();
  fmc_word_program(FMC_WRITE_START_ADDR, data);
  //lock the main FMC after the program operation */
  fmc_lock();
}

uint32_t load_dword()
{
  //
  uint32_t *ptr = (uint32_t *)FMC_WRITE_START_ADDR;
  return  ptr[0];
}


/*

static CONFIG_PARAMS g_config_params = {0};



 
static void fmc_erase_pages(void)
{
    //unlock the flash program/erase controller 
    fmc_unlock();
    //clear all pending flags 
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
    //erase the flash pages 
    fmc_page_erase(FMC_WRITE_START_ADDR);
	fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
    //lock the main FMC after the erase operation 
    fmc_lock();
}
 
static void fmc_program(uint32_t *data, int data_len)
{
    //unlock the flash program/erase controller 
    fmc_unlock();
 
    uint32_t address = FMC_WRITE_START_ADDR;
    //program flash 
    while(address <= FMC_WRITE_END_ADDR) {
        if(data_len <= 0)
            break;
        fmc_word_program(address, *data);
        address += 4U;
		data++;
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
        data_len-=4;
    }
    //lock the main FMC after the program operation 
    fmc_lock();
}
 
//参数是否有效
int is_config_params_avaliable()
{
		return (g_config_params.data_valid_flag == 0xF1)?1:0;
}
 
//加载配置参数
int load_config_params()
{	
		CONFIG_PARAMS_UNION config_params_union;
	  unsigned char *ptr = (unsigned char *)FMC_WRITE_START_ADDR;													 //将指针指向存储区首地址
		if(*ptr == 0xF1)
		{
				//从Flash中读取到有效的配置数据
				memcpy(&config_params_union, ptr, sizeof(CONFIG_PARAMS_UNION));						 
				memcpy(&g_config_params, &config_params_union.config_params, sizeof(CONFIG_PARAMS)); //拷贝数据
				return 0;
		}
		return -1;
}
 
//获取参数配置
CONFIG_PARAMS *get_config_params()
{
		return &g_config_params;
}
 
 
//保存配置参数
int save_config_params(CONFIG_PARAMS *params)
{
		if(params == NULL)
				return -1;
		
		CONFIG_PARAMS_UNION config_params_union;
		memset(&config_params_union, 0, sizeof(CONFIG_PARAMS_UNION));	//清空数据
		memcpy(&config_params_union.config_params, params, sizeof(CONFIG_PARAMS)); //拷贝数据
		//擦除FLASH
		fmc_erase_pages();
		//保存配置数据到Flash中
		fmc_program((uint32_t *)&config_params_union, sizeof(CONFIG_PARAMS_UNION));
		
		return 0;
}

 */