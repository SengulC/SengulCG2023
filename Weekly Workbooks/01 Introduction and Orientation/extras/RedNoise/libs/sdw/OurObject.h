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

std::vector<ModelTriangle> readObj(const std::string& file, std::map<std::string, Colour> mtls, float scale);
std::map<std::string, Colour> readMaterial(const std::string& file);
CanvasPoint getCanvasIntersectionPoint(glm::vec3 vertexPosition, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focalLength, float scale);
