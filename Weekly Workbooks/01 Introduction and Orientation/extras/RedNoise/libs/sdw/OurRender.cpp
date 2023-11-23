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
bool validTUV(glm::vec3 tuv) {
    float t=tuv.x; float u=tuv.y; float v=tuv.z;
    bool uTest = (u >= 0.0) && (u <= 1.0);
    bool vTest = (v >= 0.0) && (v <= 1.0);
    bool addTest = (u + v) <= 1.0;
    bool tPos = t >= 0.0;

    if (uTest && vTest && addTest && tPos) {
        return true;
    } else {
        return false;
    }
}

RayTriangleIntersection getClosestValidIntersection(glm::vec3 cameraPosition, glm::vec3 rayDirection, const std::vector<ModelTriangle>& triangles) {
    rayDirection = glm::normalize(rayDirection);
    glm::vec3 e0, e1, SPVector, possibleSolution;
    std::vector<RayTriangleIntersection> possibleSolutions, convertedSolutions1, convertedSolutions2;

    int index = 0;
    // loop through all triangles for given ray direction
    for (ModelTriangle triangle : triangles) {
        e0 = triangle.vertices[1] - triangle.vertices[0];
        e1 = triangle.vertices[2] - triangle.vertices[0];
        SPVector = cameraPosition - triangle.vertices[0];
        glm::mat3 DEMatrix(-rayDirection, e0, e1);
        possibleSolution = glm::inverse(DEMatrix) * SPVector;

        possibleSolutions.emplace_back(possibleSolution, possibleSolution.x, triangle, index);
        index++;
    }

    // std::cout<< possibleSolutions.size() << " possible solutions" <<std::endl;

    glm::vec3 convertedPoint, position;
    RayTriangleIntersection convertedIntersection;
    // loop through the possible solutions and check if they're valid
    // tuv = intersectionPoint.xyz
    for (const RayTriangleIntersection &tuv: possibleSolutions) {
        if (validTUV(tuv.intersectionPoint)) {
            // std::cout<< "valid!" <<std::endl;
            convertedIntersection = tuv; // retain all other data for RayTriangleIntersection struct just overwrite the vec3 w/ conversion
            // conversion #1: r = p0 + u(p1-p0) + v(p2-p0)
            convertedPoint = tuv.intersectedTriangle.vertices[0] + (tuv.intersectionPoint.y * e0) + (tuv.intersectionPoint.z * e1);
            convertedIntersection.intersectionPoint = convertedPoint;
            convertedSolutions1.push_back(convertedIntersection);

            convertedIntersection = tuv;
            // use the direction of the projected ray and distance t (to find the intersection point relative to the camera) or
            // conversion #2: position = cameraPosition + t * rayDirection;
            position = cameraPosition + tuv.intersectionPoint.x * rayDirection;
            convertedIntersection.intersectionPoint = position;
            convertedSolutions2.push_back(convertedIntersection);
        } else {
            continue;
        }
    }

    RayTriangleIntersection closestIntersection, closestIntersection2;
    // check if there ~was~ a valid converted solution
    if (!convertedSolutions1.empty() && !convertedSolutions2.empty()) {
        // lamba func to find smallest t val in convertedSolutions1/2
        auto intersection1 = std::min_element(
                convertedSolutions1.begin(), convertedSolutions1.end(),
                [](const RayTriangleIntersection& a, const RayTriangleIntersection& b) {return abs(a.distanceFromCamera) < abs(b.distanceFromCamera);});

        auto intersection2 = std::min_element(
                convertedSolutions2.begin(), convertedSolutions2.end(),
                [](const RayTriangleIntersection& a, const RayTriangleIntersection& b) {return abs(a.distanceFromCamera) < abs(b.distanceFromCamera);});

        // -> is some wrap-iter thing apparently (source: CLion)
        closestIntersection = RayTriangleIntersection(intersection1->intersectionPoint, intersection1->distanceFromCamera, intersection1->intersectedTriangle, intersection1->triangleIndex);
        closestIntersection2 = RayTriangleIntersection(intersection2->intersectionPoint, intersection2->distanceFromCamera, intersection2->intersectedTriangle, intersection2->triangleIndex);
    } else {
        // return index as -1 for error code
        RayTriangleIntersection erroneous = RayTriangleIntersection(glm::vec3(0, 0, 0), 0, triangles[0], INT_MAX);
        // std::cout<<"No valid intersections"<<std::endl;
        return erroneous;
    }

//    std::cout<< "1: " << closestIntersection << std::endl;
//    std::cout << (closestIntersection.intersectedTriangle.colour) << std::endl;
//    std::cout<< "2: " << closestIntersection2 << std::endl;
//    std::cout << (closestIntersection2.intersectedTriangle.colour) << std::endl;

    return closestIntersection;
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

glm::vec3 convertToDirectionVector(CanvasPoint point, float scale, float focalLength, glm::vec3 cameraPosition, glm::mat3 cameraOrientation) {
    float z = focalLength - cameraPosition.z;
    glm::vec3 direction (((((point.x) - (WIDTH/2)) / (-scale))) / (focalLength/z), ((((point.y) - (HEIGHT/2))/(scale))) / (focalLength/z), -focalLength); // works but whyyy
//    float x = (point.x - (WIDTH/2) / scale) / (focalLength / z) ;
//    float y = (point.y - (HEIGHT/2) / scale) / (focalLength / z);
//
//    glm::vec3 direction(x, y, z);
//    direction = direction * glm::inverse(cameraOrientatioxn);
//    direction += cameraPosition;

    return direction;
}

void drawRaytracedScene(DrawingWindow &window, const std::vector<ModelTriangle>& triangles, float scale, float focalLength, glm::vec3 cameraPosition, glm::mat3 cameraOrientation) {
    /* given an array of triangles
     * loop through sdl window pixels,
     * fire a ray into first pixel, use intersection formula to loop through array of triangles,
     * find closest intersection
     * if valid, set pixel
    */
    std::cout<<"in raytracer"<<std::endl;
    window.clearPixels();
    for (int x=0; x<WIDTH; x++) {
        for (int y=0; y<HEIGHT; y++) {
            CanvasPoint point(x, y, focalLength);
            glm::vec3 rayDirection =  convertToDirectionVector(point, scale, focalLength, cameraPosition, cameraOrientation);
            // std::cout<< "raydir: "<<rayDirection.x<<" "<<rayDirection.y<<" "<<rayDirection.z<<std::endl;
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, rayDirection, triangles);
            if (intersection.triangleIndex!= INT_MAX) {
                // std::cout<<"setting pixel"<<std::endl;
                window.setPixelColour(x, y, pack(unpack(intersection.intersectedTriangle.colour)));
            } else {
                continue;
            }
        }
    }
    std::cout<<"end of ray trace"<<std::endl;
}

