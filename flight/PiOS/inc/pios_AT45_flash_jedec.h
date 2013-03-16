/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASH Flash device handler
 * @{
 *
 * @file       pios_flash_w25x.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Driver for talking to W25X flash chip (and most JEDEC chips)
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#define AT45DB_BUFFER_1_WRITE                 0x84	/* 写入第一缓冲区 */
#define AT45DB_BUFFER_2_WRITE                 0x87	/* 写入第二缓冲区 */
#define AT45DB_BUFFER_1_READ                  0xD4	/* 读取第一缓冲区 */
#define AT45DB_BUFFER_2_READ                  0xD6	/* 读取第二缓冲区 */
#define AT45DB_B1_TO_MM_PAGE_PROG_WITH_ERASE  0x83	/* 将第一缓冲区的数据写入主存储器（擦除模式）*/
#define AT45DB_B2_TO_MM_PAGE_PROG_WITH_ERASE  0x86	/* 将第二缓冲区的数据写入主存储器（擦除模式）*/
#define AT45DB_MM_PAGE_TO_B1_XFER             0x53	/* 将主存储器的指定页数据加载到第一缓冲区    */
#define AT45DB_MM_PAGE_TO_B2_XFER             0x55	/* 将主存储器的指定页数据加载到第二缓冲区    */
#define AT45DB_PAGE_ERASE                     0x81	/* 页删除（每页512/528字节） */
#define AT45DB_SECTOR_ERASE                   0x7C	/* 扇区擦除（每扇区128K字节）*/
#define AT45DB_READ_STATE_REGISTER            0xD7	/* 读取状态寄存器 */
#define AT45DB_MM_PAGE_READ                   0xD2	/* 读取主储存器的指定页 */
#define AT45DB_MM_PAGE_PROG_THRU_BUFFER1      0x82  /* 通过缓冲区写入主储存器 */

struct pios_flash_jedec_cfg {
	uint32_t sector_erase;
	uint32_t chip_erase;
};

struct pios_flash_chunk {
	uint8_t * addr;
	uint32_t len;
};

uint32_t df_PageSize;

int32_t PIOS_Flash_Jedec_Init(uint32_t spi_id, uint32_t slave_num, const struct pios_flash_jedec_cfg * cfg);
int32_t PIOS_Flash_Jedec_ReadStatus();
int32_t PIOS_Flash_Jedec_ReadID();
int32_t PIOS_Flash_Jedec_EraseChip();
int32_t PIOS_Flash_Jedec_ErasePage(uint32_t add);
int32_t PIOS_Flash_Jedec_WriteData(uint32_t addr, uint8_t * data, uint16_t len);
int32_t PIOS_Flash_Jedec_ReadData(uint32_t addr, uint8_t * data, uint16_t len);
int32_t PIOS_Flash_Jedec_WriteChunks(uint32_t addr, struct pios_flash_chunk * p_chunk, uint32_t num);
int32_t PIOS_Flash_Jedec_StartTransaction();
int32_t PIOS_Flash_Jedec_EndTransaction();
int32_t PIOS_Flash_Jedec_PageSize();
int32_t PIOS_Flash_Jedec_PageToBuffer(unsigned char BufferNum, uint16_t PageAdr);
int32_t PIOS_Flash_Jedec_BufferToPage(unsigned char BufferNum, uint16_t PageAdr);
int32_t PIOS_Flash_Jedec_BufferWrite (unsigned char BufferNum, uint16_t IntPageAdr,  uint8_t * data, uint16_t len);
int32_t PIOS_Flash_Jedec_BufferRead (unsigned char BufferNum, uint16_t IntBufferAdr,  uint8_t * data, uint16_t len);
//int32_t PIOS_Flash_Jedec_ReadPage(uint32_t addr, uint8_t * data, uint16_t len);
int32_t PIOS_Flash_Jedec_ReadPage(uint32_t page, uint16_t offset, uint8_t * data, uint16_t len);
//int32_t PIOS_Flash_Jedec_WritePage(uint32_t page, uint8_t * data, uint16_t offset, uint16_t len);
int32_t PIOS_Flash_Jedec_WritePage(uint32_t page, uint16_t offset, uint8_t * data, uint16_t len);

int32_t PIOS_Flash_Jedec_PrintID();


