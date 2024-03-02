#include "stlglm.hpp"

#include <ranges>
#include <unordered_set>

#include <glm/glm.hpp>
#include <openstl/core/stl.h>

CLIGx::vec3 vertex(openstl::Vec3 v) {
    return CLIGx::vec3{v.x, v.y, v.z};
}

std::vector<CLIGx::Line> stlglm::openSTLFile(std::string filename) {
    std::unordered_set<CLIGx::Line, CLIGx::LineHash> lineSet;
    std::vector<CLIGx::Line> lines;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
        return lines;
    std::vector<openstl::Triangle> rawTriangles = openstl::deserializeStl(file);
    file.close();

    auto rawLines = std::views::transform(rawTriangles, [](openstl::Triangle &t) {
                        auto v0 = vertex(t.v0);
                        auto v1 = vertex(t.v1);
                        auto v2 = vertex(t.v2);
                        return std::array<CLIGx::Line, 3>{CLIGx::Line{v0, v1}, CLIGx::Line{v1, v2}, CLIGx::Line{v2, v0}};
                    })
                    | std::views::join;

    lineSet.insert_range(rawLines);
    lines.assign_range(lineSet);

    return lines;
}
