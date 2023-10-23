#include "OurTriangle.h"

void drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    drawLine(window, triangle.v0(), triangle.v1(), color);
    drawLine(window, triangle.v1(),triangle.v2(), color);
    drawLine(window, triangle.v0(), triangle.v2(), color);
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
        std::vector<float> leftLine = interpolateSingleFloats(points[0].x, left, height+1);
        std::vector<float> rightLine = interpolateSingleFloats(points[0].x, right, height+1);

//        std::cout<< "left vertex, right vertex, and interpolation"  << left << " " << right << " " << leftLine[leftLine.size()-1] << " " << rightLine[rightLine.size()-1] <<std::endl;

        int count = 0;
        for (int i=points[0].y; i<points[2].y; i++) {
            drawLine(window, CanvasPoint{leftLine[count], static_cast<float>(i)}, CanvasPoint{rightLine[count], static_cast<float>(i)}, color);
            count++;
        }

    // flat-topped triangle
    } else if (points[0].y == points[1].y) {
        int height = static_cast<int> (points[2].y - points[0].y);
        if (points[0].x > points[1].x) {
            left = points[1].x;
            right = points[0].x;
        } else {
            left = points[0].x;
            right = points[1].x;
        }
        std::vector<float> leftLine = interpolateSingleFloats(left, points[2].x, height+1);
        std::vector<float> rightLine = interpolateSingleFloats(right, points[2].x, height+1);

        int count = 0;
        for (int i=points[0].y; i<points[2].y; i++) {
            drawLine(window, CanvasPoint{(leftLine[count]), static_cast<float>(i)}, CanvasPoint{(rightLine[count]), static_cast<float>(i)}, color);
            count++;
        }
    }

    else {
        // static_cast<float>(s)plit triangle istatic_cast<float>(s)tatic_cast<float>(n)to 2 from middle vertex
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
            drawLine(window, CanvasPoint{topTriangleLeft[count], static_cast<float>(i)}, CanvasPoint{topTriangleRight[count], static_cast<float>(i)}, color);
            count++;
        }

        count = 0;
        for (int i = static_cast<int> (std::round(middleV.y + 1));
             i <= static_cast<int> (std::round(points[2].y)); i++) {
            std::vector<float> bottomTriangleLeft = interpolateSingleFloats(extraV.x + 1, points[2].x, bottomHeight);
            std::vector<float> bottomTriangleRight = interpolateSingleFloats(middleV.x, points[2].x, bottomHeight);
            drawLine(window, CanvasPoint{bottomTriangleLeft[count], static_cast<float>(i)}, CanvasPoint{bottomTriangleRight[count], static_cast<float>(i)}, color);
            count++;
        }
        drawLine(window, CanvasPoint{extraV.x, extraV.y}, CanvasPoint{middleV.x, middleV.y}, color);
    }
    drawStroked(window, triangle, color);
}

/*
void drawFlatBottomTriangle(DrawingWindow &window, std::vector<CanvasPoint> points, Colour color) {
    float left;
    float right;
    if (points[1].y == points[2].y) {
        int height = static_cast<int> (points[2].y - points[0].y);
        if (points[1].x > points[2].x) {
            left = points[2].x;
            right = points[1].x;
        } else {
            left = points[1].x;
            right = points[2].x;
        }
        std::vector<float> leftLine = interpolateSingleFloats(points[0].x, left, height + 1);
        std::vector<float> rightLine = interpolateSingleFloats(points[0].x, right, height + 1);

        int count = 0;
        for (int i = points[0].y; i < points[2].y; i++) {
            if ((leftLine[count]) == 0 || (leftLine[count]) >= 320) {
                continue;
            } else {
                drawLine(window, CanvasPoint{(leftLine[count]), i}, CanvasPoint{(rightLine[count]), i}, color);
            }

            count++;
        }
    }
}
void drawFlatTopTriangle(DrawingWindow &window, std::vector<CanvasPoint> points, Colour color) {
    // flat-topped triangle
    float left; float right;
    int height = static_cast<int> (points[2].y - points[0].y);
    if (points[0].x > points[1].x) {
        left = points[1].x;
        right = points[0].x;
    } else {
        left = points[0].x;
        right = points[1].x;
    }
    std::vector<float> leftLine = interpolateSingleFloats(left, points[2].x, height);
    std::vector<float> rightLine = interpolateSingleFloats(right, points[2].x, height);

    int count = 0;
    for (int i=points[0].y; i<points[2].y; i++) {
        drawLine(window, (leftLine[count]), i, (rightLine[count]), i, color);
        count++;
    }
}
void drawFilled(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    std::cout<<"FLAT BOTTOM"<<std::endl;
    std::vector<CanvasPoint> points = {triangle.v0(), triangle.v1(), triangle.v2()};
    std::sort(points.begin(), points.end(), sortByY);

    if (points[0].y == points[1].y) {
        drawFlatTopTriangle(window, points, color);
        drawStroked(window, triangle, {255,255,255});
    } else if (points[1].y == points[2].y) {
        drawFlatBottomTriangle(window, points, color);
        drawStroked(window, triangle, {255,255,255});
    } else {

        CanvasPoint middleV = points[1];
        int topHeight = static_cast<int> (std::abs(middleV.y - points[0].y));
        int bottomHeight = static_cast<int> (std::abs(points[2].y - middleV.y));
        std::vector<float> extraVInterpolatedX = interpolateSingleFloats(points[0].x, points[2].x,
                                                                         topHeight + bottomHeight);
        float extraVx = extraVInterpolatedX[topHeight];
        CanvasPoint extraV = {extraVx, middleV.y};
        std::cout<<"ELSE"<<std::endl;
        // Split the non-flat triangle into a flat-top and flat-bottom triangle
//        CanvasPoint extraPoint;
//        extraPoint.y = points[1].y;
        float horizontal = points[2].x - points[0].x;
        float vertical = points[2].y - points[0].y;
        float halfVertical = points[1].y - points[0].y;
        extraV.x = points[0].x + (horizontal / vertical) * halfVertical;
        drawFlatTopTriangle(window, {points[0], points[1], extraV}, color);
        drawFlatBottomTriangle(window, {points[1], extraV, points[2]}, color);
        drawStroked(window, triangle, {255,255,255});
    }
}
*/

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