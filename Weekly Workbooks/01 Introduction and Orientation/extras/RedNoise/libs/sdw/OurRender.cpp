#include "OurRender.h"
#include "glm/ext.hpp"

#define WIDTH 320
#define HEIGHT 240

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

std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3> rasterize(DrawingWindow &window, std::vector<ModelTriangle> modelTriangles, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focalLength, float scale, std::vector<std::vector<float>> depthMatrix, bool orbit) {
    window.clearPixels();
    depthMatrix = std::vector<std::vector<float>> (WIDTH, std::vector<float>(HEIGHT, 0.0f));
    std::vector<CanvasTriangle> twodTriangles;

    // conversion and projection onto canvas
    for (ModelTriangle &modelTriangle : modelTriangles) {
//        if (modelTriangle.colour.name == "Red" || modelTriangle.colour.name == "Blue") {
        CanvasTriangle canvasTriangle = modelToCanvasTriangle(modelTriangle);
        for (int i = 0; i < 3; i++) {
            canvasTriangle.vertices[i] = getCanvasIntersectionPoint(canvasTriangle.vertices[i], cameraPosition, cameraOrientation, focalLength, scale);
        }
        twodTriangles.push_back(canvasTriangle);
        depthMatrix = drawFilled(window, canvasTriangle, modelTriangle.colour, depthMatrix);
        drawStroked(window, canvasTriangle, modelTriangle.colour, depthMatrix);
    }

    // ORBIT
    if (orbit) {
        cameraPosition = glm::mat3 (
        cos(0.1), 0.0f, -sin(0.1),
        0.0f, 1.0f, 0.0f,
        sin(0.1), 0.0f, cos(0.1)
        ) * cameraPosition;
        cameraOrientation = LookAt(cameraOrientation, glm::vec3(0,0,0), cameraPosition);
    }
    return std::make_tuple(twodTriangles, cameraPosition, cameraOrientation);
}

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
    float x, y;
    CanvasPoint intersection;

    // CameraPos - VertexPos
    glm::vec3 distance = glm::vec3(vertexPosition.x-cameraPosition.x, vertexPosition.y-cameraPosition.y, vertexPosition.depth-cameraPosition.z);
    //... Then, multiply this vector by orientation matrix
    distance = distance * cameraOrientation;

    // Calculate the 2D coordinates on the image plane
    x = (focalLength/(distance.z)) * (distance.x);
    y = (focalLength/(distance.z)) * (distance.y);

    // Scaling and shifting
    x = x * -scale + (320.0f / 2);
    y = y * scale + (240.0f / 2); // negative scale bc y-axis was flipped

    // Populate and return intersection
    intersection.x = x;
    intersection.y = y;
    intersection.depth = 1/std::abs(distance.z);
    return intersection;
}
