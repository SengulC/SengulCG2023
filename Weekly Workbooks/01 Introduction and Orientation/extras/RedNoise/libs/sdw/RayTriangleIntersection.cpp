#include "RayTriangleIntersection.h"

RayTriangleIntersection::RayTriangleIntersection() = default;
RayTriangleIntersection::RayTriangleIntersection(const glm::vec3 &point, float distance, float t, const ModelTriangle &triangle, size_t index, bool valid) :
		intersectionPoint(point),
		distanceFromStart(distance),
        t(t),
		intersectedTriangle(triangle),
		triangleIndex(index),
        valid(valid) {}

std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection) {
	os << "Intersection is" << intersection.valid << "at [" << intersection.intersectionPoint[0] << "," << intersection.intersectionPoint[1] << "," <<
	   intersection.intersectionPoint[2] << "] on triangle " << intersection.intersectedTriangle <<
	   " at a distance of " << intersection.distanceFromStart;
	return os;
}
