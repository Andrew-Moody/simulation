#include "api.h"

#include <iostream>

#include "mesh.h"
#include "graphics.h"
#include "surfacemeshdata.h"

void delaunay_sample(float radius, int density)
{
    std::cout << "Hello, Library" << std::endl;

    using namespace moodysim;

    std::vector<Point3D> input_points{ generate_sample_points(radius, density) };

    DelaunayGenerator delaunay_gen{ input_points };

    SurfaceMeshData mesh_data = delaunay_gen.generate_delaunay_mesh();

    GLFWContext glfw_context{};

    Application application{};

    application.setup();

    application.add_mesh(std::move(mesh_data));

    application.run();
}