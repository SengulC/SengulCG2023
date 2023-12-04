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
#include "RayTriangleIntersection.h"
#include <iostream>
#include <string>
#include "OurLine.h"
#include "OurRender.h"
#include <glm/glm.hpp>
#include "glm/ext.hpp"

void printVec3 (std::string string, glm::vec3 vec);
void printVertexNormals (std::vector<ModelTriangle> triangles, std::vector<std::pair<glm::vec3, glm::vec3>> vertexNormals);
glm::vec3 calculateVertexNormal(const std::vector<ModelTriangle>& modelTriangles, glm::vec3 vertex);
std::vector<ModelTriangle> readObj(const std::string& file, std::map<std::string, Colour> mtls, float scale, bool sphere);
std::map<std::string, Colour> readMaterial(const std::string& file);
