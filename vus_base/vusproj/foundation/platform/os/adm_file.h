#ifndef _ADM_FILE_H_
#define _ADM_FILE_H_

#include "adm_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * File relative seek starting point
 */
typedef enum {
	E_ADM_FILE_SEEK_START,	///< Seek from the beginning of the file
	E_ADM_FILE_SEEK_CURRENT,	///< Seek from the current file pointer
	E_ADM_FILE_SEEK_END,		///< Seek back from the end of the file
} E_ADM_FILE_SEEK_T;

/*!
*******************************************************************************
 * Open a file , and return a file pointer to it 
 *
 * \param	inPath			The path of the requested file.
 * \param	inMode			A string with configuration for the fopen command.
 * \note					If other platforms are included this might need to
 * 							be changed to a specific parameter per file
 * 							permission flag.
 * \param	outFile			A reference to the file pointer, in which the file
 * 							pointer will be returned upon a successful call.
 *
 * \return  	ADM_ERR_OK upon a successful call,
 *			ADM_ERR_UNSPECIFIC if the type of the file could not be retrieved
 *			or ADM_ERR_BAD_INPUT if the file is a symbolic link.
*******************************************************************************
 */
ADM_ERROR adm_fopen(
		const char *path,
		const char *mode,
		void **outFile);

/*!
*******************************************************************************
 * Close a file pointed to by a file pointer (if it is not NULL), and set the
 * file pointer to null if successful.
 *
 * \param	outFile			A file pointer to the file.
 *
 * \return       	ADM_ERR_OK upon a successful call,
 *			or ADM_ERR_UNSPECIFIC if the file couldn't be closed.
*******************************************************************************
 */
ADM_ERROR adm_fclose(void **outFlile);

/*!
*******************************************************************************
 * Sync the file (write all data that is still not written).
 * parent directory is also suggested.
 *
 * \param	inFile			A file pointer to the file.
 *
 * \return  	ADM_ERR_OK upon a successful call,
 *			or ADM_ERR_UNSPECIFIC if the file couldn't be synced.
*******************************************************************************
 */
ADM_ERROR adm_fsync(void *inFile);

/*!
*******************************************************************************
 * Rename a file. 
 *
 * \param	oldpath      The old file name 
 * \param	newpath	   The new file name
 *
 * \return	       ADM_ERR_OK upon a successful call,
 *			or ADM_ERR_UNSPECIFIC if the file couldn't be renamed for any reason.
*******************************************************************************
 */
ADM_ERROR adm_frename(const char *oldpath, const char *newpath);
/*!
 *******************************************************************************
 * Seek to an offset in a file relative to a starting point:
 * - Forward from the start of the file
 * - Forward from the current file pointer
 * - Backwards from the end of the file
 *
 * After the operation is finished, the file pointer must be positioned at the
 * offset on success, the beginning of the file if searching backwards, or the
 * end of the file if searching forward.
 *
 * \param	inFile	       pointer to the file
 * \param	inOffset	       Offset to seek
 * \param	inFromWhere 	Seek starting point, an \ref E_DM_PL_FILE_Seek_t
 *						value
 *
 * \return	       ADM_ERR_OK on success, or ADM_ERR_STORAGE_SEEK otherwise
 *******************************************************************************
 */
ADM_ERROR adm_fseek(void * inFile, D32U inOffset, E_ADM_FILE_SEEK_T inFromWhere);

/*!
 *******************************************************************************
 * Write data to storage.
 *
 * After the write operation is finished, the file pointer must be
 * positioned at the end of the data written.
 *
 * \param	inDataBuffer		       Data to write.
 * \param	inDataLength		       The length of \a inData, in bytes
 * \param	outTotalWrittenSize		The size of the data which written into the file successfully.
 * \param	inFile		              Pointer to the file
 * 
 * \return	       ADM_ERR_OK on success, ADM_ERR_MEMORY if out of
 * 			memory, or ADM_ERR_STORAGE_WRITE if there is a problem writing
 *******************************************************************************
 */
ADM_ERROR adm_fwrite(const void * inDataBuffer, D32U inDataLength, D32U * outTotalWrittenSize, void *inFile);

/*!
 *******************************************************************************
 * Read data from storage.
 *
 * After the read operation is finished, the file pointer must be positioned
 * at the end of the data read.
 *
 * \param	outBuffer		       Buffer to store the read data 
 * \param	inBufferSize		The size of \a outBuffer, in bytes
 * \param	outTotalReadSize	Number of bytes actually read<br>
 *							This will be \a inBufSize unless there is
 *							no more data, in which case the value must be 0.
 * \param	inFile		       The file pointer
 * 
 * \return	       ADM_ERR_OK on success, ADM_ERR_MEMORY if out of
 * 			memory, or ADM_ERR_STORAGE_READ if there is a problem reading
 *******************************************************************************
 */
ADM_ERROR adm_fread(void * outBuffer, D32U inBufferSize, D32U * outTotalReadSize, void *inFile);

/*!
 *******************************************************************************
 * Delete one file .
 *
 * \param	inFileFullPathName		The name of the file
 *							 
 * \return	       ADM_ERR_OK on success,   ADM_ERR_STORAGE_REMOVE on failure
 *                  
 *******************************************************************************
 */
ADM_ERROR adm_fremove(const char* inFileFullPathName);

/*!
 *******************************************************************************
 * Get the size of one file .
 *
 * \param	inFileFullPathName		The name of the file
 * \param	outSize		              The pointer to the variable which hold the size of the file
 *							 
 * \return	       ADM_ERR_OK on success,   ADM_ERR_STORAGE_REMOVE on failure
 *                  
 *******************************************************************************
 */
ADM_ERROR adm_fgetfilesize(const char* inFileFullPathName,  D32U *outSize);

/*!
 *******************************************************************************
 * Get the maximum size available space for a download package. Before downloading a
 * package, DM verifies that there is enough storage space for it.
 *
 * \param	outMaxSize		The maximum size available for a download
 * 								package
 *
 * \return	       ADM_ERR_OK on success, or a \ref ADM_ERR_defs error code
 *******************************************************************************
 */
ADM_ERROR adm_fgetpkgmaxsize(D32U *outMaxSize);

/*!
 *******************************************************************************
 * Read a line from an open storage item.
 *
 * Read a line from the current position and append it, with a terminating null,
 * to \a outBuffer. After the read operation is finished, the file pointer
 * must be positioned at the end of the data read.
 *
 * \param	outBuffer		Pre-allocated buffer to store the string. A maximum
 *							of ( \a inBufferSize - 1 ) characters are read,
 *							stored in the buffer, and appended with a
 *							terminating null
 * \param	inBufferSize    The size of \a outBuffer
 * \param	inFile		The file pointer, its value is decided after the API DM_PL_file_fopen() is called
 *
 * \return	       The string, or
 *                  NULL if an error occurred or if there was nothing to read
 *******************************************************************************
 */
D8U *adm_fgets(void *outBuffer, D32U inBufferSize, void *inFile);
/*!
 *******************************************************************************
 * Check if the file exist.
 *
 * \param	outBuffer		Pre-allocated buffer to store the string. A maximum
 *							of ( \a inBufferSize - 1 ) characters are read,
 *							stored in the buffer, and appended with a 
 * \return	       TRUE,   the file exist
 *                  FALSE,  the file does not exist
 *******************************************************************************
 */
DBOOL  adm_fisexist(const char* inFileFullPathName);

/*!
 *******************************************************************************
 * Get the full path name of the update package file.
 * 
 * \param	outBuffer		       Buffer to store the update package file name 
 * \param	inBufferSize		The size of \a outBuffer, in bytes
 * \param	outSize            	The size(bytes) of the full path name of the update package file
 *							
 * \param	inURI			A URI string set by the caller; use this string to
 * 							create a file name, if desired
 * 
 * \return	       ADM_ERR_BUFFER_OVERFLOW if outBuffer == NULL or inBufferSize < *outSize,
 *                  ADM_ERR_OK on success, ADM_ERR_MEMORY if out of
 * 			memory, or ADM_ERR_STORAGE_READ if there is a problem
 *******************************************************************************
 */
ADM_ERROR adm_fgetpkgfullname(char * outBuffer, D32U inBufferSize, D32U * outSize, D8U *inURI);

/*!
 *******************************************************************************
 * Get the working directory of the DM engine.
 * 
 * DM engine need save/read some configuration or state information and the tree in this working directory,
 * The working directory is the path where the tree placed, like "C:\\dmc\\data\\" or "/system/app/dmc/data/"
 * 
 * \param	outBuffer		       Buffer to store the working directory 
 * \param	inBufferSize		The size of \a outBuffer, in bytes
 * \param	outSize            	The size(bytes) of the working directory
 * 
 * \return	       ADM_ERR_BUFFER_OVERFLOW if outBuffer == NULL or inBufferSize < *outSize,
 *                  ADM_ERR_OK on success, ADM_ERR_MEMORY if out of
 * 			memory, or ADM_ERR_STORAGE_READ if there is a problem
 *******************************************************************************
 */
ADM_ERROR adm_fgetworkingdir(char * outBuffer, D32U inBufferSize, D32U * outSize);

/*!
 *******************************************************************************
 * Get the tree file name. 
 * 
 * \param	outBuffer		       Buffer to store the tree  
 * \param	inBufferSize		The size of \a outBuffer, in bytes
 * \param	outSize            	The size(bytes) of the tree file name
 * 
 * \return	       ADM_ERR_BUFFER_OVERFLOW if outBuffer == NULL or inBufferSize < *outSize,
 *                  ADM_ERR_OK on success, ADM_ERR_MEMORY if out of
 * 			memory, or ADM_ERR_STORAGE_READ if there is a problem
 *******************************************************************************
 */
ADM_ERROR adm_fgetfilenameintree(char * outBuffer, D32U inBufferSize, D32U * outSize);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _ADM_FILE_H_ */

