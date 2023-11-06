#include "OurLine.h"

#define WIDTH 320
#define HEIGHT 240

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
/*
from(1.0, 4.0, 9.2);
to(4.0, 1.0, 9.8);

(1.0, 4.0, 9.2)
(2.0, 3.0, 9.4)
(3.0, 2.0, 9.6)
(4.0, 1.0, 9.8)'

 result = vector of vec3s
 1.0-4.0: result[0-3][0]
 */
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
    // best rounding for entire function is just std::round.
    from.x = std::round(from.x);
    to.x = std::round(to.x);
//    from.y = std::round(from.y);
//    to.y = std::round(to.y);
    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float zDiff = to.depth - from.depth;
    float steps = std::max(std::abs(xDiff), std::abs(yDiff));
    float xSteps = xDiff / steps;
    float ySteps = yDiff / steps;

    std::vector<int> colorgb = unpack(color);
    uint32_t fincolor = pack(colorgb);
// Define an output file stream
    std::ofstream outputFile("output.txt", std::ios::app); // Open the file in append mode

// Check if the file was opened successfully
    if (outputFile.is_open()) {
        for (int i = 0; i < static_cast<int>(std::round(steps)); i++) {
            float x = from.x + (xSteps * static_cast<float>(i));
            float y = from.y + (ySteps * static_cast<float>(i));

            int magnitude = static_cast<int>(sqrt(xDiff * xDiff + yDiff * yDiff + zDiff * zDiff));
            std::vector<float> depths = interpolateSingleFloats((from.depth), (to.depth), magnitude);

            float z = (depths[i]);
            int xval = static_cast<int>(std::round(x));
            int yval = static_cast<int>(std::round(y));

            if (depthMatrix[xval][yval] == 0.0f) {
                depthMatrix[xval][yval] = z;
                window.setPixelColour(xval, yval, fincolor);
                //std::cout<< "initial: matrix[x][y] = " << depthMatrix[xval][yval] << " . z: " << z <<std::endl;
                //std::cout<< "rounded: " << std::roundf(depthMatrix[xval][yval]) <<std::endl;
            } else if (z >= depthMatrix[xval][yval]) {
                //std::cout<< "z > depthmatrix[x][y]: z: " << z << ". depth x: " << depthMatrix[xval][yval] << ". overwriting color: " << colorName(fincolor) <<std::endl;
                depthMatrix[xval][yval] = z;
                window.setPixelColour(xval, yval, fincolor);
            }
            else {
                if (colorName(fincolor) == "blue" || colorName(fincolor) == "red") {
//                    std::cout<< "z < depthmatrix[x][y]: z: " << z << ". depth x: " << depthMatrix[xval][yval] << ". current color: " << colorName(fincolor) <<std::endl;
                }
                outputFile << "else: z: " << z << ". depth x: " << depthMatrix[xval][yval] << ". color: "
                           << colorName(fincolor) << std::endl;
            }
        int temp = std::round(z * 255);
        Colour col = Colour(temp, temp, temp);
//        window.setPixelColour(xval, yval, pack(unpack(col)));
        outputFile.close();
        }
    } else {
        std::cerr << "Error opening the file for writing." << std::endl;
    }
    return depthMatrix;
}


void drawPoint(DrawingWindow &window, CanvasPoint point, Colour color) {
    window.setPixelColour(static_cast<size_t>(point.x), static_cast<size_t>(point.y), pack(unpack(color)));
}