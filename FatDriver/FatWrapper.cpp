 /** ********************************************************************************
  * * @file    FatWrapper.cpp * @author  jaddina
  * * @date    Mar 29, 2025
  * *  @brief **********
  * ********************************************************************** */
/************************************ * INCLUDES ************************************/
#include "jad.hpp"
#include "ff_ramdisk.h"
#include "Mutex.hpp"
/************************************ * PRIVATE MACROS AND DEFINES ************************************/
/************************************ * VARIABLES ************************************/
bool FAT_FS::instantiated = false;
FAT_FS FAT_FS::filesystem;
/************************************ * FUNCTION DECLARATIONS ************************************/
/************************************ * FUNCTION DEFINITIONS ************************************/

Mutex fat_fs = Mutex();

/*
 * @brief 	initializes and allocates memory for file-system
 */

FAT_FS::FAT_FS() : mounted(false){
	if(instantiated){

		SOAR_PRINT("Cannot have more than 1 file-system mounted\n");
		return;
	}

	instantiated = true;

	pucDataBuffer = pvPortMalloc(SECTOR_SIZE * NUM_SECTORS); // allocates memory the size of the partition file-system is mounted on

}

FAT_FS* FAT_FS::getFAT_FS(){

	SOAR_ASSERT(instantiated);

	return &filesystem;
}

/*
 * @brief	creates disk(partition) and mounts the Fat_Fs file-system onto partition
 * @return	returns FAT_FS_OK meaning file system mounted without an error
 * @return  error: MOUNT_FAILED meaning there was a mounting error
 */

FAT_FS::FATFS_ERR FAT_FS::mount(){


	disk = FF_RAMDiskInit("SoarFAT_FS", pucDataBuffer, NUM_SECTORS, CACHE_SIZE);	//creates partition and mounts file-system onto partition


	if(disk != NULL){
		mounted = true;

		return FAT_FS_OK;
	}

	mounted = false;

	return MOUNT_FAILED;


}

/*
 * @brief	unmounts the file system from the disk partition
 * @return	error: UNMOUNT_ERROR which means that there is nothing mounted to the existing disk, or no existing disk
 * @return	FAT_FS_OK which means that the file-system was unmounted successfully
 */
FAT_FS::FATFS_ERR FAT_FS::unmount(){

	if(!mounted || disk == nullptr){

		return UNMOUNT_ERROR; // no disk is needed to be unmounted
	}

	FF_Unmount(disk);
	disk = nullptr;
	mounted = false;
	return FAT_FS_OK;
}

/*
 * @brief writes data to existing file within the file-system
 * @param filepath: directory path of file to be opened
 * @param pvBuffer: buffer of the data which is to be written to the file
 * @param datasize: size of data in bytes which is being written to the file
 * @return error: MOUNT_FAILED means there was an mounting error
 * @return error: OPEN_ERROR there was an error opening file
 * @return error: CLOSE_ERR there was and error closing file
 * @return FAT_FS_OK no error Fat_Fs is okay
 */

FAT_FS::FATFS_ERR FAT_FS::write(const char *filepath, const void *pvBuffer, uint32_t datasize){

	fat_fs.Lock();	//mutex in place so file-system can only do one task at a time

	uint32_t err = mount(); //mounts file-system

	//error check
	if(err == 1){
		return MOUNT_FAILED;
	}

	FF_FILE* file = ff_fopen(filepath, "w"); //open file

	//error check
	if(file == NULL){
		return OPEN_ERROR;
	}

	ff_fwrite(pvBuffer, 1, datasize, file);	//write to file

	err = ff_close(file);	//close file

	//error check
	if(err < 0){
		return CLOSE_ERR;
	}

	unmount();	//unmount file-system from disk

	fat_fs.Unlock(); //mutex unlocks

	return FAT_FS_OK;
}

/*
 * @brief reads data from file opened within the file-system
 * @param filepath: directory path of file to be opened
 * @param pvBuffer: buffer of the data (where data is stored) which is to be read from the file
 * @param datasize: size of data in bytes which is being read from the file
 * @return error: MOUNT_FAILED means there was an mounting error
 * @return error: OPEN_ERROR there was an error opening file
 * @return error: CLOSE_ERR there was and error closing file
 * @return FAT_FS_OK no error Fat_Fs is okay
 */

FAT_FS::FATFS_ERR FAT_FS::read(const char *filepath, const void *pvBuffer, uint32_t datasize){

	fat_fs.Lock(); //mutex in place so file-system can only do one task at a time

	uint32_t err = mount(); //mounts file-system to parition

	//error check
	if(err == 1){
		return MOUNT_FAILED;
	}

	FF_FILE* file = ff_fopen(filepath, "r"); //opens file

	//error check
	if(file == NULL){
		return OPEN_ERROR;

	}

	ff_fread(pvBuffer, 1, datasize, file); //reads file

	err = ff_close(file); //close file

	//error check
	if(err < 0){
		return CLOSE_ERR;
	}

	unmount(); // unmount file-system from partition

	fat_fs.Unlock(); // mutex unlocks

	return FAT_FS_OK;

}

/*
 * @brief makes a directory in the file-system at the given filepath
 * @param filepath: path to the point in the file-system where you want to create the directory
 * @return FAT_FS_OK no error Fat_Fs is okay
 * @return error: DIRECTORY_ERROR directory could not be created
 */

FAT_FS::FATFS_ERR FAT_FS::makeDirectory(const char *filepath){

	uint_32 err = ff_mkdir(filepath); //makes directory

	//error check
	if (err == -1){
		return DIRECTORY_ERROR;
	}

	return FAT_FS_OK;

}

/*
 * @brief removes directory at a point in the file-system
 * @param filepath: path to the point in the file-system where directory is to be removed
 * @return FAT_FS_OK no error Fat_Fs is okay
 * @return error: DIRECTORY_ERROR directory could not be removed
 */

FAT_FS::FATFS_ERR FAT_FS::removeDirectory(const char *filepath){

	uint_32 err = ff_rmdir(filepath);

	if (err == -1){
		return DIRECTORY_ERROR;
	}

	return FAT_FS_OK;

}
/*
 * @brief changes directory to another directory provided the filepath
 * @param filepath: path to the point in the file-system where directory is to be changed to
 * @return FAT_FS_OK no error Fat_Fs is okay
 * @return error: DIRECTORY_ERROR directory could not be changed
 */
FAT_FS::FATFS_ERR FAT_FS::changeDirectory(const char *filepath){

	uint_32 err = ff_chdir(filepath);

	if (err == -1){
		return DIRECTORY_ERROR;
	}

	return FAT_FS_OK;
}
/*
 * @brief checks if file-system is ready to used, meaning file system is not busy with a read or write
 * @return returns true if the file-system is ready to use
 * @return returns false if the file-system is not ready to use
 */

bool FAT_FS::isReady(){


	if(fat_fs.Lock(0))	//returns true if mutex is able to lock
	{
		fat_fs_Unlock();	//immediately unlocks mutex

		return true;
	}

	else{

		return false;
	}

 }


