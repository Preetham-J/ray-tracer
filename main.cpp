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


class Sphere
{
    public:
        Sphere(const Vec3f &center, const float &radius) :
            center_(center), radius_(radius)
        {}

        // Determine whether a ray intersects the sphere
        // Based on the geometric solution to the ray-sphere intersection algorithm 
        bool RayIntersect(const Vec3f &origin, const Vec3f &direction, float &t0) const
        {
            float t1, tca, d, thc;
            Vec3f L = center_ - origin;
            tca = L * direction;
            d = (L * L) - (tca * tca);
            if (d > radius_ * radius_)
            {
                return false;
            }
            thc = sqrtf(radius_ * radius_ - d);
            t0 = tca - thc;
            t1 = tca + thc;
            // If t0 is negative, use t1 instead
            if (t0 > t1)
            {
                t0 = t1;
            }
            // If both are negative, no intersection
            if (t0 < 0)
            {
                return false;
            }
            // Otherwise, there is an intersection
            return true;
        }

    private:
        Vec3f center_;
        float radius_;
};

// Cast a ray from the origin to the sphere in the given direction
Vec3f CastRay(const Vec3f& origin, const Vec3f& direction, const Sphere& sphere)
{
    float sphere_distance = std::numeric_limits<float>::max();
    // If the ray doesn't intersect, return background colour
    if (!sphere.RayIntersect(origin, direction, sphere_distance))
    {
        return Vec3f(0.2, 0.7, 0.8);
    }
    // Else return set colour
    return Vec3f(0.4, 0.4, 0.3);
}

// Render image
void render(const Sphere &sphere)
{
    const int width = 1024;
    const int height = 768;
    const float fov = M_PI/3.0;
    std::vector<Vec3f> frame_buffer(width * height);

    // Fill frame buffer with ray tracing results
    for (std::size_t j = 0; j < height; j++)
    {
        for (std::size_t i = 0; i < width; i++)
        {
            float x = (2*(i + 0.5)/(float)width - 1) * tan(fov/2.0)*width/(float)height;
            float y = -(2*(j + 0.5)/(float)height- 1) * tan(fov/2.0);
            Vec3f direction = Vec3f(x, y, -1).normalise();
            frame_buffer[i + j*width] = CastRay(Vec3f(0, 0, 0), direction, sphere);
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
    Sphere sphere(Vec3f(-3, 0, -16), 2);
    render(sphere);
    return 0;
}
