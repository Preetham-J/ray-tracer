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

// Light source
struct Light
{
    Light(const Vec3f& position, const float& intensity) :
        position(position), intensity(intensity)
    {}
    Vec3f position;
    float intensity;
};

// Rendered colours
struct Material
{
    Material(const Vec3f& colour) :
        diffuse_colour(colour)
    {}
    Material() : diffuse_colour() {}
    Vec3f diffuse_colour;
};

// Object used to populate the scene
class Sphere
{
    public:
        Sphere(const Vec3f& center, const float& radius, const Material& material) :
            center_(center), radius_(radius), material_(material)
        {}

        // Determine whether a ray intersects the sphere
        // Based on the geometric solution to the ray-sphere intersection algorithm 
        bool RayIntersect(const Vec3f& origin, const Vec3f& direction, float& t0) const
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
            if (t0 < 0)
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
bool SceneIntersect(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, Vec3f& point_hit, Vec3f& normal, Material& material)
{
    float spheres_distance = std::numeric_limits<float>::max();
    for (std::size_t i = 0; i < spheres.size(); i++)
    {
        float dist_i;
        if ((spheres[i].RayIntersect(origin, direction, dist_i)) && (dist_i < spheres_distance))
        {
            spheres_distance = dist_i;
            point_hit = origin + direction*dist_i;
            normal = (point_hit - spheres[i].GetCenter()).normalise();
            material = spheres[i].GetMaterial();
        }
    }
    return spheres_distance < 1000;
}

// Cast a ray from the origin to the spheres in the given direction
Vec3f CastRay(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, const std::vector<Light>& lights)
{
    Vec3f point, normal;
    Material material;
    // If the ray doesn't intersect, return background colour
    if (!SceneIntersect(origin, direction, spheres, point, normal, material))
    {
        return Vec3f(0.2, 0.7, 0.8);
    }
    // Else determine light intensity for return colour
    float diffuse_light_intensity = 0;
    for (std::size_t i = 0; i < lights.size(); i++)
    {
        // Use light direction and normal at the point of intersection to determine intensity (smaller angle = better illumination)
        Vec3f light_direction = (lights[i].position - point).normalise();
        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_direction*normal);
    }
    return material.diffuse_colour * diffuse_light_intensity;
}

// Render image
void Render(const std::vector<Sphere>& spheres, const std::vector<Light>& lights)
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
            frame_buffer[i + j*width] = CastRay(Vec3f(0, 0, 0), direction, spheres, lights);
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

    // Create light source
    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));

    // Render scene
    Render(spheres, lights);
    return 0;
}
