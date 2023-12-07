#include "OurObject.h"

#define WIDTH 320
#define HEIGHT 240

void printVec3 (std::string string, glm::vec3 vec) {
    std::cout << string << " (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
}

glm::vec3 calculateVertexNormal(const std::vector<ModelTriangle>& modelTriangles, glm::vec3 vertex) {
    glm::vec3 normal (0,0,0);
    int count = 0;
    for (ModelTriangle tri : modelTriangles) {
        if (std::find(tri.vertices.begin(), tri.vertices.end(), vertex) != tri.vertices.end()) {
            // tri.vertices contains vertex
            count++;
            normal += tri.normal;
        }
    }
    normal = normal/count;
    return normal;
}

std::vector<ModelTriangle> readObj(const std::string& file, std::map<std::string, Colour> mtls, float scale, bool sphere) {
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
        if (sphere) {
            trigColour = Colour(255,0,0);
        }
        if (myObj[0] == 'u') {
            std::vector<std::string> colorInfo = split(myObj, ' ');
            trigColour = mtls[colorInfo[1]];
        } else if (myObj[0] == 'v') {
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

    // for each vertex, calc vertex normal
    std::vector<ModelTriangle> modelTrianglesWithNormals;
    if (sphere) {
        for (ModelTriangle t : modelTriangles) {
            // for each triangle
            for (glm::vec3 v : t.vertices) {
                // for each vertex in that triangle,
                // calculate normal
                glm::vec3 normal = calculateVertexNormal(modelTriangles, v);
                // push normal of current vertex onto vertexNormals list of copied tri
                t.vertexNormals.push_back(normal);
            }
            modelTrianglesWithNormals.push_back(t); // add copied and normal'd tri to list
            // go to next tri
        }
    } else {
        modelTrianglesWithNormals = modelTriangles;
    }

    theObjFile.close();
    return modelTrianglesWithNormals;
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

