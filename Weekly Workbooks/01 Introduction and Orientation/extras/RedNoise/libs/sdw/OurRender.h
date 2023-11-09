#pragma once

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "Utils.h"
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "CanvasPoint.h"
#include "Colour.h"
#include "ModelTriangle.h"
#include <iostream>
#include <string>
#include "OurLine.h"

void bAndWdraw(DrawingWindow &window);

void rainbowDraw(DrawingWindow &window);

std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3> rasterize(DrawingWindow &window, std::vector<ModelTriangle> modelTriangles, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focalLength, float scale, std::vector<std::vector<float>> depthMatrix, bool orbit);

glm::mat3 LookAt(glm::mat3 cameraOrientation, glm::vec3 lookAtMe, glm::vec3 cameraPosition, float focalLength, float scale);