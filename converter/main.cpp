#include "imageu/image_utils.hpp"
#include "files/file_utils.hpp"

// I already checked it directly on LuckFox single-board. It works correctly.



int main(int argc, char* argv[])
{
    std::string current = "frame.raw";
    std::string converted = "frame.png";
    const int width  = 640;
    const int height = 480;

    if (argc > 1)
    {
        current = argv[1];
        std::cout << "Received info about: " << current << "\n";
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