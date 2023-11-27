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

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues);
std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues);
std::vector<std::vector<float>> drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color, std::vector<std::vector<float>> depthMatrix, bool depth);
void drawPoint(DrawingWindow &window, CanvasPoint point, Colour color);
std::vector<int> unpack(const Colour& color);
uint32_t pack(std::vector<int> colorgb);
void printMat3(const glm::mat3& matrix);
std::vector<CanvasPoint> interpolateCanvasPoint(CanvasPoint from, CanvasPoint to, int numberOfValues);
float roundToThreeSF(float num);
std::string colorName(const uint32_t color);
void printDepthMatrix(const std::vector<std::vector<float>>& depthMatrix);
std::vector<std::vector<uint32_t>> linearListTo2DMatrix(const std::vector<uint32_t>& linearList, size_t height, size_t width);
bool sortByY(const CanvasPoint& a, const CanvasPoint& b);
