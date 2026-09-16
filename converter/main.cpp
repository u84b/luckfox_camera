#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <vector>

// I already checked it directly on LuckFox single-board. It works correctly.

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

bool nv12_to_png(const std::string& rawPath,
               const std::string& pngPath,
               int width,
               int height)
{
    const size_t nv12Size = width * height * 3 / 2;

    // Загружаем RAW целиком в память.
    std::ifstream file(rawPath, std::ios::binary);
    if (!file)
        return false;

    std::vector<unsigned char> buffer(nv12Size);
    file.read(reinterpret_cast<char*>(buffer.data()), nv12Size);

    if (file.gcount() != static_cast<std::streamsize>(nv12Size))
        return false;

    // NV12 представляется как матрица высотой 1.5 * H:
    //
    // Y Y Y Y
    // Y Y Y Y
    // Y Y Y Y
    // Y Y Y Y
    // U V U V
    // U V U V
    //
    cv::Mat nv12(height * 3 / 2, width, CV_8UC1, buffer.data());

    // NV12 -> BGR
    cv::Mat bgr;
    cv::cvtColor(nv12, bgr, cv::COLOR_YUV2BGR_NV12);

    // BGR -> PNG
    return cv::imwrite(pngPath, bgr);
}

int main(int argc, char* argv[])
{
    std::string current = "frame.raw";
    std::string converted = "frame.png";
    const int width  = 640;
    const int height = 480;

    if (argc > 1)
    {
        current = argv[1];
        converted = get_filename_without_extension(current).append(".png");
    }


    if (!nv12_to_png(current, converted, width, height))
    {
        std::cerr << "Conversion failed\n";
        return 1;
    }

    std::cout << "Done\n";
    return 0;
}