/** ********************************************************************************
 * * @file    jad.hpp
 * * @author  jaddina
 * * @date    Mar 29, 2025
 * * @brief ******************************************************************************** */

#ifndef SOARFS_FATDRIVER_INC_JAD_HPP_
#define SOARFS_FATDRIVER_INC_JAD_HPP_

/************************************ * INCLUDES ************************************/

#include "ff_error.h"
#include "ff_fat.h"
#include "FreeRTOS_errno_FAT.h"
#include "ff_stdio.h"

/************************************ * MACROS AND DEFINES ************************************/

#define SECTOR_SIZE     512		//number of bytes in each sector
#define NUM_SECTORS     128		//number of sectors in the file-system
#define CACHE_SIZE (4 * SECTOR_SIZE)	//cache size in bytes

/************************************ * TYPEDEFS ************************************/
/************************************ * CLASS DEFINITIONS ************************************/
class FAT_FS{
public:
	enum FATFS_ERR : uint8_t {

		FAT_FS_OK = 0,  	// file-system OK
		MOUNT_ERROR = 1, 	// error while mounting file-system, or file system is already mounted
		OPEN_ERROR = 2, 	// error while opening file
		CLOSE_ERROR = 3, 	// error while closing file or file already closed
		UNMOUNT_ERROR = 4,	// nothing to unmount
		DIRECTORY_ERROR = 5 // error while making removing or changing directory


	};

	static FAT_FS* getFAT_FS();


	FATFS_ERR unmount();
	FATFS_ERR write(const char *filepath, const void *pvBuffer, uint32_t datasize);
	FATFS_ERR read(const char *filepath, const void *pvBuffer, uint32_t datasize);
	FATFS_ERR makeDirectory(const char *filepath);
	FATFS_ERR removeDirectory(const char *filepath);
	FATFS_ERR changeDirectory(const char *filepath);


	bool isReady();

private:
	FAT_FS(); 		// prevent construction
	FAT_FS(FAT_FS&); 	//prevent copy construction
	FAT_FS& operator= (FAT_FS&); 	//prevent assignment

	FATFS_ERR mount();
	FATFS_ERR unmount();

	static FAT_FS filesystem; 	//singular file-system object

	//state variables
	bool mounted;
	static bool instantiated;

	FF_Disk_t* disk = nullptr;
	uint8_t *pucDataBuffer;

};
/************************************ * FUNCTION DECLARATIONS ************************************/
#endif
/* SOARFS_FATDRIVER_INC_JAD_HPP_ */
