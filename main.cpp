/*
 * main.cpp
 *
 */

#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.hpp"

void render()
{
    const int width = 1024;
    const int height = 768;
    std::vector<Vec3f> frame_buffer(width * height);

    // Initialize frame buffer with random RGB values
    for (std::size_t j = 0; j < height; j++)
    {
        for (std::size_t i = 0; i < width; i++)
        {
            frame_buffer[i + j*width] = Vec3f(j / float(height), i / float(width), 0);
        }
    }

    // Create output file and write to it
    std::ofstream output_file;
    output_file.open("./output.ppm");
    output_file << "P6\n" << width << " " << height << "\n255\n";
    for (std::size_t i = 0; i < height*width; ++i)
    {
        for (std::size_t j = 0; j < 3; j++)
        {
            output_file << (char)(255 * std::max(0.f, std::min(1.f, frame_buffer[i][j])));
        }
    }
    output_file.close();
}

int main()
{
    render();
    return 0;
}
