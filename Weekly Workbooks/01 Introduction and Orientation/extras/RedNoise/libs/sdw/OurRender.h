#pragma once

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "Utils.h"
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "CanvasPoint.h"
#include "RayTriangleIntersection.h"
#include "Colour.h"
#include "ModelTriangle.h"
#include <iostream>
#include <string>
#include "OurLine.h"
#include "OurTriangle.h"
#include "glm/ext.hpp"
#include <algorithm>

void bAndWdraw(DrawingWindow &window);

void rainbowDraw(DrawingWindow &window);

std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3, std::vector<std::vector<float>>> drawRasterizedScene(DrawingWindow &window, std::vector<ModelTriangle> modelTriangles, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focalLength, float scale, std::vector<std::vector<float>> depthMatrix, bool orbit);

CanvasPoint getCanvasIntersectionPoint(CanvasPoint vertexPosition, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focalLength, float scale);

glm::mat3 LookAt(glm::mat3 cameraOrientation, glm::vec3 lookAtMe, glm::vec3 cameraPosition);

RayTriangleIntersection getClosestValidIntersection(glm::vec3 startPosition, glm::vec3 endPosition, glm::vec3 rayDirection, const std::vector<ModelTriangle>& triangles, bool shadow, size_t currIndex);

void drawRaytracedScene(DrawingWindow &window, const std::vector<ModelTriangle>& triangles, float scale, float focalLength, glm::vec3 cameraPosition, glm::mat3 cameraOrientation);

bool validTUV(glm::vec3 tuv, float dist, bool shadow);

glm::vec3 convertToDirectionVector(CanvasPoint point);