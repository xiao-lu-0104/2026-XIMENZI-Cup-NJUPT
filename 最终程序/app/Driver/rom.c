#include "rom.h"

/**
  * @brief 		Flash页擦除
  * @param  	擦除地址
  * @retval 	空
  */
void internal_flash_erase(uint32_t addr)
{

	fmc_unlock();  //FMC解锁
	//FMC标志位清除
	fmc_flag_clear(FMC_FLAG_END); 
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_PGMERR);

	//页擦除
	fmc_page_erase(addr);
	//FMC标志位清除
	fmc_flag_clear(FMC_FLAG_END); 
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_PGMERR);

	fmc_lock();	//FMC上锁
}

/**
	* @brief 		Flash扇区擦除
  * @param  	擦除地址
  * @retval 	无
  */
void internal_flash_earse_sector(uint32_t addr)
{
	fmc_unlock();  //FMC解锁
	//FMC标志位清除
	fmc_flag_clear(FMC_FLAG_END); 
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_PGMERR);

	//FMC扇区擦除
	fmc_sector_erase(addr);
	//FMC标志位清除
	fmc_flag_clear(FMC_FLAG_END); 
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_PGMERR);

	fmc_lock(); //FMC上锁
}

/**
  * @brief 		FMC写入半字
  * @param  	写入地址
  * @param  	写入数据（半字）	
  * @retval 	无
  */
void internal_flash_write(uint32_t addr , uint16_t data)
{
	fmc_unlock();	//FMC解锁

	fmc_halfword_program(addr , data);
	//FMC标志位清除
	fmc_flag_clear(FMC_FLAG_END); 
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_PGMERR);

	fmc_lock(); //FMC上锁
}

/**
  * @brief 		FMC写入字
  * @param  	写入地址
  * @param  	写入数据（字）
  * @retval 	无
  */
void internal_flash_write_word(uint32_t addr , uint32_t data)
{
	fmc_unlock(); //FMC解锁

	fmc_word_program(addr , data);
	//FMC标志位清除
	fmc_flag_clear(FMC_FLAG_END); 
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_PGMERR);

	fmc_lock(); //FMC上锁
}

/**
  * @brief 		FMC写入字符串
  * @param  	写入地址
  * @param  	写入数据（字符串指针类型）
	* @param  	数据长度
	* @retval 	无
  */
void internal_flash_write_str(uint32_t addr , uint16_t* data , uint16_t len)
{
	fmc_unlock();  //FMC解锁

	for (uint16_t i = 0; i < len; i++)
	{
		fmc_halfword_program(addr + i * 2 , data[i]);
	}
	//FMC标志位清除
	fmc_flag_clear(FMC_FLAG_END); 
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_PGMERR);

	fmc_lock();  //FMC上锁
}

/**
  * @brief 		FMC写入字符串（char型）
  * @param  	写入地址
  * @param  	写入数据（字符串指针类型）
	* @param  	数据长度
	* @retval 	无
  */
void internal_flash_write_str_Char(uint32_t addr , uint8_t* data , uint32_t len)
{
	fmc_unlock(); //FMC解锁

	for (uint32_t i = 0; i < len; i++)
	{
		fmc_byte_program(addr + i , data[i]);
		while(fmc_flag_get(FMC_FLAG_BUSY));
	}
	//FMC标志位清除
	fmc_flag_clear(FMC_FLAG_END); 
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_PGMERR);

	fmc_lock(); //FMC上锁
}

/**
  * @brief 		半字读Flash
  * @param  	读地址
  * @retval 	返回的半字
  */
uint16_t internal_flash_read(uint32_t addr)
{
	return *(volatile uint16_t*)(addr);
}

/**
  * @brief 		字节读Flash
  * @param  	读地址
  * @retval 	返回的字节
  */
uint8_t internal_flash_read_Char(uint32_t addr)
{
	return *(volatile uint8_t*)(addr);
}

/**
  * @brief 		全字读Flash
  * @param  	读地址
  * @retval 	返回的全字
  */
uint32_t internal_flash_read_word(uint32_t addr)
{
	return *(volatile uint32_t*)(addr);
}

