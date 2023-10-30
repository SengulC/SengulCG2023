#include "OurTriangle.h"

void drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    drawLine(window, triangle.v0(), triangle.v1(), color);
    drawLine(window, triangle.v1(),triangle.v2(), color);
    drawLine(window, triangle.v0(), triangle.v2(), color);
}

void drawFilled(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    std::vector<CanvasPoint> points = {triangle.v0(), triangle.v1(), triangle.v2()};
    std::sort(points.begin(), points.end(), sortByY); // sorted in ascending order of Ys
    // split triangle into 2 from middle vertex
    CanvasPoint middleV = points[1];
    int topHeight = static_cast<int> (std::abs(middleV.y - points[0].y));
    int bottomHeight = static_cast<int> (std::abs(points[2].y - middleV.y));
    // interpolate from 1st point to last point to find extra vertex
    std::vector<float> extraVInterpolatedX = interpolateSingleFloats(points[0].x, points[2].x,
                                                                     topHeight + bottomHeight);
    // std::cout << "extra v size:" << extraVInterpolatedX.size() << std::endl;
    float extraVx = extraVInterpolatedX[topHeight];
    CanvasPoint extraV = {extraVx, middleV.y};
    // define triangles for verification
    // CanvasTriangle topTriangle = {points[0], middleV, extraV};
    // CanvasTriangle bottomTriangle = {points[2], middleV, extraV};
    int count = 0;
    for (int i = static_cast<int> (std::round(points[0].y)); i < static_cast<int> (std::round(extraV.y)); i++) {
        std::vector<float> topTriangleLeft = interpolateSingleFloats(points[0].x, extraV.x, topHeight);
        std::vector<float> topTriangleRight = interpolateSingleFloats(points[0].x, middleV.x, topHeight);
        drawLine(window, CanvasPoint{topTriangleLeft[count], static_cast<float>(i)},
                 CanvasPoint{topTriangleRight[count], static_cast<float>(i)}, color);
        count++;
    }
    count = 0;
    for (int i = static_cast<int> (std::round(middleV.y + 1)); i <= static_cast<int> (std::round(points[2].y)); i++) {
        std::vector<float> bottomTriangleLeft = interpolateSingleFloats(extraV.x + 1, points[2].x, bottomHeight);
        std::vector<float> bottomTriangleRight = interpolateSingleFloats(middleV.x, points[2].x, bottomHeight);
        drawLine(window, CanvasPoint{bottomTriangleLeft[count], static_cast<float>(i)},
                 CanvasPoint{bottomTriangleRight[count], static_cast<float>(i)}, color);
        count++;
    }
    drawLine(window, CanvasPoint{std::min(extraVx, middleV.x), extraV.y},CanvasPoint{std::max(extraVx, middleV.x), extraV.y}, color);
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