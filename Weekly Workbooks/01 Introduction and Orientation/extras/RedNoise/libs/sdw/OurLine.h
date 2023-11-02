#pragma once

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "Utils.h"
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include"CanvasPoint.h"
#include"Colour.h"
#include"ModelTriangle.h"
#include <iostream>
#include <string>
#include <map>

bool sortByY(const CanvasPoint& a, const CanvasPoint& b);
std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues);
std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues);
std::vector<std::vector<int>> drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color, std::vector<std::vector<int>> depthMatrix);
void drawPoint(DrawingWindow &window, CanvasPoint point, Colour color);
std::vector<int> unpack(const Colour& color);
uint32_t pack(std::vector<int> colorgb);


