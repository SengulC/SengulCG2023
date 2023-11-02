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
    CanvasPoint extraV = {extraVx, middleV.y};

    // depth
    int debug = true;
    std::vector<float> leftDepths = interpolateSingleFloats(points[0].depth, points[2].depth, topHeight + bottomHeight);
    std::vector<float> rightDepths = interpolateSingleFloats(points[0].depth, points[1].depth, topHeight + bottomHeight);

//    if (debug && (color.name=="Red" || color.name=="Green")) {
//        std::cout << "left depths for color: " << color.name;
//        std::cout << "interpolating from: " << points[0].depth << " to: " << points[2].depth << std::endl;
//        for (float &depth: leftDepths) {
//            std::cout << depth << " ";
//        }
//        std::cout << "\n" << std::endl;
//
//        std::cout << "right depths for color: " << color.name;
//        std::cout << "interpolating from: " << points[0].depth << " to: " << points[1].depth << std::endl;
//        for (float &depth: rightDepths) {
//            std::cout << depth << " ";
//        }
//        std::cout << "\n" << std::endl;
//
//        debug = false;
//    }

    // top triangle
    int count = 0;
    for (int i = static_cast<int> (std::round(points[0].y)); i < static_cast<int> (std::round(extraV.y)); i++) {
        std::vector<float> topTriangleLeft = interpolateSingleFloats(points[0].x, extraV.x, topHeight);
        std::vector<float> topTriangleRight = interpolateSingleFloats(points[0].x, middleV.x, topHeight);
//        std::cout<<leftDepths[count]<<std::endl;
        depthMatrix = drawLine(window, CanvasPoint{topTriangleLeft[count], static_cast<float>(i), leftDepths[count]},
                 CanvasPoint{topTriangleRight[count], static_cast<float>(i),  rightDepths[count]},
                 color, depthMatrix);
        count++;
    }

    // bottom triangle
    count = 0;
    for (int i = static_cast<int> (std::round(middleV.y + 1)); i <= static_cast<int> (std::round(points[2].y)); i++) {
        std::vector<float> bottomTriangleLeft = interpolateSingleFloats(extraV.x + 1, points[2].x, bottomHeight);
        std::vector<float> bottomTriangleRight = interpolateSingleFloats(middleV.x, points[2].x, bottomHeight);
//        std::cout<<rightDepths[count]<<std::endl;
        depthMatrix = drawLine(window, CanvasPoint{bottomTriangleLeft[count], static_cast<float>(i), leftDepths[count]},
                 CanvasPoint{bottomTriangleRight[count], static_cast<float>(i), rightDepths[count]},
                 color, depthMatrix);
        count++;
    }
    depthMatrix = drawLine(window, CanvasPoint{std::min(extraVx, middleV.x), extraV.y},CanvasPoint{std::max(extraVx, middleV.x), extraV.y}, color, depthMatrix);
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