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

// Maximum recursive calls per reflected/refracted ray
#define MAXIMUM_DEPTH 4


// Light source for illumination
struct Light
{
    Light(const Vec3f& position, const float& intensity) :
        position(position), intensity(intensity)
    {}
    Vec3f position;
    float intensity;
};

// Rendered colour for an object in the scene
struct Material
{
    Material(const float& refractive, const Vec4f& albedo, const Vec3f& colour, const float& specular) :
        refractive_index(refractive), albedo(albedo), diffuse_colour(colour), specular_exponent(specular)
    {}
    Material() : refractive_index(1), albedo(1, 0, 0, 0), diffuse_colour(), specular_exponent() {}
    float refractive_index;
    Vec4f albedo;
    Vec3f diffuse_colour;
    float specular_exponent;
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

// Use Snell's Law to determine the refraction vector
Vec3f Refract(const Vec3f& incident, const Vec3f& normal, const float& refractive_index)
{
    float cos_i = -std::max(-1.f, std::min(1.f, incident*normal));
    float eta_i = 1, eta_t = refractive_index;
    Vec3f n = normal;
    // If the ray is inside the object, swap indices and invert the normal
    if (cos_i < 0)
    {
        cos_i = -cos_i;
        std::swap(eta_i, eta_t);
        n = -normal;
    }
    float eta = eta_i/eta_t;
    float k = 1 - eta*eta*(1 - cos_i*cos_i);
    return k < 0 ? Vec3f(0, 0, 0) : incident*eta + n*(eta*cos_i - sqrtf(k));
}

// Determine reflection vector using incident angle and surface normal
Vec3f Reflect(const Vec3f& incident, const Vec3f& normal)
{
    return incident - normal*2.f*(incident*normal);
}

// Determine overlapping sphere priority (intersections)
bool SceneIntersect(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, Vec3f& point_hit,
                    Vec3f& normal, Material& material)
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
Vec3f CastRay(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, 
              const std::vector<Light>& lights, std::size_t depth = 0)
{
    Vec3f point, normal;
    Material material;

    // If the ray doesn't intersect, return background colour
    // Additionally, boundary check maximum depth
    if ((depth > MAXIMUM_DEPTH) || (!SceneIntersect(origin, direction, spheres, point, normal, material)))
    {
        return Vec3f(0.2, 0.7, 0.8);
    }

    // Recursively call CastRay for each reflected ray
    Vec3f reflect_direction = Reflect(direction, normal);
    Vec3f reflect_origin = reflect_direction*normal < 0 ? point - normal*1e-3 : point + normal*1e-3;
    Vec3f reflect_colour = CastRay(reflect_origin, reflect_direction, spheres, lights, depth + 1);

    // Recursively call CastRay for each refracted ray
    Vec3f refract_direction = Refract(direction, normal, material.refractive_index).normalise();
    Vec3f refract_origin = refract_direction*normal < 0 ? point - normal*1e-3 : point + normal*1e-3;
    Vec3f refract_colour = CastRay(refract_origin, refract_direction, spheres, lights, depth + 1);

    // Else determine diffuse and specular light intensities for return colour
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (std::size_t i = 0; i < lights.size(); i++)
    {
        // Use light direction and normal at the point of intersection to determine intensity (smaller angle = better illumination)
        Vec3f light_direction = (lights[i].position - point).normalise();

        // Check if there is an object between the point and the light source; if there is, skip the light source
        float light_distance = (lights[i].position - point).norm();
        Vec3f shadow_origin = light_direction*normal < 0 ? point - normal*1e-3 : point + normal*1e-3;
        Vec3f shadow_point, shadow_normal;
        Material temp_material;
        if ((SceneIntersect(shadow_origin, light_direction, spheres, shadow_point, shadow_normal, temp_material)) &&
                            ((shadow_point - shadow_origin).norm() < light_distance))
        {
            continue;
        }

        // Diffuse illumination
        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_direction*normal);
        // Use the Phong reflection model to determine specular lighting
        specular_light_intensity += powf(std::max(0.f, -Reflect(-light_direction, normal)*direction),
                                         material.specular_exponent) * lights[i].intensity;
    }
    return material.diffuse_colour*diffuse_light_intensity*material.albedo[0] + 
           Vec3f(1.0, 1.0, 1.0)*specular_light_intensity*material.albedo[1] +
           reflect_colour*material.albedo[2] +
           refract_colour*material.albedo[3];
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
        Vec3f& c = frame_buffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1)
        {
            c = c * (1./max);
        }
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
    Material ivory(1.0, Vec4f(0.6, 0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3), 50.0);
    Material glass(1.5, Vec4f(0.0, 0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8), 125.0);
    Material red_rubber(1.0, Vec4f(0.9, 0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1), 10.0);
    Material mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.0);

    // Create spheres
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, glass));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, mirror));

    // Create light sources
    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
    lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f(30, 20, 30), 1.7));

    // Render scene
    Render(spheres, lights);
    return 0;
}
