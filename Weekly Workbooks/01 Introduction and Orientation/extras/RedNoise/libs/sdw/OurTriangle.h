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

std::vector<std::vector<int>> drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color, std::vector<std::vector<int>> depthMatrix);
std::vector<std::vector<int>> drawFilled(DrawingWindow &window, CanvasTriangle triangle, Colour color, std::vector<std::vector<int>> depthMatrix);
CanvasTriangle randomTriangle();
Colour randomColor();