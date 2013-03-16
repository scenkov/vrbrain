/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASHFS_OBJLIST Object list based flash filesystem (low ram)
 * @{
 *
 * @file       pios_flashfs_objlist.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      A file system for storing UAVObject in flash chip
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


#include "openpilot.h"
#include "uavobjectmanager.h"

uint32_t object_table_magicXX;
// Private functions
static int32_t PIOS_FLASHFS_ClearObjectTableHeader();
static int32_t PIOS_FLASHFS_GetObjAddress(uint32_t objId, uint16_t instId);
static int32_t PIOS_FLASHFS_GetNewAddress(uint32_t objId, uint16_t instId);

// Private variables
static int32_t numObjects = -1;
//unsigned char AT45_buffer[528];

// Private structures
// Header for objects in the file system table
struct objectHeader {
	uint32_t objMagic;
	uint32_t objId;
	uint32_t instId;
	uint32_t address;
} __attribute__((packed));;

struct fileHeader {
	uint32_t id;
	uint16_t instId;
	uint16_t size;
} __attribute__((packed));


#define MAX_BADMAGIC       1000
/*
void clear_AT45dbx_buffer()
{
for(uint32_t i = 0; i < 528; i ++) {
	AT45_buffer[i] = 0xff;
}
}
*/
static const struct flashfs_cfg * cfg;
/**
 * @brief Initialize the flash object setting FS
 * @return 0 if success, -1 if failure
 */
int32_t PIOS_FLASHFS_Init(const struct flashfs_cfg * new_cfg)
{
//PIOS_LED_On(PIOS_LED_ALARM);
	cfg = new_cfg;
	
	// Check for valid object table or create one
	uint32_t object_table_magic;
	uint32_t magic_fail_count = 0;
	bool magic_good = false;

//	PIOS_Flash_Jedec_ErasePage(0);

	while(!magic_good) {
//PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
		if (PIOS_Flash_Jedec_ReadPage(0, 0,(uint8_t *)&object_table_magic, sizeof(object_table_magic)) != 0)
			return -1;

	//	object_table_magicXX=object_table_magic;

		if(object_table_magic != new_cfg->table_magic) {
//			PIOS_LED_On(PIOS_LED_GREEN);
			if(magic_fail_count++ > MAX_BADMAGIC) {
	//			PIOS_LED_On(PIOS_LED_GREEN);
				if(PIOS_FLASHFS_Format() != 0)
					return -1;
#if defined(PIOS_LED_HEARTBEAT)
				PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
	//		PIOS_LED_Toggle(PIOS_LED_RED);
#endif	/* PIOS_LED_HEARTBEAT */
				magic_fail_count = 0;
				magic_good = true;
			} else {
				PIOS_DELAY_WaituS(1000);
			}
		}
		else {
//			PIOS_LED_On(PIOS_LED_YELLOW);
			magic_good = true;
		}

	}

//	int32_t addr = cfg->obj_table_start;
	struct objectHeader header;
	numObjects = 0;

	// Loop through header area while objects detect to count how many saved
	// Read Page 1 -  Table of objects
//	if (PIOS_Flash_Jedec_ReadData(1, (uint8_t *)&AT45_buffer, sizeof(AT45_buffer)) != 0)
//				return -1;
	int32_t addr = 0x00;
	while(addr < 528) {
		if (PIOS_Flash_Jedec_ReadPage(1, addr, (uint8_t *)&header, sizeof(header)) != 0)
						return -1;
//		header.objMagic = (AT45_buffer[addr] << 24) + (AT45_buffer[addr+1] << 16) + (AT45_buffer[addr+2] << 8) + (AT45_buffer[addr+3] );
		if(header.objMagic != cfg->obj_magic)
					break;

				numObjects++;
				addr += sizeof(header);

	}
/*
	header.objMagic = cfg->obj_magic;
	header.objId = 0x99887766;
	header.instId = 0xaabbccdd;
	header.address = 0x11223344;

//	if(PIOS_Flash_Jedec_WriteData(1, (uint8_t *) &header, sizeof(header)) != 0)
	if(PIOS_Flash_Jedec_WriteData(1, (uint8_t *) &header,  sizeof(header)) != 0)
			return -3;
	*/
/*
	header.objMagic = cfg->obj_magic;
	header.objId = 0x99887766;
	header.instId = 0xaabbccdd;
	header.address = 0x11223344;
*/
//	clear_AT45dbx_buffer();
/*
	if(PIOS_Flash_Jedec_BufferWrite(1, 0 ,(uint8_t *) &header,  sizeof(header)) != 0)
			return -3;
	//if(PIOS_Flash_Jedec_WriteData(3, (uint8_t *) &header, sizeof(header)) != 0)
	if(PIOS_Flash_Jedec_WritePage(2, (uint8_t *) &header, 0 , sizeof(header)) != 0)
			return -3;


	if(PIOS_Flash_Jedec_WriteData(3, (uint8_t *) &header,  sizeof(header)) != 0)
			return -3;

	if(PIOS_Flash_Jedec_WriteData(4, (uint8_t *) &header,  sizeof(header)) != 0)
			return -3;
	if(PIOS_Flash_Jedec_WriteData(14, (uint8_t *) &header, sizeof(header)) != 0)
			return -3;
	if(PIOS_Flash_Jedec_WritePage(255, (uint8_t *) &header, 0 , sizeof(header)) != 0)
			return -3;
	if(PIOS_Flash_Jedec_WritePage(256, (uint8_t *) &header, 0 , sizeof(header)) != 0)
			return -3;
	if(PIOS_Flash_Jedec_WritePage(512, (uint8_t *) &header, 0 , sizeof(header)) != 0)
			return -3;
*/
//PIOS_LED_Off(PIOS_LED_ALARM);
	return 0;
}

/**
 * @brief Erase the whole flash chip and create the file system
 * @return 0 if successful, -1 if not
 */
int32_t PIOS_FLASHFS_Format() 
{

//PIOS_LED_On(PIOS_LED_RED);
	if(PIOS_Flash_Jedec_EraseChip() != 0){
		return -1;
	}

	if(PIOS_FLASHFS_ClearObjectTableHeader() != 0)
		return -1;
//	PIOS_LED_Off(PIOS_LED_RED);
	return 0;
}

/**
 * @brief Erase the headers for all objects in the flash chip
 * @return 0 if successful, -1 if not
 */
static int32_t PIOS_FLASHFS_ClearObjectTableHeader()
{
	uint32_t object_table_magic;
//	PIOS_Flash_Jedec_PageToBuffer(1, 511); // read page 511 to buffer 1 ( fill buffer with 0xff )
	PIOS_Flash_Jedec_ReadPage(511, 0, (uint8_t *)&object_table_magic, sizeof(object_table_magic));

//PIOS_LED_On(PIOS_LED_GREEN);

	if (PIOS_Flash_Jedec_WritePage(0, 0, (uint8_t *)&cfg->table_magic, sizeof(cfg->table_magic)) != 0){
//		PIOS_LED_On(PIOS_LED_GREEN);
		return -1;
}

	PIOS_Flash_Jedec_ReadPage(0, 0, (uint8_t *)&object_table_magic, sizeof(object_table_magic));
	if(object_table_magic != cfg->table_magic){
//		PIOS_LED_On(PIOS_LED_YELLOW);
		return -1;
		}
//	PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
	return 0;
}

/**
 * @brief Get the address of an object
 * @param obj UAVObjHandle for that object
 * @parma instId Instance id for that object
 * @return address if successful, -1 if not found
 */
static int32_t PIOS_FLASHFS_GetObjAddress(uint32_t objId, uint16_t instId)
{
	struct objectHeader header;
	// Read Page 1 -  Table of objects
	//if (PIOS_Flash_Jedec_ReadData(1, (uint8_t *)&AT45_buffer, sizeof(AT45_buffer)) != 0)
//	if (PIOS_Flash_Jedec_ReadPage(1, 0, (uint8_t *)&AT45_buffer, sizeof(AT45_buffer)) != 0)
//				return -1;
//	PIOS_Flash_Jedec_PageToBuffer(1,1); // buffer 1 , page 1

	int32_t addr = 0x00;
	while(addr < 512) {
		if (PIOS_Flash_Jedec_ReadPage(1, addr, (uint8_t *)&header, sizeof(header)) != 0)
						return -1;
/*
		header.objMagic = (AT45_buffer[addr] << 48) + (AT45_buffer[addr+1] << 32) + (AT45_buffer[addr+2] << 16) + (AT45_buffer[addr+3]);
		header.objId = (AT45_buffer[addr+4] << 48) + (AT45_buffer[addr+5] << 32) + (AT45_buffer[addr+6] << 16) + (AT45_buffer[addr+7]);
		header.instId = (AT45_buffer[addr+8] << 48) + (AT45_buffer[addr+9] << 32) + (AT45_buffer[addr+10] << 16) + (AT45_buffer[addr+11]);
		header.address = (AT45_buffer[addr+12] << 48) + (AT45_buffer[addr+13] << 32) + (AT45_buffer[addr+14] << 16) + (AT45_buffer[addr+15]);
*/
		if(header.objMagic != cfg->obj_magic)
			break; // stop searching once hit first non-object header
		else if (header.objId == objId && header.instId == instId)
			break;
		addr += sizeof(header);
	}

	if (header.objId == objId && header.instId == instId){
//		printf(" H ");
		return header.address;
	}
//	printf(" -1 ");
	return -1;
}

/**
 * @brief Returns an address for a new object and creates entry into object table
 * @param[in] obj Object handle for object to be saved
 * @param[in] instId The instance id of object to be saved
 * @return 0 if success or error code
 * @retval -1 Object not found
 * @retval -2 No room in object table
 * @retval -3 Unable to write entry into object table
 * @retval -4 FS not initialized
 * @retval -5
 */
int32_t PIOS_FLASHFS_GetNewAddress(uint32_t objId, uint16_t instId)
{
	struct objectHeader header;

	if(numObjects < 0)
		return -4;
	// read page 1 to buffer
	PIOS_Flash_Jedec_PageToBuffer(1, 0x01); // read buffer 1 , page 1 (Table of objects)

	/*
	if (PIOS_Flash_Jedec_ReadPage(1, 0x00, (uint8_t *)&header, sizeof(header)) != 0)
					return -1;
	*/
	// Don't worry about max size of flash chip here, other code will catch that
	header.objMagic = cfg->obj_magic;
	header.objId = objId;
	header.instId = instId;
//	header.address = cfg->obj_table_end + cfg->sector_size * numObjects;
	header.address = 2 + numObjects; // Page address for this objID

	//int32_t addr = cfg->obj_table_start + sizeof(header) * numObjects;
	// New table offset address for this objID
	int32_t addr = 0x00 + sizeof(header) * numObjects;
	// No room for this header in object table
	if((addr + sizeof(header)) > cfg->obj_table_end)
		return -2;

	// Verify the address is within the chip
//	if((addr + cfg->sector_size) > cfg->chip_size)
//		return -5;
	// write new header data to buffer
	/*
	if(PIOS_Flash_Jedec_BufferWrite(1, addr ,(uint8_t *) &header,  sizeof(header)) != 0)
				return -3;
	*/
	PIOS_Flash_Jedec_BufferWrite(1, addr ,(uint8_t *) &header,  sizeof(header));
	PIOS_Flash_Jedec_BufferToPage(1,1);
	/*
	if(PIOS_Flash_Jedec_WritePage(1, addr , (uint8_t *) &header, sizeof(header)) != 0)
		return -3;
*/
	// This numObejcts value must stay consistent or there will be a break in the table
	// and later the table will have bad values in it
	numObjects++;
	printf(" obj %d Ha %d",(uint8_t )numObjects , (uint8_t )header.address);
	/*
	printf(" Tm ");
	for(uint16_t i = 0; i < 6; i ++) {
		printf(" %02x %02x %02x %02x",
				(uint8_t )(object_table_magic>>24),
				(uint8_t )(object_table_magic>>16),
				(uint8_t )(object_table_magic>>8),
				(uint8_t )object_table_magic
		);}
	*/
	return header.address;
}


/**
 * @brief Saves one object instance per sector
 * @param[in] obj UAVObjHandle the object to save
 * @param[in] instId The instance of the object to save
 * @return 0 if success or -1 if failure
 * @note This uses one sector on the flash chip per object so that no buffering in ram
 * must be done when erasing the sector before a save
 */
int32_t PIOS_FLASHFS_ObjSave(UAVObjHandle obj, uint16_t instId, uint8_t * data)
{
	uint32_t objId = UAVObjGetID(obj);
	uint8_t crc = 0;
/*
	printf(" Ob %02x%02x%02x%02x",
					(uint8_t )(objId>>24),
					(uint8_t )(objId>>16),
					(uint8_t )(objId>>8),
					(uint8_t )objId
			);

	printf(" In %02x%02x%02x%02x",
					(uint8_t )(instId>>24),
					(uint8_t )(instId>>16),
					(uint8_t )(instId>>8),
					(uint8_t )instId
			);
*/

//	struct objectHeader header;
	// Read Page 1 -  Table of objects
	//if (PIOS_Flash_Jedec_ReadData(1, (uint8_t *)&AT45_buffer, sizeof(AT45_buffer)) != 0)
//	if (PIOS_Flash_Jedec_ReadPage(1, 0, (uint8_t *)&AT45_buffer, sizeof(AT45_buffer)) != 0)
//				return -1;


	if(PIOS_Flash_Jedec_StartTransaction() != 0) // Grab the semaphore to perform a transaction
		return -1;

	int32_t addr = PIOS_FLASHFS_GetObjAddress(objId, instId);

	// Object currently not saved
	if(addr < 0)
		addr = PIOS_FLASHFS_GetNewAddress(objId, instId);

	// Could not allocate a sector
	if(addr < 0) {
		PIOS_Flash_Jedec_EndTransaction();
		return -1;
	}
//PIOS_LED_On(PIOS_LED_RED);

	struct fileHeader header = {
		.id = objId,
		.instId = instId,
		.size = UAVObjGetNumBytes(obj)
	};
/*
	printf(" size %02x %02x %02x %02x ",
			(uint8_t )(header.size >>24),
			(uint8_t )(header.size>>16),
			(uint8_t )(header.size>>8),
			(uint8_t )header.size
	);
*/
	// Update CRC
	crc = PIOS_CRC_updateCRC(0, (uint8_t *) &header, sizeof(header));
	crc = PIOS_CRC_updateCRC(crc, (uint8_t *) data, UAVObjGetNumBytes(obj));
//PIOS_LED_On(PIOS_LED_YELLOW);

	PIOS_Flash_Jedec_PageToBuffer(1, 511); // read page 511 to buffer 1 ( fill buffer with 0xff )

//PIOS_LED_On(PIOS_LED_GREEN);
	struct pios_flash_chunk chunks[3] = {
		{
			.addr = (uint8_t *) &header,
			.len = sizeof(header),
		},
		{
			.addr = (uint8_t *) data,
			.len = UAVObjGetNumBytes(obj)
		},
		{
			.addr = (uint8_t *) &crc,
			.len = sizeof(crc)
		}
	};

	if(PIOS_Flash_Jedec_WriteChunks(addr, chunks, NELEMENTS(chunks)) != 0) {
		PIOS_Flash_Jedec_EndTransaction();
	   return -1;
	}
//PIOS_LED_On(PIOS_LED_ALARM);
	if(PIOS_Flash_Jedec_EndTransaction() != 0) {
		PIOS_Flash_Jedec_EndTransaction();
		return -1;
	}

	return 0;
}

/**
 * @brief Load one object instance per sector
 * @param[in] obj UAVObjHandle the object to save
 * @param[in] instId The instance of the object to save
 * @return 0 if success or error code
 * @retval -1 if object not in file table
 * @retval -2 if unable to retrieve object header
 * @retval -3 if loaded data instId or objId don't match
 * @retval -4 if unable to retrieve instance data
 * @retval -5 if unable to read CRC
 * @retval -6 if CRC doesn't match
 * @note This uses one sector on the flash chip per object so that no buffering in ram
 * must be done when erasing the sector before a save
 */
int32_t PIOS_FLASHFS_ObjLoad(UAVObjHandle obj, uint16_t instId, uint8_t * data)
{
	uint32_t objId = UAVObjGetID(obj);
	uint16_t objSize = UAVObjGetNumBytes(obj);
	uint8_t crc = 0;
	uint8_t crcFlash = 0;
	const uint8_t crc_read_step = 8;
	uint8_t crc_read_buffer[crc_read_step];

	if(PIOS_Flash_Jedec_StartTransaction() != 0)
		return -1;

	int32_t addr = PIOS_FLASHFS_GetObjAddress(objId, instId);

	// Object currently not saved
	if(addr < 0) {
		PIOS_Flash_Jedec_EndTransaction();
		return -1;
	}

	struct fileHeader header;

	// Load header
	// This information IS redundant with the object table id.  Oh well.  Better safe than sorry.
//	if(PIOS_Flash_Jedec_ReadData(addr, (uint8_t *) &header, sizeof(header)) != 0) {
	if(PIOS_Flash_Jedec_ReadPage(addr, 0, (uint8_t *) &header, sizeof(header)) != 0) {
		PIOS_Flash_Jedec_EndTransaction();
		return -2;
	}
	
	// Update CRC
	crc = PIOS_CRC_updateCRC(0, (uint8_t *) &header, sizeof(header));

	if((header.id != objId) || (header.instId != instId)) {
		PIOS_Flash_Jedec_EndTransaction();
		return -3;
	}
	
	// To avoid having to allocate the RAM for a copy of the object, we read by chunks
	// and compute the CRC
	for(uint32_t i = 0; i < objSize; i += crc_read_step) { // 8 byte step
		PIOS_Flash_Jedec_ReadPage(addr , sizeof(header) + i, crc_read_buffer, crc_read_step); // read 8 byte of data
		uint8_t valid_bytes = ((i + crc_read_step) >= objSize) ? objSize - i : crc_read_step;
		crc = PIOS_CRC_updateCRC(crc, crc_read_buffer, valid_bytes);
	}

	// Read CRC (written so will work when CRC changes to uint16)
	if(PIOS_Flash_Jedec_ReadPage(addr , sizeof(header) + objSize, (uint8_t *) &crcFlash, sizeof(crcFlash)) != 0) {
		PIOS_Flash_Jedec_EndTransaction();
		return -5;
	}

	if(crc != crcFlash) {
		PIOS_Flash_Jedec_EndTransaction();
		return -6;
	}

	// Read the instance data
	if (PIOS_Flash_Jedec_ReadPage(addr , sizeof(header), data, objSize) != 0) {
		PIOS_Flash_Jedec_EndTransaction();
		return -4;
	}

	if(PIOS_Flash_Jedec_EndTransaction() != 0)
		return -1;

	return 0;
}

/**
 * @brief Delete object from flash
 * @param[in] obj UAVObjHandle the object to save
 * @param[in] instId The instance of the object to save
 * @return 0 if success or error code
 * @retval -1 if object not in file table
 * @retval -2 Erase failed
 * @note To avoid buffering the file table (1k ram!) the entry in the file table
 * remains but destination sector is erased.  This will make the load fail as the
 * file header won't match the object.  At next save it goes back there.
 */
int32_t PIOS_FLASHFS_ObjDelete(UAVObjHandle obj, uint16_t instId)
{
	uint32_t objId = UAVObjGetID(obj);

	int32_t addr = PIOS_FLASHFS_GetObjAddress(objId, instId);

	// Object currently not saved
	if(addr < 0)
		return -1;

	if(PIOS_Flash_Jedec_ErasePage(addr) != 0)
		return -2;

	return 0;
}
