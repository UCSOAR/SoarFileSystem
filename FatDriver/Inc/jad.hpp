/** ******************************************************************************** * @file    jad.hpp * @author  jaddina * @date    Mar 29, 2025 * @brief ******************************************************************************** */
#ifndef SOARFS_FATDRIVER_INC_JAD_HPP_
#define SOARFS_FATDRIVER_INC_JAD_HPP_
/************************************ * INCLUDES ************************************/
#include "ff_error.c"
#include "ff_fat.c"
#include "FreeRTOS_errno_FAT.h"
#include "ff_stdio.c"
/************************************ * MACROS AND DEFINES ************************************/
/************************************ * TYPEDEFS ************************************/
/************************************ * CLASS DEFINITIONS ************************************/
class FAT_FS{
public:
	enum FATFS_ERR : uint8_t {

		FAT_FS_OK = 0,  	// file-system OK
		MOUNT_ERROR = 1, 	//
		OPEN_ERROR = 2, 	//
		CLOSE_ERROR = 3, 	//
		UNMOUNT_ERROR = 4	// nothing to unmount



	};

	static FAT_FS* getFAT_FS();


	//FATFS_ERR unmount();
	FATFS_ERR write();
	FATFS_ERR read();
	FATFS_ERR makeDirectory();
	FATFS_ERR changeDirectory();


	bool isReady();

private:
	FAT_FS();
	FAT_FS(FAT_FS&);
	FAT_FS& operator= (FAT_FS&);

	FATFS_ERR mount();
	FATFS_ERR unmount();

	static FAT_FS filesystem;

	FF_Disk_t* disk = nullptr;
	bool mounted;
	static bool isntantiated;
	uint8_t *pucDataBuffer;

};
/************************************ * FUNCTION DECLARATIONS ************************************/
#endif
/* SOARFS_FATDRIVER_INC_JAD_HPP_ */
