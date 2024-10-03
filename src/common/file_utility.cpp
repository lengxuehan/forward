//
// Created by wq on 22-4-28.
//

#include "common/file_utility.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>

namespace forward{
namespace common{
    bool FileUtility::is_dir(const std::string& str_dir_path){
        struct stat sb{};

        if ((stat(str_dir_path.c_str(), &sb) == 0) && S_ISDIR(sb.st_mode)){
            return true;
        }
        else{
            return false;
        }
    }

    int32_t FileUtility::delete_directory(const std::string &str_dir_path) {
        if (str_dir_path.empty()){
            return 0;
        }

        DIR *the_folder = opendir(str_dir_path.c_str());
        struct dirent *next_file;
        int32_t ret_val;

        if (the_folder == nullptr){
            return errno;
        }
        next_file = readdir(the_folder);
        while (next_file != nullptr ){
            // build the path for each file in the folder
            std::string str_file_name = next_file->d_name;
            std::string str_file_path;
            (void)str_file_path.append(str_dir_path).append("/").append(str_file_name);
            //we don't want to process the pointer to "this" or "parent" directory
            if ((str_file_name == "..") || (str_file_name == ".")){
                next_file = readdir(the_folder);
                continue;
            }

            //is_dir will check if the "filepath" is a directory
            if (is_dir(str_file_path)){
                ret_val = delete_directory(str_file_path);
                if (ret_val != 0){   // Error.
                    (void)closedir(the_folder);
                    return ret_val;
                }
            }else{
                ret_val = remove(str_file_path.c_str());    // delete file in this folder
                //ENOENT occurs when i folder is empty, or is a dangling link, in
                //which case we will say it was a success because the file is gone
                if ((ret_val != 0) && (ret_val != ENOENT)){
                    (void)closedir(the_folder);
                    return ret_val;
                }
            }
            next_file = readdir(the_folder);
        }

        (void)closedir(the_folder);

        return rmdir(str_dir_path.c_str());
    }

    int32_t FileUtility::mk_dir(const std::string& str_dir_path) {
        if (str_dir_path.empty()) { //空目录，直接返回0
            return 0;
        }
        if (str_dir_path.size() == 1U) { // 只有一个字母缺少/ 或者 只是一个'/' 直接返回0
            return 0;
        }
        std::string::size_type search_pos = 0U;
        do {
            search_pos = str_dir_path.find('/', search_pos + 1U);
            if (search_pos == std::string::npos) {
                break;
            }
            std::string sub_dir = str_dir_path.substr(0U, search_pos);

            if ((mkdir(sub_dir.c_str(), S_IRWXU |
                                       S_IRGRP | S_IXGRP |
                                       S_IROTH | S_IXOTH) == -1)
                && (errno != EEXIST)) {
                return -1;
            }
        } while (true);
        return 0;
    }

    std::string FileUtility::get_process_path() {
        // using the id, find the name of this process
        char current_process_name[PATH_MAX+1];
        char path[PATH_MAX];
        (void)sprintf(path, "/proc/%d/cmdline", getpid());    // /proc/%d/exefile in qnx
        (void)memset(current_process_name, 0, sizeof(current_process_name));
        int32_t pinfo = open(path, O_RDONLY);
        if (pinfo >= 0){
            // Read process name from file descriptor pinfo
            // http://linux.die.net/man/2/read
            const ssize_t r = read(pinfo, current_process_name, PATH_MAX);
            if ((r > 0) && (r <= PATH_MAX)){
                current_process_name[static_cast<uint32_t>(r)] = '\0';
                //MEGA_LOG_INFO << "Current process:" << current_process_name << ". Process id is" << getpid();
            }else{
                //MEGA_LOG_ERROR << "Could not read process name." << path;
            }
            (void)close(pinfo);
        }else{
            //MEGA_LOG_ERROR << "Failed to open" << path;
            current_process_name[0] = '\0';
        }
        std::string str(current_process_name);
        if (!str.empty()){
            const std::string::size_type n_pos = str.rfind('/');
            if (n_pos == std::string::npos) {
                //MEGA_LOG_ERROR << "Not found / in" << str;
            } else {
                str = str.substr(0U, n_pos);
            }
        }
        return str;
    }
}
}