#include "OurLine.h"

#define WIDTH 320
#define HEIGHT 240

std::vector<int> unpack(const Colour& color) {
    std::vector<int> colour = {color.red, color.green, color.blue};
    return colour;
}
uint32_t pack(std::vector<int> colorgb) {
    return (255 << 24) + (int(colorgb[0]) << 16) + (int(colorgb[1]) << 8) + int(colorgb[2]);
}

bool sortByY(const CanvasPoint& a, const CanvasPoint& b) {
    return a.y < b.y;
}

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
//    numberOfValues = numberOfValues+1;
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

std::vector<std::vector<float>> drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color, std::vector<std::vector<float>> depthMatrix) {
    from.x = std::round(from.x);
    to.x = std::round(to.x);
    float xDiff = to.x-from.x;
    float yDiff = to.y-from.y;
    float zDiff = to.depth-from.depth;
    float steps = std::max(std::abs(xDiff), std::abs(yDiff));
    float xSteps = xDiff / steps;
    float ySteps = yDiff / steps;

    std::vector<int> colorgb = unpack(color);
    uint32_t fincolor = pack(colorgb);

    for (int i = 0; i < static_cast<int>(std::round(steps)); i++) {
        float x = from.x + (xSteps* static_cast<float>(i));
        float y = from.y + (ySteps* static_cast<float>(i));

        int magnitude = static_cast<int>(sqrt(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff));
        std::vector<float> depths = interpolateSingleFloats(from.depth, to.depth, magnitude);
        // numberOfValues in interpolation = magnitude ??

        float z = (depths[i]);
        int xval = static_cast<int>(std::round(x));
        int yval = static_cast<int>(std::round(y));
//        if (fincolor == 4278255360 || fincolor == 4294901760) {
            if (depthMatrix[xval][yval] == 0 || z > std::round(depthMatrix[xval][yval])) {
                depthMatrix[xval][yval] = std::round(z);
                window.setPixelColour(xval, yval, fincolor);
            } else if (fincolor == 4294901760) {
                std::cout << "curr mtx[x][y] z depth is: " << depthMatrix[xval][yval] << std::endl;
                std::cout << "curr pixel is of triangle : " << color.name << ". z :" << z << std::endl;
            }
//        }
    }
    return depthMatrix;
}

void drawPoint(DrawingWindow &window, CanvasPoint point, Colour color) {
    window.setPixelColour(static_cast<size_t>(point.x), static_cast<size_t>(point.y), pack(unpack(color)));
}