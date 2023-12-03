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
#include <unordered_map>

std::vector<std::pair<glm::vec3, glm::vec3>> calculateVertexNormals(const std::vector<ModelTriangle>& modelTriangles);
std::vector<ModelTriangle> readObj(const std::string& file, std::map<std::string, Colour> mtls, float scale, bool sphere);
std::map<std::string, Colour> readMaterial(const std::string& file);
