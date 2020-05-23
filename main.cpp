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


// Determine rendered colours
struct Material
{
    Material(const Vec3f& colour) :
        diffuse_colour_(colour)
    {}
    Material() : diffuse_colour_() {}
    Vec3f diffuse_colour_;
};

// Object used to populate the scene
class Sphere
{
    public:
        Sphere(const Vec3f &center, const float &radius, const Material& material) :
            center_(center), radius_(radius), material_(material)
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

        Vec3f GetCenter() const { return center_; }
        float GetRadius() const { return radius_; }
        Material GetMaterial() const { return material_; }
    private:
        Vec3f center_;
        float radius_;
        Material material_;
};

// Determine overlapping sphere priority (intersections)
bool SceneIntersect(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, Vec3f& hit, Vec3f& N, Material& material)
{
    float spheres_distance = std::numeric_limits<float>::max();
    for (std::size_t i = 0; i < spheres.size(); i++)
    {
        float dist_i;
        if ((spheres[i].RayIntersect(origin, direction, dist_i)) && (dist_i < spheres_distance))
        {
            spheres_distance = dist_i;
            hit = origin + direction*dist_i;
            N = (hit - spheres[i].GetCenter()).normalise();
            material = spheres[i].GetMaterial();
        }
    }
    return spheres_distance < 1000;
}

// Cast a ray from the origin to the spheres in the given direction
Vec3f CastRay(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres)
{
    Vec3f point, N;
    Material material;
    // If the ray doesn't intersect, return background colour
    if (!SceneIntersect(origin, direction, spheres, point, N, material))
    {
        return Vec3f(0.2, 0.7, 0.8);
    }
    // Else return material of sphere
    return material.diffuse_colour_;
}

// Render image
void Render(const std::vector<Sphere>& spheres)
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
            // Use normalized device coordinates remapped from [0:1] to [-1:1]
            float x_ndc = 2 * ((i + 0.5) / (float)width) - 1;
            float y_ndc = 1 - 2 * ((j + 0.5) / (float)height);
            // Factor in FOV and aspect ratio
            float x_camera = x_ndc * tan(fov/2.0) * width/(float)height;
            float y_camera = y_ndc * tan(fov/2.0);
            // Create direction vector to pixel and store ray results in buffer
            Vec3f direction = Vec3f(x_camera, y_camera, -1).normalise();
            frame_buffer[i + j*width] = CastRay(Vec3f(0, 0, 0), direction, spheres);
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
    // Create materials
    Material ivory(Vec3f(0.4, 0.4, 0.3));
    Material red_rubber(Vec3f(0.3, 0.1, 0.1));

    // Create spheres
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, ivory));

    // Render scene
    Render(spheres);
    return 0;
}
