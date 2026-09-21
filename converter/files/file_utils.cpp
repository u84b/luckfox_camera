#include "file_utils.hpp"

std::string get_filename_without_extension(const std::string& path) {
    // 1. Find the last path separator to isolate the filename
    size_t last_sep = path.find_last_of("/\\");
    std::string filename = "";
    if (last_sep == std::string::npos)
    {
        filename = path;
    }
    else {
        filename = path.substr(last_sep + 1);
    }

    // 2. Find the last dot in the isolated filename
    size_t last_dot = filename.find_last_of('.');
    if (last_dot != std::string::npos) {
        return filename.substr(0, last_dot);
    }
    
    return filename; // Return as-is if there is no extension
}