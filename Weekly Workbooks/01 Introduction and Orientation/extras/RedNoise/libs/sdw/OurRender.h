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

std::vector<CanvasTriangle> rasterize(DrawingWindow &window, std::vector<ModelTriangle> modelTriangles, glm::vec3 cameraPosition, float focalLength, float scale, std::vector<std::vector<int>> depthMatrix);