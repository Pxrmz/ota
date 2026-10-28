/**
 * @file adm_cfg_api.h
 * @author mengwei@abupdate.com
 * @brief 配置文件的解析，设置或获取字段等，包括：VUS、SYSI、eweb、rpc等的所有配置信息
 * @version 0.1
 * @date 2021-09-27
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_CFG_API_H__
#define __ADM_CFG_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_cfg_default.h"


typedef D32S (*ADM_CFG_API_WRITE_FILE_DATA_CALLBACK_F)(const DCHAR* filePath, const DCHAR* fileData);  //!< 写数据到文件中
typedef DBOOL (*ADM_CFG_API_IS_FILE_EXIST_CALLBACK_F)(const DCHAR* pFilePath);   //!< 判断文件是否存在


/**
 * @brief 为iniParser初始化增加需要的基础函数
 * 
 */
typedef struct 
{
    ADM_CFG_API_WRITE_FILE_DATA_CALLBACK_F writeFileDataCb;     //!< 写入数据到文件中
    ADM_CFG_API_IS_FILE_EXIST_CALLBACK_F    isFileExistCb;       //!< 判断文件是否存在
} ADM_CFG_API_INFO_T;


/**
 * @brief 初始化配置文件解析
 * 
 * @param filename [IN]     const DCHAR*    配置文件的绝对路径，支持可读
 * @param pstData  [IN]     const ADM_CFG_API_INFO_T*   为iniParser初始化增加需要的基础函数
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_cfg_iniParserInit(const DCHAR* filename, const ADM_CFG_API_INFO_T* pstData);

/**
 * @brief 反初始化配置文件解析
 * 
 */
extern void adm_cfg_iniParserDeinit(void);

/**
 * @brief 从配置文件中获取对应key的字符串
 * 
 * @param key [in]  待查找的关键字
 * @param notfound [in] 如果未找到键，将返回默认值。
 * @return const DCHAR*   指向静态分配字符串的指针
 * 
 * @note 这个函数在字典中查询一个键。从ini文件中读取的键被给出为"section:key"。
 * 返回的char指针指向在字典中分配的字符串，不要释放或修改它。
 * 
 */
extern const DCHAR* adm_cfg_iniParserGetString(const DCHAR* key, const DCHAR* notfound);

/**
 * @brief 从配置文件中获取对应key的整型值
 * 
 * @param key [in]  待查找的关键字
 * @param notfound [in] 如果未找到键，将返回默认值。
 * @return D32S     查找到的整型值
 * 
 * @note 
 * 这个函数在字典中查询一个键。从一个
 * Ini文件以“section:key”的形式给出。如果找不到钥匙，返回notfound值。
 * 支持的整数值包括常用的C符号
 * 十进制，八进制(从0开始)十六进制(从0x开始)都受支持。
 * 例子:
 * - "42" -> 42
 * - "042" -> 34(八进制->十进制)
 * - "0x42" -> 66 (hexa ->十进制)
 */
extern D32S adm_cfg_iniParserGetInt(const DCHAR* key, D32S notfound);

/**
 * @brief 从配置文件中获取对应key的长整型值
 * 
 * @param key [in]  待查找的关键字
 * @param notfound [in] 如果未找到键，将返回默认值。
 * @return DSIZE    查找到的长整型值
 * 
 * @note 
 * 这个函数在字典中查询一个键。从一个
 * Ini文件以“section:key”的形式给出。如果找不到钥匙，返回notfound值。
 * 支持的整数值包括常用的C符号
 * 十进制，八进制(从0开始)十六进制(从0x开始)都受支持。
 * 例子:
 * - "42" -> 42
 * - "042" -> 34(八进制->十进制)
 * - "0x42" -> 66 (hexa ->十进制)
 * 
 */
extern long int adm_cfg_iniParserGetLongInt(const DCHAR* key, long int notfound);

/**
 * @brief 从配置文件中获取对应key的浮点数值
 * 
 * @param key [in]  待查找的关键字
 * @param notfound [in] 如果未找到键，将返回的默认值。
 * @return double   查找到的浮点数值
 * 
 * @note
 * 这个函数在字典中查询一个键。从ini文件中读取的键被给出为“section:key”。如果找不到键，则返回notfound值。
 */
extern double adm_cfg_iniParserGetDouble(const DCHAR* key, double notfound);

/**
 * @brief 从配置文件中获取对应key的布尔类型值
 * 
 * @param key [in]  待查找的关键字
 * @param notfound [in] 如果未找到键，将返回的默认值。
 * @return D32S     查找到的布尔类型值
 * 
 * @note
 * 这个函数在字典中查询一个键。从ini文件中读取的键被给出为“section:key”。如果找不到键，则返回notfound值。
 * 如果匹配下列任一项，则会发现一个true布尔值:
 * -以“y”开头的字符串
 * -以“Y”开头的字符串
 * -以“t”开头的字符串
 * -以“T”开头的字符串
 * -以“1”开头的字符串
 * 
 * 如果匹配以下任一项，则会发现一个false boolean:
 * —以“n”开头的字符串
 * —以“N”开头的字符串
 * -以“f”开头的字符串
 * -以“F”开头的字符串
 * -以'0'开头的字符串
 * 如果没有标识布尔值，则返回的notfound值不一定是0或1。
 */
extern DBOOL adm_cfg_iniParserGetBoolean(const DCHAR* key, DBOOL notfound);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_CFG_API_H__