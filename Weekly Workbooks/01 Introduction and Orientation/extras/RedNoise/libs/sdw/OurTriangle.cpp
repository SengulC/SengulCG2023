#include "OurTriangle.h"

std::vector<std::vector<float>> drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color, std::vector<std::vector<float>> depthMatrix) {
    depthMatrix = drawLine(window, triangle.v0(), triangle.v1(), color, depthMatrix);
    depthMatrix = drawLine(window, triangle.v1(),triangle.v2(), color, depthMatrix);
    depthMatrix = drawLine(window, triangle.v0(), triangle.v2(), color, depthMatrix);
    return depthMatrix;
}

// from and to canvaspoints passed to drawline need depth values (z)!!!
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
        depthMatrix = drawLine(window,topStart[i], topEnd[i], color, depthMatrix);
    }

    // bottom triangle
    std::vector<CanvasPoint> bottomStart = interpolateCanvasPoint(extraV, points[2], bottomHeight+1);
    std::vector<CanvasPoint> bottomEnd = interpolateCanvasPoint(middleV, points[2], bottomHeight+1);
    for (int i = 0; i <= bottomHeight; i++) {
        depthMatrix = drawLine(window, bottomStart[i], bottomEnd[i], color, depthMatrix);
    }

    depthMatrix = drawLine(window, CanvasPoint{std::min(extraVx, middleV.x), extraV.y},
                           CanvasPoint{std::max(extraVx, middleV.x), extraV.y}, color, depthMatrix);
    return depthMatrix;
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