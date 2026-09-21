#include "image_utils.hpp"

// it works correctly

bool nv12_to_png(const std::string& rawPath,
               const std::string& pngPath,
               int width,
               int height)
{
    const size_t nv12Size = width * height * 3 / 2;

    // load RAW in memory
    std::ifstream file(rawPath, std::ios::binary);
    if (!file)
        return false;

    std::vector<unsigned char> buffer(nv12Size);
    file.read(reinterpret_cast<char*>(buffer.data()), nv12Size);

    if (file.gcount() != static_cast<std::streamsize>(nv12Size))
        return false;

    cv::Mat nv12(height * 3 / 2, width, CV_8UC1, buffer.data());

    // NV12 -> BGR
    cv::Mat bgr;
    cv::cvtColor(nv12, bgr, cv::COLOR_YUV2BGR_NV12);

    // BGR -> PNG
    return cv::imwrite(pngPath, bgr);
}