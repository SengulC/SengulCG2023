#include "OurLine.h"

#define WIDTH 320
#define HEIGHT 240

void printMat3(const glm::mat3& matrix) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void printDepthMatrix(const std::vector<std::vector<float>>& depthMatrix) {
    for (const auto& row : depthMatrix) {
        for (float depth : row) {
            std::cout << depth << " ";
        }
        std::cout << std::endl;
    }
}

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

float roundToThreeSF(float num) {
    if (num == 0.0) {
        return 0.0;  // Avoid division by zero
    }

    // Calculate the order of magnitude
    float magnitude = std::pow(10.0, std::floor(std::log10(std::fabs(num))));

    // Extract the first three non-zero digits
    int firstDigit = static_cast<int>(std::fabs(num) / magnitude);
    int secondDigit = static_cast<int>((std::fabs(num) / magnitude * 10) - (firstDigit * 10));
    int thirdDigit = static_cast<int>((std::fabs(num) / magnitude * 100) - (firstDigit * 100) - (secondDigit * 10));

    // Combine the three digits and multiply by the magnitude
    float rounded = (firstDigit * 100 + secondDigit * 10 + thirdDigit) * magnitude / 1000.0;

    // Preserve the sign of the original number
    rounded *= (num < 0) ? -1 : 1;

    return rounded;
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

        /* int temp = std::round(z * 255);
        Colour col = Colour(temp, temp, temp);
        window.setPixelColour(xval, yval, convertColor(col));
         */
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