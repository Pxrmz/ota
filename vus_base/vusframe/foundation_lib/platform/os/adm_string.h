
/*!
 *******************************************************************************
 * \file	aADM_string.h
 *
 * \brief	External String Handling APIs
 *******************************************************************************
 */

#ifndef adm_STRING_H
#define adm_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <adm_typedefs.h>

/*!
 * @ platform
 */

/*!
 *******************************************************************************
 * Calculate the length of a character string (not including the terminating
 * NULL).
 *
 * \param	inString		The string
 *
 * \return	The length of the string, in bytes
 *******************************************************************************
 */
extern D32U adm_strlen(const char *inString);

/*!
*******************************************************************************
 * Calculates the length of a null terminated byte character string with  protected max len.
 *
 * \param   string      The string.
 * \param   max_len     The max length of string.
 *
 * \return  The length of the string in bytes.
*******************************************************************************
 */
extern D32U adm_strnlen(const char *str, D32U max_len);


/*!
 *******************************************************************************
 * Get the position of the first occurrence of a character in a character string.
 *
 * \param	inString	The string
 * \param	inChar		The character
 *
 * \return	A pointer to the first occurrence of the character in the string,
 *			or NULL if the character is not found
 *******************************************************************************
*/
extern char *adm_strchr(const char *inString, char inChar);


/*!
 *******************************************************************************
 * Get the position of the last occurrence of a character in a character string.
 *
 * \param	inString	The string
 * \param	inChar		The character
 *
 * \return	A pointer to the last occurrence of the character in the string,
 *			or NULL if the character is not found
 *******************************************************************************
 */
extern char *adm_strrchr(const char *inString, char inChar);


/*!
 *******************************************************************************
 * Compare two character strings.
 *
 * \param	inString1	The first string
 * \param	inString2	The second string
 *
 * \return	< 0 if \a inString1 is lexically less than \a inString2, 
 * 			  0 if \a inString1 is lexically the same as \a inString2, or
 * 			> 0 if \a inString1 is lexically greater than \a inString2
 *******************************************************************************
 */
extern D32S adm_strcmp(const char *inString1, const char *inString2);


/*!
 *******************************************************************************
 * Partially compare two character strings. Characters that follow a NULL
 * character are not compared.
 *
 * \param	inString1		The first string
 * \param	inString2		The second string
 * \param	inLen			Number of characters to compare
 *
 * \return	Considering only the number of characters compared:
 * 			< 0 if \a inString1 is lexically less than \a inString2,
 * 			  0 if \a inString1 is lexically equal to \a inString2, or
 * 			> 0 if \a inString1 is lexically greater than \a inString2
 *******************************************************************************
 */
extern D32S adm_strncmp(const char *inString1, const char *inString2, D32U inLen);

/*!
*******************************************************************************
 * Copy a null terminated byte character string to the other null
 * terminated byte character string.
 *
 * \param   outDest      The destination string.
 * \param   inSrc        The source string.
 * \param   dstLen       Size of the destination buffer.
 *
 * \return  pointer the destination string outDest. �ú�������һ��ָ��Ŀ��洢��outDest��ָ��
 *          
*******************************************************************************/
DCHAR *adm_strcpy(DCHAR *outDest, const DCHAR *inSrc, DSIZE dstLen);

/*!
*******************************************************************************
 * Copy src to dest with checking of destination buffer overflow
 * terminated byte character string.
 *
 * \param   inSrc        The source string.
 * \param   outDest      The destination string.
 * \param   srcLen       Size of the source string which copy. Ҫ��Դ�и��Ƶ��ַ���
 * \param   dstLen       Size of the destination string buffer. 
 *
 * \return  NULL if copy fail
 * \return  dest pointer if success
 *          
*******************************************************************************/
DCHAR *adm_strncpy(DCHAR *outDest, const DCHAR *inSrc, DSIZE srcLen, DSIZE dstLen);

//extern char *adm_strcpy(char *outDest, const char *inSrc);


/*!
*******************************************************************************
 * Copy src to dest with checking of destination buffer overflow
 * terminated byte character string.
 *
 * \param   inSrc        The source string.
 * \param   outDest      The destination string.
 * \param   srcLen       Size of the source string which copy. Ҫ��Դ�и��Ƶ��ַ���
 * \param   dstLen       Size of the destination string buffer. 
 *
 * \return  0 if copy fail
 * \return  Size of copy string
 *          
*******************************************************************************
*/
DSIZE adm_strlcpy(DCHAR *outDest, DCHAR *inSrc, DSIZE srcLen, DSIZE dstLen);
//extern D32U adm_strlcpy(char *outDest, const char *inSrc, D32U inSize);

/*!
*******************************************************************************
 * Concatenate a null terminated byte character string to the specified null
 * terminated byte character string.
 *
 * \param   ioDest      The destination string.
 * \param   inSrc       The source string to concatenate.
 * \param   dstLlen     The destination buf size include dest and source string total length+1.
 *
 * \return  A pointer to the destination null terminated byte character string.
*******************************************************************************
 */ 
DCHAR *adm_strcat (DCHAR *ioDest, const DCHAR *append, DSIZE dstlen);

/*!
*******************************************************************************
 * Concatenate a null terminated byte character string to the specified null
 * terminated byte character string .
 * It will append at most size - strlen(ioDest) - 1 bytes, NUL-terminating the
 * result.
 * ��append��ָ�ַ�����ǰappendLen���ַ����ӵ�ioDest��ָ�ַ����Ľ�β����������ioDest��ָ
 * �ַ�����β��'\0'���Ӷ�ʵ���ַ���������
 * 
 * \param   ioDest     The destination string.
 * \param   append     The source string to concatenate. Ҫ׷�ӵ��ַ���
 * \param   appendLen  Size of source string to concatenate. Ҫ׷�ӵ��ַ�����
 * \param   dstLen     Size of destination buffer with null terminated char. 
 *                     Ŀ�껺������󳤶ȣ���������Ŀ��ַ���
 *
 * \return  The destination string pointer. return NULL if length verify not enough.
 		  ����һ��ָ�����յ�Ŀ���ַ���ioDest��ָ��,����У�鲻����Ҫ�󣬷���NULL
*******************************************************************************
*/
extern DCHAR *adm_strncat (DCHAR *ioDest, const DCHAR *append, DSIZE appendLen, DSIZE dstLen);

/*!
 *******************************************************************************
 * Compare two strings, ignoring case.
 *
 * \param	inString1		The first string
 * \param	inString2		The second string
 *
 * \return	Ignoring case:
 * 			< 0 if \a inString1 is lexically less than \a inString2, 
 * 			  0 if \a inString1 is lexically equal to \a inString2, or
 * 			> 0 if \a inString1 is lexically greater than \a inString2
 *******************************************************************************
 */
extern D32S adm_strcasecmp(const char *inString1, const char *inString2);

/*!
 *******************************************************************************
 * Partially compare two character strings, ignoring case. Characters that
 * follow a NULL character are not compared.
 *
 * \param	inString1		The first string
 * \param	inString2		The second string
 * \param	inLen			Number of characters to compare
 *
 * \return	Considering only the number of characters compared, and ignoring
 * 			case:
 * 			< 0 if \a inString1 is lexically less than \a inString2,
 * 			  0 if \a inString1 is lexically equal to \a inString2, or
 * 			> 0 if \a inString1 is lexically greater than \a inString2
*******************************************************************************
 */
extern D32S adm_strncasecmp(
	const char *inString1, const char *inString2, D32U inLen);

/*!
 *******************************************************************************
 * Locate a substring within a character string.
 *
 * \param	inHaystack	String within which to search
 * \param	inNeedle	String to search for within \a inHaystack
 *
 * \return	Pointer to the first occurrence of \a inNeedle within \a inHaystack
 * 			if it exists, or NULL if not found 
 *******************************************************************************
 */
extern char *adm_strstr(const char *inHaystack, const char *inNeedle);

/*!
 *******************************************************************************
 * Convert string to lower case.
 *
 * \param	ioString		The string to convert
 *
 * \return	Pointer to the converted string
 *******************************************************************************
 */
extern DCHAR *adm_strlwr(DCHAR *ioString);
extern int htoi(const char s[]);
/* @ */


/*!
 *******************************************************************************
 * Copy string from source str. Note not sercurity function, need you free by call adm_free
 * Please use adm_strdup
 * \param	str		The string to copy
 *
 * \return	Pointer to the destination string
 *******************************************************************************
 */
extern DCHAR *_adm_strdup(const DCHAR *str, const DCHAR *funcname, D32U line);

#define adm_strdup(str) _adm_strdup(str, __FUNCTION__, __LINE__)


D32S adm_snprintf(DCHAR *outBuffer, DSIZE inCount, const DCHAR *inFormat, ...);
DCHAR *adm_strjoin(DCHAR *inSeparator, ...);
unsigned long adm_strtoul(const char *inString, char **ioEndString, D8U inRadix, DBOOL *outIsSuccess);
D32U adm_atoD32U(const char *inString, D8U inRadix, DBOOL *outIsSuccess);
DBOOL adm_isspace(const char inChar);
void adm_D32Utoa(D32U value, char *string, D8U radix);

extern D32S adm_stringToInt32(const DCHAR* inString, D32S* outValue);
extern D32S adm_hexStringToInt32(const DCHAR* inHexString, D32S* outValue);
extern D32S adm_stringToUint32(const DCHAR* inString, D32U* outValue);
extern D32S adm_stringToInt64(const DCHAR* inString, D64S* outValue);
extern D32S adm_stringToUint64(const DCHAR* inString, D64U* outValue);
extern D32S adm_stringToFloat(const DCHAR* inString, DFLOAT* outValue);
extern D32S adm_stringToDouble(const DCHAR* inString, DOUBLE* outValue);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif

