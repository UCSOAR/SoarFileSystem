 /** ********************************************************************************
  * * @file    jad.cpp * @author  jaddina *
  *  @date    Mar 29, 2025 *
  *  @brief **********
  * ********************************************************************** */
/************************************ * INCLUDES ************************************/
#include "jad.hpp"
#include "ff_ramdisk.c"
#include "Mutex.hpp"
/************************************ * PRIVATE MACROS AND DEFINES ************************************/
#define SECTOR_SIZE     512
#define NUM_SECTORS     128
#define CACHE_SIZE (4 * SECTOR_SIZE)
/************************************ * VARIABLES ************************************/

/************************************ * FUNCTION DECLARATIONS ************************************/
/************************************ * FUNCTION DEFINITIONS ************************************/
bool FAT_FS::instantiated = false;
FAT_FS FAT_FS::filesystem;

Mutex fat_fs = Mutex();

FAT_FS::FAT_FS() : mounted(false){
	if(mounted){
		SOAR_PRINT("Cannot have more than 1 filesystem mounted\n");

		return;
	}
	pucDataBuffer = pvPortMalloc(SECTOR_SIZE * NUM_SECTORS);

}

FAT_FS* FAT_FS::getFAT_FS(){

	SOAR_ASSERT(instantiated);

	return &filesystem;
}

FAT_FS::FATFS_ERR FAT_FS::mount(){


	disk = FF_RAMDiskInit("SoarFAT_FS", pucDataBuffer, NUM_SECTORS, CACHE_SIZE);


	if(disk != NULL){
		mounted = true;

		return FAT_FS_OK;
	}

	mounted = false;

	return MOUNT_FAILED;


}

FAT_FS::FATFS_ERR FAT_FS::unmount(){

	if(!mounted || disk == nullptr){

		return UNMOUNT_ERROR; // no disk is needed to be unmounted
	}

	FF_Unmount(disk);
	disk = nullptr;
	mounted = false;
	return FAT_FS_OK;


}

FAT_FS::FATFS_ERR FAT_FS::write(const char *filepath, const void *pvBuffer, uint32_t datasize){

	fat_fs.Lock();

	uint32_t err = mount();

	if(err == 1){
		return MOUNT_FAILED;
	}

	FF_FILE* file = ff_fopen(filepath, "w");

	if(file == NULL){
		return OPEN_ERROR;
	}

	ff_fwrite(pvBuffer, 1, datasize, file);

	err = ff_close(file);

	if(err < 0){
		return CLOSE_ERR;
	}

	unmount();

	fat_fs.Unlock();

	return FAT_FS_OK;


}


FAT_FS::FATFS_ERR FAT_FS::read(const char *filepath, const void *pvBuffer, uint32_t datasize){

	fat_fs.Lock();

	uint32_t err = mount();

	if(err == 1){
		return MOUNT_FAILED;
	}

	FF_FILE* file = ff_fopen(filepath, "r");

	if(file == NULL){
		return OPEN_ERROR;

	}

	ff_fread(pvBuffer, 1, datasize, file);

	err = ff_close(file);

	if(err < 0){
		return CLOSE_ERR;
	}

	unmount();
	fat_fs.Unlock();

	return FAT_FS_OK;




}

bool FAT_FS::isReady(){
	if(fat_fs.Lock(0)){

		fat_fs_Unlock();

		return true;
	}

	else{

		return false;
	}

 }


