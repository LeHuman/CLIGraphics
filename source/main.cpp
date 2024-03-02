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
    CLIGx::CLIGraphics<50, 20> gx;

    CLIGx::vec4 tp{0.0f, 0.0f, 0.0f, 1.0f};

    while (true) {
        if (mouse.active) {
            // gx.setCenterPosition(CLIGx::vec3{mouse.x / 500.0f, mouse.y / 500.0f, 0.0f});
            gx.setCameraPosition(CLIGx::vec3{mouse.x / 250.0f, mouse.y / 250.0f, mouse.wheelVertical / 10.0f});
        } else {
            CLIGx::vec3 relativePosition = gx.eye - gx.center;
            static float angle = 0.01f;
            // angle += 0.0001f;
            CLIGx::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(gx.up));
            relativePosition = glm::vec3(rotationMatrix * glm::vec4(relativePosition, 1.0f));
            CLIGx::vec3 newPosition = gx.center + relativePosition;
            gx.setCameraPosition(newPosition);
        }

        gx.drawLines(gx.getHLines(stlglm::openSTLFile("models/ring.stl")));
        gx.clearBuffer();
    }

    return 0;
}
