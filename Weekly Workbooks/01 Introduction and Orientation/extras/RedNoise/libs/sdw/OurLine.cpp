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

void drawLine(DrawingWindow &window, float fromX, float fromY, float toX, float toY, Colour color) {
    fromX = std::floor(fromX);
    toX = std::floor(toX);
    float xDiff = toX-fromX;
    float yDiff = toY-fromY;
    float steps = std::max(std::abs(xDiff), std::abs(yDiff));
    float xSteps = xDiff / steps;
    float ySteps = yDiff / steps;

    std::vector<int> colorgb = unpack(color);
    uint32_t fincolor = pack(colorgb);

    for (int i = 0; i < static_cast<int>(std::round(steps)); i++) {
        float x = fromX + (xSteps* static_cast<float>(i));
        float y = fromY + (ySteps* static_cast<float>(i));

        int xval = static_cast<int>(std::round(x));
        int yval = static_cast<int>(std::round(y));
//        if (xval > WIDTH || yval > HEIGHT) {
//            std::cout << "EXCEEDING BOUNDARIES" << std::endl;
//            continue;
//        } else {
        window.setPixelColour(xval, yval, fincolor);
//        }
    }
}

void drawPoint(DrawingWindow &window, CanvasPoint point, Colour color) {
    window.setPixelColour(static_cast<size_t>(point.x), static_cast<size_t>(point.y), pack(unpack(color)));
}