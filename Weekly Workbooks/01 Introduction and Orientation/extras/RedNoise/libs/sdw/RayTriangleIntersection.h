#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "ModelTriangle.h"

struct RayTriangleIntersection {
	glm::vec3 intersectionPoint;
    float distanceFromStart;
    float t;
	ModelTriangle intersectedTriangle;
	size_t triangleIndex;

	RayTriangleIntersection();
	RayTriangleIntersection(const glm::vec3 &point, float distance, float t, const ModelTriangle &triangle, size_t index);
	friend std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection);
};
