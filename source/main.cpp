#include <iostream>
#include <string>
#include <unordered_map>

#include <cxxopts.hpp>
#include <fmt/format.h>
#include <glm/glm.hpp>

#include "cligx.hpp"
#include "mouse.hpp"
#include "stlglm.hpp"

auto main(int argc, char **argv) -> int {
    mouse.setClamp(500, 500);
    mouse.startPolling();
    CLIGx::CLIGraphics<100, 40> gx(1, CLIGx::CHARSET_braille);

    while (true) {
        gx.setCenterPosition(CLIGx::vec3{mouse.x / 200.0f, mouse.y / 200.0f, mouse.wheelVertical / 10.0f});

        CLIGx::vec3 relativePosition = gx.eye - gx.center;
        static float angle = 0.1f;
        CLIGx::mat4 rotationMatrix = glm::rotate(CLIGx::mat4(1.0f), angle, gx.up);
        relativePosition = CLIGx::vec3(rotationMatrix * CLIGx::vec4(relativePosition, 1.0f));
        CLIGx::vec3 newPosition = gx.center + relativePosition;
        gx.setCameraPosition(newPosition);

        gx.drawLines(gx.getHLines(stlglm::openSTLFile("models/Stanford_Bunny_Min.stl")));
        gx.clearBuffer();
    }

    return 0;
}
