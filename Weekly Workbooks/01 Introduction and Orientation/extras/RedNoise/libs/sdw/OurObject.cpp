#include "OurObject.h"
#include <glm/glm.hpp>
#include "glm/ext.hpp"

#define WIDTH 320
#define HEIGHT 240

std::vector<ModelTriangle> readObj(const std::string& file, std::map<std::string, Colour> mtls, float scale) {
    // remember that vertices in OBJ files are indexed from 1 (whereas vectors are indexed from 0).
    std::vector<ModelTriangle> modelTriangles;
    ModelTriangle tempTriangle;
    std::vector<glm::vec3> vertices;
    Colour trigColour;

    std::string myObj; // init string
    std::ifstream theObjFile(file); // read file

    // read the file line by line
    // make a list of vec3s for the vertices
    while (getline (theObjFile, myObj)) {
        if (myObj[0] == 'u') {
            std::vector<std::string> colorInfo = split(myObj, ' ');
            trigColour = mtls[colorInfo[1]];
        }
        else if (myObj[0] == 'v') {
            std::vector<std::string> xyz = split(myObj, ' ');
            glm::vec3 currVector{std::stof(xyz[1])*scale, std::stof(xyz[2])*scale, std::stof(xyz[3])*scale}; // xyz[0] = 'v'
            vertices.push_back(currVector);
        }
        else if (myObj[0] == 'f') {
            // e.g. myObj = "f 2/ 3/ 4/"
            std::vector<std::string> facet = split(myObj, ' '); // ["f", "2/", "3/", "4/"]
            // facet[x][0] gets vertex num. convert that to int. look up that vec3 in vertices list (-1 bc of indexing).
            glm::vec3 v0 = vertices[std::stoi(facet[1])-1];
            glm::vec3 v1 = vertices[std::stoi(facet[2])-1];
            glm::vec3 v2 = vertices[std::stoi(facet[3])-1];
            glm::vec3 edge1 (v1-v0); glm::vec3 edge2 (v2-v0);
            // normal vector to triangle
            glm::vec3 normal = glm::cross(edge1, edge2);
            tempTriangle = {v0,v1,v2, trigColour};
            tempTriangle.normal = normal;
            modelTriangles.push_back(tempTriangle);
        }
    }
    theObjFile.close();
    return modelTriangles;
}

std::map<std::string, Colour> readMaterial(const std::string& file) {
    std::map<std::string, Colour> palette;
    std::string myObj; // init string
    std::ifstream theObjFile(file); // read file
    std::string name;

    // read the file line by line
    // add a colour to the palette for each newmtl
    while (getline (theObjFile, myObj)) {
        if (myObj[0] == 'n') {
            name = split(myObj, ' ')[1];
        }
        if (myObj[0] == 'K') {
            std::vector<std::string> nrgb = split(myObj, ' ');
            // [Kd, 0.700000, 0.700000, 0.700000]
            float r = std::stof(nrgb[1])*255;
            float g = std::stof(nrgb[2])*255;
            float b = std::stof(nrgb[3])*255;
            Colour currColor = {name, int(r), int(g), int(b)};
            palette.insert({name,currColor});
        }
    }

    theObjFile.close();
    return palette;
}

