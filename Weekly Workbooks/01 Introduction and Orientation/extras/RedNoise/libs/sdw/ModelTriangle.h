#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include <vector>
#include "Colour.h"
#include "TexturePoint.h"

struct ModelTriangle {
	std::array<glm::vec3, 3> vertices{};
    std::vector<std::pair<glm::vec3, glm::vec3>> vertexNormals;
    // list of pairs ~ vertex normals 'dictionary'
    // pair: vertex (x,y,z) has a normal (xn, yn, zn) based off of all the triangles vertex appears in
	std::array<TexturePoint, 3> texturePoints{};
	Colour colour{};
	glm::vec3 normal{};

	ModelTriangle();
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, Colour trigColour);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
