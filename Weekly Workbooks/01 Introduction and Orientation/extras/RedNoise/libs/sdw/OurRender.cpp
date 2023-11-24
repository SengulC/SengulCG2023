#include "OurRender.h"

#define WIDTH 320
#define HEIGHT 240

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
    depthMatrix = std::vector<std::vector<float>> (WIDTH, std::vector<float>(HEIGHT, 0.0f));
    std::vector<CanvasTriangle> twodTriangles;

    // conversion and projection onto canvas
    for (ModelTriangle &modelTriangle : modelTriangles) {
//        if (modelTriangle.colour.name == "Cyan" || modelTriangle.colour.name == "White") {
        CanvasTriangle canvasTriangle = modelToCanvasTriangle(modelTriangle);
        for (int i = 0; i < 3; i++) {
            canvasTriangle.vertices[i] = getCanvasIntersectionPoint(canvasTriangle.vertices[i], cameraPosition,
                                                                    cameraOrientation, focalLength, scale);
        }
        twodTriangles.push_back(canvasTriangle);
        depthMatrix = drawFilled(window, canvasTriangle, modelTriangle.colour, depthMatrix);
        drawStroked(window, canvasTriangle, modelTriangle.colour, depthMatrix);
//        }
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
    return std::make_tuple(twodTriangles, cameraPosition, cameraOrientation, depthMatrix);
}

// ray-traced render
bool validTUV(const RayTriangleIntersection& intersect, float dist, bool shadow, size_t currIndex) {
    glm::vec3 tuv = intersect.intersectionPoint;
    float t=tuv.x; float u=tuv.y; float v=tuv.z;
    bool uTest = (u >= 0.0) && (u <= 1.0);
    bool vTest = (v >= 0.0) && (v <= 1.0);
    bool addTest = (u + v) <= 1.0;
    bool tPos = t >= 0.0;
    bool sameTri = (intersect.triangleIndex != currIndex);
    bool shadowT;

    if (shadow) {shadowT = abs(t) < dist;}
    else { shadowT = true; }

    return (uTest && vTest && addTest && tPos && shadowT && sameTri);
}

RayTriangleIntersection getClosestValidIntersection(glm::vec3 startPosition, glm::vec3 endPosition, glm::vec3 rayDirection, const std::vector<ModelTriangle>& triangles, bool shadow, size_t currIndex) {
    rayDirection = glm::normalize(rayDirection);
    glm::vec3 e0, e1, SPVector, possibleSolution;
    std::vector<RayTriangleIntersection> possibleSolutions, convertedSolutions1, convertedSolutions2;

    size_t index = 0;
    // loop through all triangles for given ray direction
    for (ModelTriangle triangle : triangles) {
        // don't compare the same triangle to itself for intersection!
//        if (index != currIndex) {
            e0 = triangle.vertices[1] - triangle.vertices[0];
            e1 = triangle.vertices[2] - triangle.vertices[0];
            SPVector = startPosition - triangle.vertices[0];
            glm::mat3 DEMatrix(-rayDirection, e0, e1);
            possibleSolution = glm::inverse(DEMatrix) * SPVector;

            possibleSolutions.emplace_back(possibleSolution, glm::distance(startPosition, endPosition),
                                           possibleSolution.x, triangle, index, true);
            index++;
//        }
    }

    glm::vec3 convertedPoint, position;
    RayTriangleIntersection convertedIntersection;
    // loop through the possible solutions and check if they're valid
    // tuv = intersectionPoint.xyz
    for (const RayTriangleIntersection &tuv: possibleSolutions) {
        if (validTUV(tuv, glm::distance(endPosition, startPosition), shadow, currIndex)) {
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
        closestIntersection = RayTriangleIntersection(intersection1->intersectionPoint, intersection1->distanceFromStart, intersection1->t, intersection1->intersectedTriangle, intersection1->triangleIndex, intersection1->valid);
        closestIntersection2 = RayTriangleIntersection(intersection2->intersectionPoint, intersection2->distanceFromStart, intersection1->t, intersection2->intersectedTriangle, intersection2->triangleIndex, intersection2->valid);
    } else {
        // return error codes within RayTriangleIntersection (primarily valid: false)
        RayTriangleIntersection erroneous = RayTriangleIntersection(glm::vec3(0, 0, 0), 1000, 1000, triangles[0], INT_MAX, false);
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
    // direction += cameraPosition;

    return direction;
}

void drawRaytracedScene(DrawingWindow &window, const std::vector<ModelTriangle>& triangles, float scale, float focalLength, glm::vec3 cameraPosition, glm::mat3 cameraOrientation) {
    std::cout <<"in raytracer"<< std::endl;
    window.clearPixels();
    glm::vec3 lightPosition (0,0.9,0);
    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            CanvasPoint point(x, y, focalLength);
            glm::vec3 rayDirection =  convertToDirectionVector(point, scale, focalLength, cameraPosition, cameraOrientation);
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, glm::vec3(x,y,focalLength), rayDirection, triangles, false, 10000);
            if (intersection.valid) {
                // intersection.intersectionPoint = glm::vec3(intersection.intersectionPoint.x+0.001f, intersection.intersectionPoint.y+0.001f, intersection.intersectionPoint.z+0.001f);
                glm::vec3 shadowRay = glm::normalize(lightPosition-(intersection.intersectionPoint));
                RayTriangleIntersection closestObjIntersection = getClosestValidIntersection((intersection.intersectionPoint), lightPosition, shadowRay, triangles, true, intersection.triangleIndex);
                if (closestObjIntersection.valid &&
                    glm::distance(closestObjIntersection.intersectionPoint, intersection.intersectionPoint) >= 0.0001) {
/*                     IF THE SURFACE HAS SOME INTERSECTION.. THAT IS NOT THE LIGHT. SET SHADOW
                    if (intersection.intersectedTriangle.colour.name=="Cyan" || intersection.intersectedTriangle.colour.name=="Yellow") {
                    std::cout<< "me: " << intersection.intersectedTriangle.colour << ". index: " << intersection.triangleIndex <<std::endl;
                    std::cout<< "my intersection: " << closestObjIntersection.intersectedTriangle.colour << ". index: " << closestObjIntersection.triangleIndex << std::endl;
                    }*/
                    uint32_t shadow = pack(unpack(Colour(90,90,90)));
                    window.setPixelColour(x, y, shadow);
                } else {
                    window.setPixelColour(x, y, pack(unpack(intersection.intersectedTriangle.colour)));
                }
            }
        }
    }
    std::cout<<"end of ray trace"<<std::endl;
}

