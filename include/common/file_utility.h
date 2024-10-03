/** @addtogroup common
 * \ingroup forward
 *  @{
 */
/**
* @file file_utility.h
* @brief utility of file operation for RVCS
* @details
* @author		wuting.xu
* @date		    2024/10/03
* @par Copyright(c): 	2024. All rights reserved.
*/

#pragma once

#include <string>

namespace forward{
namespace common{
class FileUtility {
public:
    FileUtility() = default;
    virtual ~FileUtility() = default;
    FileUtility(FileUtility const&) = delete;
    FileUtility& operator =(FileUtility const&) = delete;
    FileUtility(FileUtility&&) = delete;
    FileUtility& operator=(FileUtility&&) = delete;

    /**
     * checks if a specific directory exists
     * @param[in] str_dir_path the path to check
     * @return if the path exists
     */
    static bool is_dir(const std::string& str_dir_path);

    /**
     * deletes all the files in a folder recursively and this folder
     * @param[in] str_dir_path the directory to delete the contents of (can be full or
     * relative path)
     * @return 0 on success
     *         errno on failure, values can be from unlink or rmdir
     */
    static int32_t delete_directory(const std::string& str_dir_path);

    /**
     * \brief 支持递归创建目录，如果每一集目录不存在，则创建，直到找不到/为止，退出循环。
     * \param str_dir_path : [in] 路径，可以是相对路径，可以带文件名。/a/b/ ：b被作为目录创建。/a/b：b被认为是文件
     *                                      a：被认为是文件
     * \return 0 ：成功；-1为失败，查询errno，查找失败原因。
     */
    static int32_t mk_dir(const std::string& str_dir_path);

    /**
     * \brief 获得当前运行进程所在的目录。
     * \return 当前运行进程所在的目录。
     * \remark Not portable. Applicable to linux, not applicable to qnx.
     */
    static std::string get_process_path();
};
}
}
/** @}*/    // end of group forward