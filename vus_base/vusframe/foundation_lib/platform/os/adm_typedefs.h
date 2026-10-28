/**
 * @file abup_types.h
 * @brief VDI数据类型定义和相关宏申明。
 * @details  This is the detail description.
 * @author godtf
 * @version v0.1.0
 * @date 2021-08-15
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 *
 * @htmlonly
 * <span style="font-weight: bold">History</span>
 * @endhtmlonly
 * Version|Auther|Date|Describe
 * ------|----|------|--------
 * V0.1.0|godtf|2021-04-29|创建文件
 *
 * @note
 * ADM uses platform-independent versions of C built-in types and
 * defines to ensure portability. When implementing the Porting Layer code, do
 * not use C built-in types directly; always use these types.
 *
 * The following types and defines apply to most platforms. Specific definitions
 * may vary for some platforms. For example, on some platforms, <b>int</b> may
 * be defined as <b>D16S</b> rather than <b>D32S</b>. If the definitions do not
 * match your architecture, contact Software and request a special
 * version of the Framework to match your platform data types.
 */

#ifndef _ADM_TYPEDEFS_H_
#define _ADM_TYPEDEFS_H_ ///< Internal

/**
 * @defgroup pl_system	System types
 * @ingroup pl_system
 *
 * @defgroup  pl_macro	System macro
 * @ingroup pl_macro
 */

/** @defgroup pl_macros Sytem types
 *  @ingroup pl_system
 *	@{
 */

/*! The handle returned by different objects of the framework */
typedef void *adm_handle_t;

/* Is target platform a 64 bit platform */
/* TODO move to SBuild define, and define per platform, instead of detecting */
#if (defined _WIN64) || (defined __amd64__)
#define ADM_P64 ///< 64 bit processor
#else
#define ADM_P32 ///< 32 bit processor
#endif

/** x64 : 64bit / x86 : 32bit */

/** The handle returned by different objects of the framework */
typedef void DVOID;

#ifndef D64U_DEFINED
#define D64U_DEFINED
typedef unsigned long long int D64U; ///< 64bit integer (unsigned)
#endif

#ifndef D64S_DEFINED
#define D64S_DEFINED
typedef signed long long int D64S; ///< 64bit integer (signed)
#endif

#ifndef DFLOAT_DEFINED
#define DFLOAT_DEFINED
typedef float DFLOAT; ///< float
#endif

#ifndef DOUBLE_DEFINED
#define DOUBLE_DEFINED
typedef double DOUBLE; ///< double
#endif

#ifndef D32S_DEFINED
#define D32S_DEFINED
typedef int D32S; ///< 32bit integer
#endif

#ifndef D32U_DEFINED
#define D32U_DEFINED
typedef unsigned int D32U; ///< 32bit integer (unsigned)
#endif

#ifndef DUINT_DEFINED
#define DUINT_DEFINED         ///< Internal
typedef unsigned int D4UBYTE; ///< integer (unsigned)
#endif

#ifndef D16S_DEFINE
#define D16S_DEFINED
typedef short D16S; ///< 16bit integer
#endif

#ifndef D16U_DEFINED
#define D16U_DEFINED
typedef unsigned short D16U; ///< 16bit integer (unsigned)
#endif

#ifndef D8S_DEFINED
#define D8S_DEFINED
typedef signed char D8S; ///< 8bit integer
#endif

#ifndef D8U_DEFINED
#define D8U_DEFINED
typedef unsigned char D8U; ///< 8bit integer (unsigned)
#endif

#ifndef DCHAR_DEFINED
#define DCHAR_DEFINED
typedef char DCHAR; ///< 8bit integer (unsigned)
#endif

#ifndef TIME_T_DEFINED
#define TIME_T_DEFINED ///< Internal
#ifdef ADM_P64
typedef D64U TIME_T; ///< Time on a 64 bit computer
#else
typedef D32U TIME_T; ///< Time on a 32 bit computer
#endif
#endif

#ifndef ADM_PRIORITY_DEFINED
#define ADM_PRIORITY_DEFINED   ///< Internal
typedef TIME_T ADM_PRIORITY_T; ///< Time, referenced by the priority queue
#endif

#ifdef ADM_P64
typedef unsigned long DSIZE; ///< size_t on a 64 bit computer
#else
typedef unsigned int DSIZE; ///< size_t on a 32 bit computer
#endif

#ifndef HANDLE_DEFINED
#define HANDLE_DEFINED
typedef unsigned long long DHANDLE; ///< DHANDLE types
#endif

/**
 * <b>DBOOL</b> must be at least as wide as the larger of <b>int</b> (the
 * compiler's natural implicit Boolean type), <b>Ix32</b>,
 * to allow bit-wise operations on any vdi type to be stored i an
 * <b>DBOOL</b> without loss of information.
 */
#ifndef DBOOL_DEFINED
#define DBOOL_DEFINED ///< Internal
typedef char DBOOL;   ///< Boolean type
#endif

/** UTF8-encoded Unicode strings, NULL-terminated */
#ifndef UTF8Str_DEFINED
#define UTF8Str_DEFINED ///< Internal
typedef D8U *UTF8Str;   ///< UTF8 encoded Unicode string, NULL-terminated
#endif
#ifndef UTF8CStr_DEFINED
#define UTF8CStr_DEFINED     ///< Internal
typedef const D8U *UTF8CStr; ///< UTF8 encoded Unicode string constant, NULL-terminated
#endif

/** An integer that represents an array of bit flags */
#ifndef DBITFLAGS_DEFINED
#define DBITFLAGS_DEFINED ///< Internal
typedef D32U DBITFLAGS;   ///< An integer representing an array of bit flags
#endif

/** @} */

/** @defgroup pl_macros Useful Macros
 *  @ingroup pl_macro
 *	@{
 */

/** Macro to remove warnings relating to unused function arguments */
#ifndef UNUSED
#define UNUSED(x) \
  {               \
    (void)x;      \
  }
#endif

/*! Macro for adm_min */
#ifndef adm_min
#define adm_min(a, b) (((a) > (b)) ? (b) : (a))
#endif

/*! Macro for adm_max */
#ifndef adm_max
#define adm_max(a, b) (((a) < (b)) ? (b) : (a))
#endif

/** Maximum value for a signed 32-bit integer */
#ifndef MAX_D32S
#define MAX_D32S 0x7FFFFFFF
#endif

/** Maximum value for an unsigned 32-bit integer */
#ifndef MAX_D32U
#define MAX_D32U 0xFFFFFFFF
#endif

/** Maximum value for an unsigned 64-bit integer */
#ifndef MAX_D64U
#define MAX_D64U 0xFFFFFFFFFFFFFFFF
#endif

#define BYTES_ALIGNED(num) (((num) + ((4) - 1)) & (~((4) - 1)))

/** @} */

#define ABADAPTOR_EXPORT __attribute__((visibility("default")))
#define _EXTERN_ extern

#ifndef INVALIDHANDLE_DEFINED
#define INVALIDHANDLE_DEFINED
#define D_INVALID_HANDLE (DHANDLE)(-1)
#endif

/** True */
#ifndef DTRUE
#define DTRUE 1
#endif

#ifndef TRUE
#define TRUE 1
#endif

/** False */
#ifndef DFALSE
#define DFALSE 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

/** Replacement for NULL when <b>\<stdio.h\></b> is not wanted */
#ifndef NULL
  #ifdef __cplusplus
    #define	NULL	0
  #else
    #define	NULL	((void *)0)
  #endif
#endif

#ifndef DNULL
#ifdef __cplusplus
#define DNULL 0
#else
#define DNULL ((void *)0)
#endif
#endif

/*! @} */

#endif // _ADM_TYPEDEFS_H_
