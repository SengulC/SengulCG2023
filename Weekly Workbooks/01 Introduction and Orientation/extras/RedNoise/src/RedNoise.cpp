#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <ModelTriangle.h>
#include <iostream>
#include <string>
#include <map>
#include "glm/ext.hpp"
#include "Utils.h"
#include <fstream>
#include "RayTriangleIntersection.h"
#include <algorithm>
#include <math.h>
#include "TexturePoint.h"
#include "TextureMap.h"

#define WIDTH 320
#define HEIGHT 240

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

std::vector<CanvasTriangle> twodTriangles;
int indexcheck;
bool toggle = true;
bool orbit = false;
std::vector<std::vector<float>> depthMatrix(WIDTH, std::vector<float>(HEIGHT, 0.0f));
//float focalLength = 2.0; // sphere
float focalLength = 1.8; //cornell
float scale = 240.0f;
//glm::vec3 cameraPosition {0.0, 0.5, 4.0}; /*sphere*/
glm::vec3 cameraPosition {0.0, 0.0, 4.0}; // cornell
glm::vec3 lightPosition {0.0, 0.6, 0.0}; // cornell reg
//glm::vec3 lightPosition(-0.5, -0.5,-0.5); // cornell blue box specular
//glm::vec3 lightPosition(1.5, -1.5,0.5); // cornell RED box specular
std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
std::vector<ModelTriangle> sphereTriangles = readObj("models/sphere.obj", mtls, 0.35, true);
auto modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35, false);
glm::mat3 cameraOrientation(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
        );
glm::mat3 origin(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
        );
// [right up forward]
glm::mat3 rotateX(
        1.0f, 0.0f, 0.0f,
        0.0f, cos(0.1), -sin(0.1),
        0.0f, sin(0.1), cos(0.1)
);
glm::mat3 rotateY(
        cos(0.1), 0.0f, sin(0.1),
        0.0f, 1.0f, 0.0f,
        -sin(0.1), 0.0f, cos(0.1)
);
//glm::vec3 lightPosition(0.0,0.5,2.5); sphere


void printVec3 (std::string string, glm::vec3 vec) {
    std::cout << string << " (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
}

// texture mapping
std::vector<std::vector<uint32_t>> linearListTo2DMatrix(const std::vector<uint32_t>& linearList, size_t height, size_t width) {
    std::vector<std::vector<uint32_t>> matrix(height, std::vector<uint32_t>(width, 0));

    // size_t bc height/width in texturemap r size_t
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            matrix[i][j] = linearList[i * width + j];
        }
    }

    return matrix;
}

std::string colorName(const uint32_t color) {
    std::string name;
    switch (color) {
        case 4278190335:
            name = "blue";
            break;
        case 4294901760:
            name = "red";
            break;
        case 4289901234:
            name = "gray";
            break;
        case 4278255615:
            name = "cyan";
            break;
        case 4294967295:
            name = "white";
            break;
        default:
            name = "unknown";
    }
    return name;
}

std::vector<int> unpack(const Colour& color) {
    std::vector<int> colour = {color.red, color.green, color.blue};
    return colour;
}

uint32_t pack(std::vector<int> colorgb) {
    return (255 << 24) + (int(colorgb[0]) << 16) + (int(colorgb[1]) << 8) + int(colorgb[2]);
}

uint32_t convertColor(const Colour& color) {
    return pack(unpack (color));
}

bool sortByY(const CanvasPoint& a, const CanvasPoint& b) {
    return a.y < b.y;
}

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
    float step = (to-from)/(static_cast<float>(numberOfValues));
    std::vector<float> vect;
    vect.push_back(from);
    float next = from;
    for (int i = 0; i < numberOfValues; i++) {
        float num = next+step;
        vect.push_back(num);
        next = next + step;
    }
    vect.push_back(to);
    return vect;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
    std::vector<glm::vec3> vect;
    std::vector<float> zero = interpolateSingleFloats(from[0], to[0], numberOfValues);
    std::vector<float> one = interpolateSingleFloats(from[1], to[1], numberOfValues);
    std::vector<float> two = interpolateSingleFloats(from[2], to[2], numberOfValues);

    for (int i = 0; i < numberOfValues; i++) {
        glm::vec3 temp(zero[i], one[i], two[i]);
        vect.push_back(temp);
    }
    return vect;
}

std::vector<CanvasPoint> interpolateCanvasPoint(CanvasPoint from, CanvasPoint to, int numberOfValues) {
    std::vector<CanvasPoint> vect;
    // create vector/list interpolations of floats: x y z
    std::vector<float> x = interpolateSingleFloats(from.x, to.x, numberOfValues);
    std::vector<float> y = interpolateSingleFloats(from.y, to.y, numberOfValues);
    std::vector<float> z = interpolateSingleFloats(from.depth, to.depth, numberOfValues);

    // create canvaspoints by drawing from each list of float interpolations: x y z @ index i
    for (int i = 0; i < numberOfValues; i++) {
        CanvasPoint temp(x[i], y[i], z[i]);
        vect.push_back(temp);
    }
    return vect;
}

TextureMap txtmap("models/texture.ppm");
auto textureMap = linearListTo2DMatrix(txtmap.pixels, txtmap.height, txtmap.width);

void drawTexturedLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, TexturePoint textureFrom, TexturePoint textureTo) {
    from.x = std::ceil(from.x);
    to.x = std::ceil(to.x);
    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float zDiff = to.depth - from.depth;
    float steps = std::max(std::abs(xDiff), std::abs(yDiff));
    float xSteps = xDiff / steps;
    float ySteps = yDiff / steps;
    auto Xtextures = interpolateSingleFloats(textureFrom.x, textureTo.x, steps+100);
    auto Ytextures = interpolateSingleFloats(textureFrom.y, textureTo.y, steps+100);

    for (int i = 0; i < static_cast<int>(std::ceil(steps)) + 1; i++) {
        float x = from.x + (xSteps * static_cast<float>(i));
        float y = from.y + (ySteps * static_cast<float>(i));

        int magnitude = static_cast<int>(sqrt(xDiff * xDiff + yDiff * yDiff + zDiff * zDiff));
        std::vector<float> depths = interpolateSingleFloats((from.depth), (to.depth), magnitude);

        float z = (depths[i]);
        int xval = static_cast<int>(std::round(x));
        int yval = static_cast<int>(std::round(y));

        if (xval < 320 && yval < 240 && xval > 0 && yval > 0) {
            if (i < txtmap.width && i < txtmap.height) {
                uint32_t color = textureMap[Xtextures[i]][Ytextures[i]];
                window.setPixelColour(xval, yval, color);
            }
        }
    }
    std::cout<<"done w drawline"<<std::endl;
}


std::vector<std::vector<float>> drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color, std::vector<std::vector<float>> depthMatrix, bool depth) {
    from.x = std::ceil(from.x);
    to.x = std::ceil(to.x);
    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float zDiff = to.depth - from.depth;
    float steps = std::max(std::abs(xDiff), std::abs(yDiff));
    float xSteps = xDiff / steps;
    float ySteps = yDiff / steps;

    std::vector<int> colorgb = unpack(color);
    uint32_t fincolor = pack(colorgb);
    for (int i = 0; i < static_cast<int>(std::ceil(steps)) + 1; i++) {
        float x = from.x + (xSteps * static_cast<float>(i));
        float y = from.y + (ySteps * static_cast<float>(i));

        int magnitude = static_cast<int>(sqrt(xDiff * xDiff + yDiff * yDiff + zDiff * zDiff));
        std::vector<float> depths = interpolateSingleFloats((from.depth), (to.depth), magnitude);

        float z = (depths[i]);
        int xval = static_cast<int>(std::round(x));
        int yval = static_cast<int>(std::round(y));

        if (xval < 320 && yval < 240 && xval > 0 && yval > 0) {
            // within drawing bounds
            if (depth) {
                if (depthMatrix[xval][yval] == 0.0f && colorName(fincolor) == "cyan") {
                    // if trying to draw ceiling cyan and coordinate depth is 0 (so init. state), paint it, otherwise dont.
                    depthMatrix[xval][yval] = z;
                    window.setPixelColour(xval, yval, fincolor);
                } else if (colorName(fincolor) == "cyan") {
                    continue;
                } else if (z > depthMatrix[xval][yval]) {
                    // casual depth check
                    depthMatrix[xval][yval] = z;
                    window.setPixelColour(xval, yval, fincolor);
                }
            } else {
                window.setPixelColour(xval, yval, fincolor);
            }
        }
    }
    return depthMatrix;
}

void drawPoint(DrawingWindow &window, CanvasPoint point, CanvasPoint otherPoint, Colour color) {
    if(point.depth > otherPoint.depth) {
        window.setPixelColour(static_cast<size_t>(point.x), static_cast<size_t>(point.y), convertColor(color));
    } else {
        std::cout<<"pos already set"<<std::endl;
    }
}


std::vector<std::vector<float>> drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color, std::vector<std::vector<float>> depthMatrix) {
    depthMatrix = drawLine(window, triangle.v0(), triangle.v1(), color, depthMatrix, true);
    depthMatrix = drawLine(window, triangle.v1(),triangle.v2(), color, depthMatrix, true);
    depthMatrix = drawLine(window, triangle.v0(), triangle.v2(), color, depthMatrix, true);
    return depthMatrix;
}

std::vector<std::vector<float>> drawFilled(DrawingWindow &window, CanvasTriangle triangle, Colour color, std::vector<std::vector<float>> depthMatrix) {
    std::vector<CanvasPoint> points = {triangle.v0(), triangle.v1(), triangle.v2()};
    std::sort(points.begin(), points.end(), sortByY); // sorted in ascending order of Ys

    // split triangle into 2 from middle vertex
    CanvasPoint middleV = points[1];
    int topHeight = static_cast<int> (std::abs(middleV.y - points[0].y));
    int bottomHeight = static_cast<int> (std::abs(points[2].y - middleV.y));

    // interpolate from 1st point to last point to find extra vertex
    std::vector<float> extraVInterpolatedX = interpolateSingleFloats(points[0].x, points[2].x,topHeight + bottomHeight);
    float extraVx = extraVInterpolatedX[topHeight];
    float ratio = (points[1].y - points[0].y)/(points[2].y-points[0].y);
    float extraVz = ratio * (points[2].depth - points[0].depth) + points[0].depth;
    CanvasPoint extraV = {extraVx, middleV.y, extraVz};

    // top triangle
    std::vector<CanvasPoint> topStart = interpolateCanvasPoint(points[0], extraV, topHeight+1);
    std::vector<CanvasPoint> topEnd = interpolateCanvasPoint(points[0], middleV, topHeight+1);
    for (int i = 0; i <= topHeight; i++) {
        depthMatrix = drawLine(window,topStart[i], topEnd[i], color, depthMatrix, true);
    }

    // bottom triangle
    std::vector<CanvasPoint> bottomStart = interpolateCanvasPoint(extraV, points[2], bottomHeight+1);
    std::vector<CanvasPoint> bottomEnd = interpolateCanvasPoint(middleV, points[2], bottomHeight+1);
    for (int i = 0; i <= bottomHeight; i++) {
        depthMatrix = drawLine(window, bottomStart[i], bottomEnd[i], color, depthMatrix, true);
    }

    return depthMatrix;
}

std::array<CanvasPoint, 3> sortAndLinkTextures(std::array<CanvasPoint, 3> vertices, std::vector<TexturePoint> texturePoints) {

    // sort
    std::sort(vertices.begin(), vertices.end(), sortByY);
    std::sort(texturePoints.begin(), texturePoints.end(), [](const TexturePoint &a, const TexturePoint &b) {
        return a.y < b.y;
    });

    // link
    for (int i=0; i <3; i++) {
        vertices[i].texturePoint = texturePoints[i];
    }

    return vertices;
}

std::vector<TexturePoint> interpolateTexturePoint (TexturePoint from, TexturePoint to, int numberOfValues) {
    std::vector<TexturePoint> vect;
    std::vector<float> x = interpolateSingleFloats(from.x, to.x, numberOfValues);
    std::vector<float> y = interpolateSingleFloats(from.y, to.y, numberOfValues);

    for (int i = 0; i < numberOfValues; i++) {
        TexturePoint temp(x[i], y[i]);
        vect.push_back(temp);
    }
    return vect;
}

void drawTextureFilled(DrawingWindow &window, CanvasTriangle triangle, const TextureMap& textureMap, const std::vector<TexturePoint>& texturePoints) {
    std::cout<<"drawing textured tri"<<std::endl;

    std::vector<std::vector<uint32_t>> textureMatrix = linearListTo2DMatrix(textureMap.pixels, textureMap.height, textureMap.width);
    // create canvas points and sort in terms of linked canvas points and texture points
    std::array<CanvasPoint, 3> points = sortAndLinkTextures(triangle.vertices, texturePoints);

    CanvasPoint middleV = points[1];
    int topHeight = static_cast<int> (std::abs(middleV.y - points[0].y));
    int bottomHeight = static_cast<int> (std::abs(points[2].y - middleV.y));

    // extra vertex
    std::vector<float> extraVInterpolatedX = interpolateSingleFloats(points[0].x, points[2].x,topHeight + bottomHeight);
    float extraVx = extraVInterpolatedX[topHeight];
    float ratio = (points[1].y - points[0].y)/(points[2].y-points[0].y);
    float extraVz = ratio * (points[2].depth - points[0].depth) + points[0].depth;
    CanvasPoint extraV = {extraVx, middleV.y, extraVz};

    uint32_t color;

    auto leftTextures = interpolateTexturePoint(points[0].texturePoint, points[2].texturePoint, topHeight+bottomHeight+100);
    auto rightTextures = interpolateTexturePoint(points[0].texturePoint, points[1].texturePoint, topHeight+bottomHeight+100);

    // top triangle
    std::vector<CanvasPoint> topStartVertices = interpolateCanvasPoint(points[0], extraV, topHeight+1);
    std::vector<CanvasPoint> topEndVertices = interpolateCanvasPoint(points[0], middleV, topHeight+1);

    for (int i = 0; i <= topHeight; i++) {
        TexturePoint from (leftTextures[i].x, leftTextures[i].y);
        TexturePoint to (rightTextures[i].x, rightTextures[i].y);
        drawTexturedLine(window, topStartVertices[i], topEndVertices[i], from, to);
    }
    std::cout<<"done w top"<<std::endl;

    // bottom triangle
    std::vector<CanvasPoint> bottomStartVertices = interpolateCanvasPoint(extraV, points[2], bottomHeight+1);
    std::vector<CanvasPoint> bottomEndVertices = interpolateCanvasPoint(middleV, points[2], bottomHeight+1);

    for (int i = 0; i <= bottomHeight; i++) {
        TexturePoint from (leftTextures[i].x, leftTextures[i].y);
        TexturePoint to (rightTextures[i].x, rightTextures[i].y);
        drawTexturedLine(window, bottomStartVertices[i], bottomEndVertices[i], from, to);
    }
    std::cout<<"done w top"<<std::endl;
}

CanvasTriangle randomTriangle() {
    CanvasPoint v0 = {static_cast<float>(rand()%320),static_cast<float>(rand()%240),0,0};
    CanvasPoint v1 = {static_cast<float>(rand()%320),v0.y,0,0};
    CanvasPoint v2 = {static_cast<float>(rand()%320),static_cast<float>(rand()%240),0,0};
    CanvasTriangle triangle = {v0,v1,v2};
    return triangle;
}

Colour randomColor() {
    Colour color = {rand()%256, rand()%256, rand()%256};
    return color;
}

CanvasTriangle modelToCanvasTriangle(ModelTriangle mTri) {
    // model = vec3
    // canvas = canvaspoint
    CanvasTriangle cTri;
    std::array<glm::vec3,3> vecTices {mTri.vertices[0],mTri.vertices[1], mTri.vertices[2]};
    std::array<CanvasPoint,3> points {CanvasPoint(vecTices[0].x,vecTices[0].y,vecTices[0].z),
                                      CanvasPoint(vecTices[1].x,vecTices[1].y,vecTices[1].z),
                                      CanvasPoint(vecTices[2].x,vecTices[2].y,vecTices[2].z)};
    cTri.vertices = points;
    return cTri;
}


// rednoise
void bAndWdraw(DrawingWindow &window) {
    window.clearPixels();
    std::vector<float> colors = interpolateSingleFloats(255.0, 0.0, window.width);
    for (size_t y = 0; y < window.height; y++) {
        for (size_t x = 0; x < window.width; x++) {
            float red = colors[x];
            float green = colors[x];
            float blue = colors[x];
            uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
            window.setPixelColour(x, y, colour);
        }
    }
}

void rainbowDraw(DrawingWindow &window) {
    window.clearPixels();
    glm::vec3 red(255, 0, 0);
    glm::vec3 blue(0, 0, 255);
    glm::vec3 green(0, 255, 0);
    glm::vec3 yellow(255, 255, 0);

    // fill 1st column
    std::vector<glm::vec3> redyellow = interpolateThreeElementValues(red, yellow, window.height);
    for (size_t y = 0; y < window.height; y++) {
        window.setPixelColour(0, y, (255 << 24) + (int(redyellow[y][0]) << 16) + (int(redyellow[y][1]) << 8) + int(redyellow[y][2]));
    }

    // fill last column
    std::vector<glm::vec3> bluegreen = interpolateThreeElementValues(blue, green, window.height);
    for (size_t y = 0; y < window.height; y++) {
        window.setPixelColour((window.width)-1, y, (255 << 24) + (int(bluegreen[y][0]) << 16) + (int(bluegreen[y][1]) << 8) + int(bluegreen[y][2]));
    }

    // fill each row
    for (size_t y = 0; y < window.height; y++) {
        for (size_t x = 0; x < window.width; x++) {

            uint32_t from = window.getPixelColour(0, y);
            uint32_t to = window.getPixelColour(window.width-1, y);

            // XXXXXXXXX XXXXXXXXX XXXXXXXXX XXXXXXXXX
            int r = (from >> 16) & 0xFF;
            int g = (from >> 8) & 0xFF;
            int b = from & 0xFF;
            glm::vec3 vecFrom(r,g,b);

            r = (to >> 16) & 0xFF;
            g = (to >> 8) & 0xFF;
            b = to & 0xFF;
            glm::vec3 vecTo(r,g,b);

            std::vector<glm::vec3> colors = interpolateThreeElementValues(vecFrom, vecTo, window.width);
            // have a list of vec3s, need to extract each vec3 to get rgb data
            glm::vec3 color = colors[x];
            float redY = color[0];
            float greenY = color[1];
            float blueY = color[2];
            uint32_t colour = (255 << 24) + (int(redY) << 16) + (int(greenY) << 8) + int(blueY);
            window.setPixelColour(x, y, colour);
        }
    }
}

// rasterized render
glm::mat3 LookAt(glm::mat3 cameraOrientation, glm::vec3 lookAtMe, glm::vec3 cameraPosition) {
    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 right;

    forward = glm::normalize(cameraPosition - lookAtMe);

    glm::vec3 vertical(0.0f,1.0f,0.0f);
    right = glm::normalize((glm::cross(vertical, forward)));
    up = glm::normalize(glm::cross(forward, right));

    // [right up forward]
    cameraOrientation = glm::mat3(right, up, forward);
    return cameraOrientation;
}

CanvasPoint getCanvasIntersectionPoint(CanvasPoint vertexPosition, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focalLength, float scale) {
    // converting from a 3D vertex position into a 2D canvas position...
    float x, y;
    CanvasPoint intersection;

    // VertexPos - CameraPos
    glm::vec3 distance = glm::vec3(vertexPosition.x-cameraPosition.x, vertexPosition.y-cameraPosition.y, vertexPosition.depth-cameraPosition.z);
    //... Then, multiply this vector by orientation matrix
    distance = distance * cameraOrientation;

    // Calculate the 2D coordinates on the image plane
    x = (focalLength/(distance.z)) * (distance.x);
    y = (focalLength/(distance.z)) * (distance.y);

    // Scaling and shifting
    x = x * -scale + (WIDTH / 2); // negative scale bc x-axis was flipped
    y = y * scale + (HEIGHT/ 2);

    // Populate and return intersection
    intersection.x = x;
    intersection.y = y;
    intersection.depth = 1/std::abs(distance.z);
    return intersection;
}

std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3, std::vector<std::vector<float>>> drawRasterizedScene(DrawingWindow &window, std::vector<ModelTriangle> modelTriangles, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focalLength, float scale, std::vector<std::vector<float>> depthMatrix, bool orbit) {
    window.clearPixels();
    std::cout<<"rasterizer"<<std::endl;
    depthMatrix = std::vector<std::vector<float>> (WIDTH, std::vector<float>(HEIGHT, 0.0f));
    std::vector<CanvasTriangle> twodTriangles;

    // conversion and projection onto canvas
    for (ModelTriangle &modelTriangle : modelTriangles) {
        CanvasTriangle canvasTriangle = modelToCanvasTriangle(modelTriangle);
        for (int i = 0; i < 3; i++) {
            canvasTriangle.vertices[i] = getCanvasIntersectionPoint(canvasTriangle.vertices[i], cameraPosition,
                                                                    cameraOrientation, focalLength, scale);
        }
        twodTriangles.push_back(canvasTriangle);
        depthMatrix = drawFilled(window, canvasTriangle, modelTriangle.colour, depthMatrix);
        drawStroked(window, canvasTriangle, modelTriangle.colour, depthMatrix);
    }

    // ORBIT
    float angle = 0.5f;
    if (orbit) {
        cameraPosition = glm::mat3 (
                cos(angle), 0.0f, -sin(angle),
                0.0f, 1.0f, 0.0f,
                sin(angle), 0.0f, cos(angle)
        ) * cameraPosition;
        cameraOrientation = LookAt(cameraOrientation, glm::vec3(0,0,0), cameraPosition);
    }
    std::cout<<"end of rasterizer"<<std::endl;
    return std::make_tuple(twodTriangles, cameraPosition, cameraOrientation, depthMatrix);
}

// ray-traced render
bool validTUV(const RayTriangleIntersection& intersect, float dist, bool shadow, size_t currIndex, bool mirror, std::string mirrorColor) {
    glm::vec3 tuv = intersect.intersectionPoint;
    float t=tuv.x; float u=tuv.y; float v=tuv.z;
    bool uTest = (u >= 0.0) && (u <= 1.0);
    bool vTest = (v >= 0.0) && (v <= 1.0);
    bool addTest = (u + v) <= 1.0;
    bool tPos = t >= 0.0;
    bool sameTri = (intersect.triangleIndex != currIndex);
    bool shadowT, mirrorT = true;

    if (shadow) {shadowT = abs(t) < dist;} else { shadowT = true; }
//    if (mirror) {mirrorT = intersect.intersectedTriangle.colour.name!=mirrorColor;} else { mirrorT = true; }


    return (uTest && vTest && addTest && tPos && shadowT && mirrorT && sameTri);
}

RayTriangleIntersection getClosestValidIntersection(glm::vec3 startPosition, glm::vec3 endPosition, glm::vec3 rayDirection, const std::vector<ModelTriangle>& triangles, bool shadow, size_t currIndex, bool mirror, std::string mirrorColor) {
    rayDirection = glm::normalize(rayDirection);
    glm::vec3 e0, e1, SPVector, possibleSolution;
    std::vector<RayTriangleIntersection> possibleSolutions, convertedSolutions1, convertedSolutions2;

    size_t index = 0;
    // loop through all triangles for given ray direction
    for (ModelTriangle triangle : triangles) {
        e0 = triangle.vertices[1] - triangle.vertices[0];
        e1 = triangle.vertices[2] - triangle.vertices[0];
        SPVector = startPosition - triangle.vertices[0];
        glm::mat3 DEMatrix(-rayDirection, e0, e1);
        possibleSolution = glm::inverse(DEMatrix) * SPVector;

        possibleSolutions.emplace_back(possibleSolution, glm::distance(startPosition, endPosition),
                                       possibleSolution.x, possibleSolution, triangle, index, true);
        index++;
    }

    glm::vec3 convertedPoint, position;
    RayTriangleIntersection convertedIntersection;
    // loop through the possible solutions and check if they're valid
    // tuv = intersectionPoint.xyz
    for (const RayTriangleIntersection &tuv: possibleSolutions) {
        if (validTUV(tuv, glm::distance(endPosition, startPosition), shadow, currIndex, mirror, mirrorColor)) {
            convertedIntersection = tuv; // retain all other data for RayTriangleIntersection struct just overwrite the vec3 w/ conversion
            // conversion #1: r = p0 + u(p1-p0) + v(p2-p0)
            convertedPoint = tuv.intersectedTriangle.vertices[0] + (tuv.intersectionPoint.y * e0) + (tuv.intersectionPoint.z * e1);
            convertedIntersection.intersectionPoint = (convertedPoint);
            convertedSolutions1.push_back(convertedIntersection);

            convertedIntersection = tuv; // same reason as above^
            // conversion #2: position = startPosition + t * rayDirection;
            position = startPosition + tuv.intersectionPoint.x * rayDirection;
            convertedIntersection.intersectionPoint = (position);
            convertedSolutions2.push_back(convertedIntersection);
        }
    }

    RayTriangleIntersection closestIntersection, closestIntersection2;
    // check if there ~was~ a valid converted solution
    if (!convertedSolutions1.empty() && !convertedSolutions2.empty()) {
        // lamba func to find smallest t val in convertedSolutions1/2
        auto intersection1 = std::min_element(
                convertedSolutions1.begin(), convertedSolutions1.end(),
                [](const RayTriangleIntersection& a, const RayTriangleIntersection& b) {return (a.t) < (b.t);});

        auto intersection2 = std::min_element(
                convertedSolutions2.begin(), convertedSolutions2.end(),
                [](const RayTriangleIntersection& a, const RayTriangleIntersection& b) {return (a.t) < (b.t);});

        // -> is some wrap-iter thing apparently (source: CLion)
        closestIntersection = RayTriangleIntersection(intersection1->intersectionPoint, intersection1->distanceFromStart, intersection1->t, intersection1->tuv, intersection1->intersectedTriangle, intersection1->triangleIndex, intersection1->valid);
        closestIntersection2 = RayTriangleIntersection(intersection2->intersectionPoint, intersection2->distanceFromStart, intersection2->t,  intersection2->tuv, intersection2->intersectedTriangle, intersection2->triangleIndex, intersection2->valid);
    } else {
        // return error codes within RayTriangleIntersection (primarily valid: false)
        RayTriangleIntersection erroneous = RayTriangleIntersection({0,0,0} , 1000, 1000, {0,0,0}, triangles[0], INT_MAX, false);
        return erroneous;
    }

    return closestIntersection2;
}

glm::vec3 convertToDirectionVector(CanvasPoint startPoint, float scale, float focalLength, glm::vec3 endPoint, glm::mat3 cameraOrientation) {
    // Reversed scaling and shifting
    float x = ((startPoint.x) - (WIDTH/2)) / (-scale);
    float y = ((startPoint.y) - (HEIGHT/2)) / (scale);

    // Calculate direction vector
    float zDiff = startPoint.depth - endPoint.z;
    x = x / (focalLength / zDiff);
    y = y / (focalLength / zDiff);
    glm::vec3 direction (x, y, -focalLength);

    direction = glm::inverse(cameraOrientation) * direction;
//     direction += endPoint;

    return direction;
}

float calculateBrightness(glm::vec3 lightPosition, glm::vec3 cameraPosition, glm::vec3 vertex, glm::vec3 normal) {
    float radius = glm::length(lightPosition - vertex);
    float brightness = 5/(3*M_PI*radius*radius);
    glm::vec3 surfaceToLight = lightPosition - vertex;
    float angle = glm::normalizeDot(normal, surfaceToLight);
    glm::vec3 lightToSurface = vertex-lightPosition;
    glm::vec3 reflectionVector (lightToSurface - ((2*normal)*(glm::dot(lightToSurface, normal))));
    glm::vec3 surfaceToCam(cameraPosition-vertex);
    float specular = glm::normalizeDot(reflectionVector, surfaceToCam);
    specular = pow(specular, 512);

    // restrict a given value between 0-1
    float intensity = (brightness*angle*5)+specular;
    if (intensity > 1) {
        intensity = 1;
    } else if (intensity < 0.1) {
        intensity = 0.1;
    }

    return intensity;
}

std::pair<std::vector<glm::vec3>, std::vector<float>> createLights(float startX, float endX, float startY, float endY, float startZ, float endZ) {
    std::vector<glm::vec3> lights;
    std::vector<float> weights;
    int steps = 2;
    std::vector<float> width = interpolateSingleFloats(startX, endX, steps);
    std::vector<float> height = interpolateSingleFloats(startY, endY, steps);
    std::vector<float> depth = interpolateSingleFloats(startZ, endZ, steps);
//    std::cout<<height.size()<<std::endl;
    float intensity;

    for (auto x : width) {
        for (auto y : height) {
            for (auto z : depth) {
                if (y==startY) {
                    intensity = 0.5;
                } else if (y==startY) {
                    intensity = 0.4;
                } else if (y==startY) {
                    intensity = 0.3;
                } else {
                    intensity = 0.2;
                }
                lights.emplace_back(x, y, z);
                weights.push_back(intensity);
            }
        }
    }
    return std::make_pair(lights, weights);
}

std::tuple<bool, float> shootShadowRays(std::vector<glm::vec3> allOfTheLights, std::vector<float> allOfTheWeights, glm::vec3 cameraPosition, RayTriangleIntersection intersection, const std::vector<ModelTriangle>& triangles) {
//    std::vector<std::pair<float, float>> shadowData; // floats repping: light weight, pixel brightness wrt current light
    std::vector<float> validIntersectionsRegularLighting;
    std::vector<float> weights;
//    int count = 0;
//    float weight, brightness = 0, weightedBrightness = 0;

    // loop over canvas
    // shoot a ray from camera to model
    // if a valid intersection has been made from cam-model
    // shoot a ray from model surface to lights
    // so loop over lights and shoot a ray for each surface-to-light
    // if a valid intersection has been made from surface-light
    // add current light to validShadows list
    // now we have a list of validShadows. calculate weight as: validShadows.size/allOfTheLights.size
    // final shadow intensity for current pixel = weighted calculation calculated by...
    // shadow intensity = (1-weight) * avg of current surface point's regular light intensity

    // got back shadow intensity for given surface-to-lights
    // paint shadowed pixels as RGB * shadow intensity
    // and other pixels as RGB * regular light intensity

    float brightness;
    float weight;
    int i = 0;
    std::vector<float> allOfTheValidWeights;
    for (auto lightPosition : allOfTheLights) {
        glm::vec3 shadowRay = glm::normalize(lightPosition-(intersection.intersectionPoint));
        RayTriangleIntersection closestObjIntersection = getClosestValidIntersection((intersection.intersectionPoint), lightPosition, shadowRay, triangles, true, intersection.triangleIndex, false, "");
        if (closestObjIntersection.valid)
        {
            brightness = calculateBrightness(lightPosition, cameraPosition, intersection.intersectionPoint, intersection.intersectedTriangle.normal);
            weight = glm::distance(lightPosition,intersection.intersectionPoint);
            weights.push_back(weight);
            allOfTheValidWeights.push_back(allOfTheWeights[i]);
            validIntersectionsRegularLighting.push_back(brightness);
        }
        i++;
    }

    if (validIntersectionsRegularLighting.empty()) {
        return std::make_tuple(!validIntersectionsRegularLighting.empty(), 0);
    }

//    float avgRegIntensity = 0;
//    for (float regIntensity : validIntersectionsRegularLighting) {
//        avgRegIntensity += regIntensity;
//    }
    float weighted = 0;
    for (int count=0; count<validIntersectionsRegularLighting.size(); count++) {
        weighted += (validIntersectionsRegularLighting[count]/* * weights[count]*/ * allOfTheValidWeights[count]);
    }

    weighted = weighted/validIntersectionsRegularLighting.size();
    float weightedBrightness = weighted;
//    std::cout<<weightedBrightness<<std::endl;
//    avgRegIntensity = avgRegIntensity/validIntersectionsRegularLighting.size();
//    float avgRegIntensity = validIntersectionsRegularLighting[0];
//    float weight = 1-(validIntersectionsRegularLighting.size())/allOfTheLights.size();
//    float weightedBrightness = avgRegIntensity * (weighted);
//    weightedBrightness = 1-weightedBrightness;

    if (weightedBrightness > 1) {
        weightedBrightness = 1;
    } else if (weightedBrightness < 0.1) {
        weightedBrightness = 0.1;
    }

    return std::make_tuple(!validIntersectionsRegularLighting.empty(), weightedBrightness);
}

void drawRaytracedSceneWithSoft(DrawingWindow &window, const std::vector<ModelTriangle>& triangles, float scale, float focalLength, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, glm::vec3 lightPosition, std::pair<std::vector<glm::vec3>, std::vector<float>> allOfTheLightData) {
    std::cout <<"in raytracer"<< std::endl;
    window.clearPixels();
    std::vector<float> brightnesses;
    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            CanvasPoint point(x, y, focalLength);
            glm::vec3 rayDirection =  convertToDirectionVector(point, scale, focalLength, cameraPosition, cameraOrientation);
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, glm::vec3(x,y,focalLength), rayDirection, triangles, false, 10000, false, "");
            if (intersection.valid) {
                // shoot a bunch of shadow rays...
//                auto allOfTheLightData = createLights(-0.2, 0.2, 0.2, 0.4, 0.4, 0.5);
                auto allOfTheLights = allOfTheLightData.first;
                auto allOfTheWeights = allOfTheLightData.second;

//                for (auto light : allOfTheLights) {
//                    CanvasPoint point (light.x, light.y, light.z);
//                    point = getCanvasIntersectionPoint(point, cameraPosition, cameraOrientation, focalLength, scale);
//                    window.setPixelColour(point.x, point.y, convertColor({255,255,255}));
//                }

                //glm::vec3 shadowRay = glm::normalize(lightPosition-(intersection.intersectionPoint));
                //RayTriangleIntersection closestObjIntersection = getClosestValidIntersection((intersection.intersectionPoint), lightPosition, shadowRay, triangles, true, intersection.triangleIndex);

                glm::vec3 normal = intersection.intersectedTriangle.normal;
                float intensity = /*calculateBrightness(lightPosition, cameraPosition, intersection.intersectionPoint, normal)*/1;

                auto shadowData = shootShadowRays(allOfTheLights, allOfTheWeights, cameraPosition, intersection, triangles);
                //if there were/was a valid shadow, use below intensity, otherwise use above
                bool validShadow = std::get<0>(shadowData);
                float shadowIntensity = (std::get<1>(shadowData));
                Colour currColor = intersection.intersectedTriangle.colour;

                if (validShadow && shadowIntensity!=1) {
                    //std::cout<<shadowIntensity<<std::endl;
                    // SOFT SHADOW: how many shadow rays respond with an intersection?
                    uint32_t shadow = convertColor(Colour(currColor.red * shadowIntensity, currColor.green * shadowIntensity, currColor.blue * shadowIntensity));
                    window.setPixelColour(x, y, shadow);
                } else if (intersection.intersectedTriangle.colour.name=="White") {
                    // hardcoding lightbox lolf
                    window.setPixelColour(x, y, convertColor(Colour(255,255,255)));
                } else {
                    uint32_t color = convertColor(Colour(currColor.red * intensity, currColor.green * intensity, currColor.blue * intensity));
                    window.setPixelColour(x, y, color);
                }
            }
        }
    }
    std::cout<<"end of ray trace"<<std::endl;
}

void drawRaytracedScene(DrawingWindow &window, const std::vector<ModelTriangle>& triangles, float scale, float focalLength, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, glm::vec3 lightPosition) {
    std::cout <<"in raytracer"<< std::endl;
    window.clearPixels();
    std::vector<float> brightnesses;
    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            CanvasPoint point(x, y, focalLength);
            glm::vec3 rayDirection =  convertToDirectionVector(point, scale, focalLength, cameraPosition, cameraOrientation);
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, glm::vec3(x,y,focalLength), rayDirection, triangles, false, 10000, false, "");
            if (intersection.valid) {
                glm::vec3 shadowRay = glm::normalize(lightPosition-(intersection.intersectionPoint));
                RayTriangleIntersection closestObjIntersection = getClosestValidIntersection((intersection.intersectionPoint), lightPosition, shadowRay, triangles, true, intersection.triangleIndex, false, "");

                Colour currColor;
                std::string mirrorColor = "";
                float shadowIntensity =1;
                glm::vec3 normal = intersection.intersectedTriangle.normal;
                float intensity=1;
                if (closestObjIntersection.valid &&
                    glm::distance(closestObjIntersection.intersectionPoint, intersection.intersectionPoint) >= 0.0001) {
                    // if SHADOW
                    Colour currColor = intersection.intersectedTriangle.colour;
                    shadowIntensity = 0.3;
                }
                // if mirror
                if (intersection.intersectedTriangle.colour.name == mirrorColor) {
                    auto camToSurface = intersection.intersectionPoint - cameraPosition;
                    auto reflectionRay = (camToSurface - ((2 * normal) * (glm::normalizeDot(camToSurface, normal))));
                    reflectionRay += 0.0001f;
                    auto mirrIntersect = getClosestValidIntersection(intersection.intersectionPoint, {0, 0, 0},
                                                                     reflectionRay, triangles, false,
                                                                     intersection.triangleIndex, true, mirrorColor);
                    if (mirrIntersect.valid) {
                        intensity = calculateBrightness(lightPosition, cameraPosition, mirrIntersect.intersectionPoint,
                                                        mirrIntersect.intersectedTriangle.normal);
                        currColor = mirrIntersect.intersectedTriangle.colour;
                    } else {
                        currColor = {0, 0, 0};
                    }
                // if not mirror
                } else {
                    intensity = calculateBrightness(lightPosition, cameraPosition, intersection.intersectionPoint, normal);;
                    currColor = intersection.intersectedTriangle.colour;
                }
                uint32_t color = convertColor(Colour(currColor.red * intensity * shadowIntensity, currColor.green * intensity * shadowIntensity, currColor.blue * intensity * shadowIntensity));
                window.setPixelColour(x, y, color);
            }
        }
    }
    std::cout<<"end of ray trace"<<std::endl;
}

std::vector<float> calculateBrightnesses(glm::vec3 lightPosition, glm::vec3 cameraPosition, RayTriangleIntersection intersection) {
    std::vector<float> brightnesses;
    int index = 0;
    for (auto vertex: intersection.intersectedTriangle.vertices) {
        float radius = glm::length(lightPosition - vertex);
        float brightness = 5 / (3 * M_PI * radius * radius); // PROXIMITY
        glm::vec3 surfaceToLight = lightPosition - vertex;
        glm::vec3 normal = intersection.intersectedTriangle.vertexNormals[index];
        float angle = glm::normalizeDot(normal, surfaceToLight); // AOL
        glm::vec3 lightToSurface = vertex - lightPosition;
        glm::vec3 reflectionVector(lightToSurface - ((2 * normal) * (glm::dot(lightToSurface, normal))));
        glm::vec3 surfaceToCam(cameraPosition - vertex);
        float specular = glm::normalizeDot(reflectionVector, surfaceToCam);
        specular = pow(specular, 1024); // SPECULAR
        float intensity = (brightness * angle * 5) + specular;
        if (intensity > 1) {
            intensity = 1;
        } else if (intensity < 0.1) {
            intensity = 0.1;
        }

        brightnesses.push_back(intensity);
        index++;
    }
    return brightnesses;
}

void drawGouraucedScene(DrawingWindow &window, const std::vector<ModelTriangle>& triangles, float scale, float focalLength, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, glm::vec3 lightPosition) {
    std::cout<<"in drawGouraucedScene"<<std::endl;
    window.clearPixels();

    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            CanvasPoint point(x, y, focalLength);
            glm::vec3 rayDirection =  convertToDirectionVector(point, scale, focalLength, cameraPosition, cameraOrientation);
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, glm::vec3(x,y,focalLength), rayDirection, triangles, false, 10000, false, "");
            if (intersection.valid) {
                std::vector<float> vertexBrightnesses = calculateBrightnesses(lightPosition, cameraPosition, intersection);
                float c1 = vertexBrightnesses[0], c2 = vertexBrightnesses[1], c3 = vertexBrightnesses[2];

                glm::vec3 tuv = intersection.tuv;
                float u = tuv.y, v = tuv.z, w = 1-(u+v);

                // barycentric coords
                float intensity = (u * c2) + (v * c3) + (w * c1);

                Colour currColor = intersection.intersectedTriangle.colour;
                uint32_t color = convertColor(Colour(currColor.red * intensity, currColor.green * intensity, currColor.blue * intensity));
                window.setPixelColour(x, y, color);
            }
        }
    }
    std::cout<<"end of ray trace"<<std::endl;
}


void drawPhongdScene(DrawingWindow &window, const std::vector<ModelTriangle>& triangles, float scale, float focalLength, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, glm::vec3 lightPosition) {
    std::cout<<"in drawPhongdScene"<<std::endl;
    window.clearPixels();

    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            CanvasPoint point(x, y, focalLength);
            glm::vec3 rayDirection =  convertToDirectionVector(point, scale, focalLength, cameraPosition, cameraOrientation);
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, glm::vec3(x,y,focalLength), rayDirection, triangles, false, 10000, false, "");
            if (intersection.valid) {
                glm::vec3 tuv = intersection.tuv;
                float u = tuv.y, v = tuv.z, w = 1-(u+v);

                auto normals = intersection.intersectedTriangle.vertexNormals;
                // barycentric normal
                glm::vec3 normal = (u * normals[1]) + (v * normals[2]) + (w * normals[0]);

                float radius = glm::length(lightPosition - intersection.intersectionPoint);
                float brightness = 5/(3*M_PI*radius*radius);
                glm::vec3 surfaceToLight = lightPosition - intersection.intersectionPoint;
                float angle = glm::normalizeDot(normal, surfaceToLight);
                glm::vec3 lightToSurface = intersection.intersectionPoint-lightPosition;
                glm::vec3 reflectionVector (lightToSurface - ((2*normal)*(glm::dot(lightToSurface, normal))));
                glm::vec3 surfaceToCam(cameraPosition-intersection.intersectionPoint);
                float specular = glm::normalizeDot(reflectionVector, surfaceToCam);
                specular = pow(specular, 2048);

                // restrict a given value between 0-1
                float intensity = (brightness*angle*5)+specular;
                if (intensity > 1) {
                    intensity = 1;
                } else if (intensity < 0.1) {
                    intensity = 0.1;
                }

                Colour currColor = intersection.intersectedTriangle.colour;
                uint32_t color = convertColor(Colour(currColor.red * intensity, currColor.green * intensity, currColor.blue * intensity));
                window.setPixelColour(x, y, color);
            }
        }
    }
    std::cout<<"end of ray trace"<<std::endl;
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        // translation: up/down/left/right
        if (event.key.keysym.sym == SDLK_UP) {
            cameraPosition += glm::vec3{0,0.1,0};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        else if (event.key.keysym.sym == SDLK_DOWN) {
            cameraPosition -= glm::vec3{0,0.1,0};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        else if (event.key.keysym.sym == SDLK_LEFT) {
            cameraPosition -= glm::vec3{0.1,0,0};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        else if (event.key.keysym.sym == SDLK_RIGHT) {
            cameraPosition += glm::vec3{0.1,0,0};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
            // translation: z axis: ;/'
        else if (event.key.keysym.sym == SDLK_SEMICOLON) {
            cameraPosition -= glm::vec3{0,0,0.1};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        else if (event.key.keysym.sym == SDLK_QUOTE) {
            cameraPosition += glm::vec3{0,0,0.1};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
            // rotation </>
        else if (event.key.keysym.sym == SDLK_COMMA) {
            cameraPosition = rotateX * cameraPosition;
        }
        else if (event.key.keysym.sym == SDLK_PERIOD) {
            cameraPosition = rotateY * cameraPosition;
        }
            // toggle orbit
        else if (event.key.keysym.sym == 'o') {
            if (orbit) { orbit = false; } else { orbit = true; }
        }
            // lookAt origin
        else if (event.key.keysym.sym == 'l') {
            cameraOrientation = LookAt(cameraOrientation, glm::vec3(0,0,0), cameraPosition);
        }
            // tilting/panning
        else if (event.key.keysym.sym == 't') {
            cameraOrientation = rotateX * cameraOrientation;
        }
        else if (event.key.keysym.sym == 'p') {
            cameraOrientation = rotateY * cameraOrientation;
        }
            // triangle stuff
        else if (event.key.keysym.sym == 'u') {
            drawStroked(window, randomTriangle(), randomColor(), depthMatrix);
        }
        else if (event.key.keysym.sym == 'f') {
            drawFilled(window, randomTriangle(), randomColor(), depthMatrix);
        }
            // clear
        else if (event.key.keysym.sym == 'c') {
            window.clearPixels();
            depthMatrix = std::vector<std::vector<float>> (WIDTH, std::vector<float>(HEIGHT, 0.0f));
            indexcheck = 0;
            cameraPosition = {0.0, 0.0, 4.0};
            cameraOrientation = glm::mat3 (
                    1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 1.0f
            );
        }
            // lighting. z+-
        else if (event.key.keysym.sym == 'a'){
            lightPosition -= glm::vec3(0,0,0.1);
            std::cout<<"light: " << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z <<std::endl;
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
        else if (event.key.keysym.sym == 'd'){
            lightPosition += glm::vec3(0,0,0.1);
            std::cout<<"light: " << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z <<std::endl;
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
            // lighting. y+-
        else if (event.key.keysym.sym == 'w'){
            lightPosition -= glm::vec3(0,0.1,0);
            std::cout<<"light: " << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z <<std::endl;
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
        else if (event.key.keysym.sym == 's'){
            lightPosition += glm::vec3(0,0.1,0);
            std::cout<<"light: " << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z <<std::endl;
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
        else if (event.key.keysym.sym == 'h'){
            drawRaytracedScene(window, modelTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false); //fullscreen
	SDL_Event event;

    // RASTERIZER
//    auto tuple = drawRasterizedScene(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix, orbit);
//    twodTriangles = std::get<0>(tuple);
//    cameraPosition = std::get<1>(tuple);
//    cameraOrientation = std::get<2>(tuple);
//    depthMatrix = std::get<3>(tuple);

    // RAYTRACER
//    drawPhongdScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);

    std::vector<glm::vec3> lights = {{-0.1,0.5,2.5}, {0.0,0.5,2.5}, {0.1,0.5,2.5},
                                     {-0.1,0.6,2.5}, {0.0,0.6,2.5}, {0.1,0.6,2.5},
                                     {-0.1,0.5,2.5}, {0.0,0.5,2.5}, {0.1,0.5,2.5}};
//    std::vector<glm::vec3> lights = {{-0.6, -0.5,-0.5}, {1.5, -1.5,0.5}}; // cornell blue box specular
    int count = 0;

    //Canvas Point		Texture Point
    //(160, 10)	→	(195, 5)
    //(300, 230)	→	(395, 380)
    //(10, 150)	→	(65, 330

    // (DrawingWindow &window, CanvasTriangle triangle, const TextureMap& textureMap, const std::vector<TexturePoint>& texturePoints
//    std::vector<TexturePoint> texturePoints {TexturePoint{195,5}, TexturePoint{395,380}, TexturePoint{65,330}};
//    CanvasTriangle triangle (CanvasPoint(160,10), CanvasPoint(300,230), CanvasPoint(10,150));
//    drawTextureFilled(window, triangle, TextureMap("models/texture.ppm"), texturePoints);

//    auto lights1 = createLights(-0.2, 0.2, 0.2, 0.4, 0.4, 0.5);
//    auto lights2 = createLights(-0.2, 0.2, 0.4, 0.6, 0.4, 0.5);
//    auto lights3 = createLights(-0.2, 0.2, 0.6, 0.8, 0.4, 0.5);
//    std::vector<std::pair<std::vector<glm::vec3>, std::vector<float>>> allOfTheLights {lights1, lights2, lights3};
    drawRaytracedScene(window, modelTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
//    drawGouraucedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);

    while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);

//        if (count < allOfTheLights.size()) {
//            printVec3("light", lights[count]);
//            drawRaytracedSceneWithSoft(window, modelTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition, allOfTheLights[count]);
//            window.savePPM("./CornellLighting/soft_shadows" + std::to_string(count) + ".ppm");
//            window.saveBMP("./CornellLighting/soft_shadows" + std::to_string(count) + ".bmp");
//        } else {
//            std::cout<<"done"<<std::endl;
//        }
//        count++;

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
