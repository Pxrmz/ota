
/*!
 *******************************************************************************
 * \file	aADM_memory.h
 *
 * \brief	Memory Management APIs
 *******************************************************************************
 */
#ifndef _ADM_MEMORY_H_
#define _ADM_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @addtogroup platform_system
 * @{
 */
#define ADM_DBG_MEM

/*!
 *******************************************************************************
 * clean memory value.
 *
 * \param	buf		    Pointer to first byte in memory to be set
 * \param	size		memory size
 *
 * \return	None
 *******************************************************************************
 */
extern void *adm_memzero(void *buf, DSIZE size);


/*!
 *******************************************************************************
 * Set a byte sequence to a common value.
 *
 * \param	dest		Pointer to first byte in memory to be set
 * \param	val			Value to which bytes will be set
 * \param	count		Number of bytes to set
 * \param	dstLen		Size of destination buffer
 *
 * \return	None
 *******************************************************************************
 */
extern void *adm_memset(void *dest, D32S val, DSIZE count, DSIZE dstLen);

/*!
 *******************************************************************************
 * Copy bytes from one place to another. The source and destination bytes
 * must not overlap.
 *
 * \param	dest		Destination pointer
 * \param	src		Source pointer
 * \param	srcLen		Number of bytes to copy
 * \param	dstLen		Size of destination buffer
 *
 * \return	NULL or dest pointer
 *******************************************************************************
 */
extern void *adm_memcpy(void *dest, const void *src, DSIZE srcLen, DSIZE dstLen);

/*!
 *******************************************************************************
 * Compare two buffers of bytes.
 *
 * \param	inBuf1		The first memory buffer
 * \param	inBuf2		The second memory buffer
 * \param	inLen		Number of bytes to compare
 *
 * \return	< 0 if \a inBuf1 < \a inBuf2, 
 * 			  0 if \a inBuf1 == \a inBuf2, or
 * 			> 0 if \a inBuf1 > \a inBuf2
 *******************************************************************************
 */
extern D32S adm_memcmp(const void *inBuf1, const void *inBuf2, D32U inLen);

/*!
 *******************************************************************************
 * Copy bytes from one place to another. The source and destination bytes
 * may overlap.
 *
 * \param	dest		Destination pointer
 * \param	src		Source pointer
 * \param	srcLen		Number of bytes to copy
 * \param	dstLen		Size of destination buffer
 *
 * \return	None
 *******************************************************************************
 */
extern void *adm_memmove(void *dest, const void *src, DSIZE srcLen, DSIZE dstLen);

/*! @} */

extern void *adm_memdup(void *buf, D32U len);

extern void *__adm_malloc(DSIZE size, const DCHAR *funcname, D32U line);

extern void __adm_free(void *pp, const DCHAR *funcname, D32U line);

//Macro to free a non-NULL pointer and set it to NULL.


/*!
*******************************************************************************
 * Locate character in block of memory.
 * Searches within the first count bytes of the block of memory pointed by buf for the first occurrence 
 * of value (interpreted as an unsigned char), and returns a pointer to it.
 * \param	buf 	Pointer to the block of memory where the search is performed.
 * \param	ch		Value to be located. The value is passed as an int, but the function performs 
 *					a byte per byte search using the unsigned char conversion of this value.
 * \param	count	Number of bytes to be analyzed.
 *
 * \return	NULL if buf == NULL || count == 0
 *		  A pointer to the first occurrence of value in the block of memory pointed by buf.
 *		  If the value is not found, the function returns a null pointer.
*******************************************************************************
 */
extern void *adm_memchr(const void *buf, int ch, DSIZE count);

/*!
 *******************************************************************************
 * malloc ram space
 *
 * \param	size	want to malloc bytes number
 *
 * \return	NULL or dest pointer
 *******************************************************************************
 */
#define adm_malloc(size) __adm_malloc(size, __FUNCTION__, __LINE__)

/*!
 *******************************************************************************
 * free ram space which malloced by adm_malloc
 *
 * \param	void **pp	pointer to address of memry
 *
 * \return	NON
 *******************************************************************************
 */
//#define adm_free(pp) __adm_free(&(pp), __FUNCTION__, __LINE__) //__FUNCTION__
#define adm_free(ioPtr)	\
	do {								\
		if (NULL != (ioPtr))					\
		{								\
			__adm_free((ioPtr), __FUNCTION__, __LINE__);\
			(ioPtr) = NULL;				\
		}								\
	} while ( FALSE == TRUE )

/*!
 *******************************************************************************
 * For debug info function call back, need set by adm_memlog_set
 * If need obersve memory malloc info, need exec adm_memdbg_view
 *
 *******************************************************************************
 */
typedef void (*ADM_MEMDBG_LOG_CB)(const char *format, ...);
extern void adm_memlog_set(ADM_MEMDBG_LOG_CB logfunc);
extern DBOOL adm_memdbg_view(void);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif

