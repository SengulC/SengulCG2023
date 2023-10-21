#include "OurTriangle.h"

void drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    drawLine(window, triangle.v0().x, triangle.v0().y, triangle.v1().x, triangle.v1().y, color);
    drawLine(window, triangle.v1().x, triangle.v1().y, triangle.v2().x, triangle.v2().y, color);
    drawLine(window, triangle.v0().x, triangle.v0().y, triangle.v2().x, triangle.v2().y, color);
}

void drawFilled(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    std::vector<CanvasPoint> points = {triangle.v0(), triangle.v1(), triangle.v2()};
    std::sort(points.begin(), points.end(), sortByY); // sorted in ascending order of Ys: 0,1,2...HEIGHT-1
    float left; float right;

    // flat-bottomed triangle
    if (points[1].y == points[2].y) {
        int height = static_cast<int> (points[2].y - points[0].y);
        if (points[1].x > points[2].x) {
            left = points[2].x;
            right = points[1].x;
        } else {
            left = points[1].x;
            right = points[2].x;
        }
        std::vector<float> leftLine = interpolateSingleFloats(points[0].x, left, height);
        std::vector<float> rightLine = interpolateSingleFloats(points[0].x, right, height);

        int count = 0;
        for (int i=points[0].y; i<points[2].y; i++) {
            drawLine(window, leftLine[count], i, rightLine[count], i, color);
            count++;
        }
    }

    else {
        // split triangle into 2 from middle vertex
        CanvasPoint middleV = points[1];
        int topHeight = static_cast<int> (std::abs(middleV.y - points[0].y));
        int bottomHeight = static_cast<int> (std::abs(points[2].y - middleV.y));

        // interpolate from 1st point to last point to find extra vertex
        std::vector<float> extraVInterpolatedX = interpolateSingleFloats(points[0].x, points[2].x,
                                                                         topHeight + bottomHeight);
        if (extraVInterpolatedX[topHeight] == 0) {
            std::cout << "IT IS ZERO" << std::endl;
        }

        float extraVx = extraVInterpolatedX[topHeight];
        CanvasPoint extraV = {extraVx, middleV.y};

        int count = 0;
        for (int i = static_cast<int> (std::round(points[0].y)); i < static_cast<int> (std::round(extraV.y)); i++) {
            std::vector<float> topTriangleLeft = interpolateSingleFloats(points[0].x, extraV.x, topHeight);
            std::vector<float> topTriangleRight = interpolateSingleFloats(points[0].x, middleV.x, topHeight);
            drawLine(window, topTriangleLeft[count], i, topTriangleRight[count], i, color);
            count++;
        }

        count = 0;
        for (int i = static_cast<int> (std::round(middleV.y + 1));
             i <= static_cast<int> (std::round(points[2].y)); i++) {
            std::vector<float> bottomTriangleLeft = interpolateSingleFloats(extraV.x + 1, points[2].x, bottomHeight);
            std::vector<float> bottomTriangleRight = interpolateSingleFloats(middleV.x, points[2].x, bottomHeight);
            drawLine(window, bottomTriangleLeft[count], i, bottomTriangleRight[count], i, color);
            count++;
        }

        drawLine(window, extraV.x, extraV.y, middleV.x, middleV.y, color);
    }
    drawStroked(window, triangle, {255, 255, 255});
}

CanvasTriangle randomTriangle() {
    CanvasPoint v0 = {static_cast<float>(rand()%320),static_cast<float>(rand()%240),0,0};
    CanvasPoint v1 = {static_cast<float>(rand()%320),static_cast<float>(rand()%240),0,0};
    CanvasPoint v2 = {static_cast<float>(rand()%320),static_cast<float>(rand()%240),0,0};
    CanvasTriangle triangle = {v0,v1,v2};
    return triangle;
}

Colour randomColor() {
    Colour color = {rand()%256, rand()%256, rand()%256};
    return color;
}