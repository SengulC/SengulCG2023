#include "OurTriangle.h"

std::vector<std::vector<float>> drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color, std::vector<std::vector<float>> depthMatrix) {
    depthMatrix = drawLine(window, triangle.v0(), triangle.v1(), color, depthMatrix, true);
    depthMatrix = drawLine(window, triangle.v1(),triangle.v2(), color, depthMatrix, true);
    depthMatrix = drawLine(window, triangle.v0(), triangle.v2(), color, depthMatrix, true);
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
        depthMatrix = drawLine(window,topStart[i], topEnd[i], color, depthMatrix, true);
    }

    // bottom triangle
    std::vector<CanvasPoint> bottomStart = interpolateCanvasPoint(extraV, points[2], bottomHeight+1);
    std::vector<CanvasPoint> bottomEnd = interpolateCanvasPoint(middleV, points[2], bottomHeight+1);
    for (int i = 0; i <= bottomHeight; i++) {
        depthMatrix = drawLine(window, bottomStart[i], bottomEnd[i], color, depthMatrix, true);
    }

    return depthMatrix;
}

std::array<CanvasPoint, 3> sortAndLinkTextures(std::array<CanvasPoint, 3> vertices, std::vector<TexturePoint> texturePoints) {

    // sort
    std::sort(vertices.begin(), vertices.end(), sortByY);
    std::sort(texturePoints.begin(), texturePoints.end(), [](const TexturePoint &a, const TexturePoint &b) {
        return a.y < b.y;
    });

    // link
    for (int i=0; i <3; i++) {
        vertices[i].texturePoint = texturePoints[i];
    }

    return vertices;
}

//void drawTextureFilled(DrawingWindow &window, CanvasTriangle triangle, const TextureMap& textureMap, const std::vector<TexturePoint>& texturePoints) {
//    /*
//     * Each TextureMap object has a publicly accessible pixels attribute that holds all of the pixel data loaded in from the PPM file.
//     * This pixels attribute is a single dimension vector of pixel-packed RGB integers.
//     * Note that there is no representation of x or y positioning within the data.
//     * The pixel data is just stored linearly: pixels from one row flow directly into those from the next row
//     * The vector contains pure RGB data - there is no concept of a "newline" or "end of row" marker.
//     * You will need to work out the positioning of pixels yourself.
//     * There are however publicly accessible width and height attributes of the TextureMap class
//     * that you can use to find out the dimensions of the original image.
//     */
//
//    // given triangle vertices, giving rectangular texturemap, interpolate and set colors with texturePoint
//    std::vector<std::vector<uint32_t>> textureMatrix = linearListTo2DMatrix(textureMap.pixels, textureMap.height, textureMap.width);
//    // now we have texturemap as a 2d matrix for easier access.
//    // draw triangle via interpolation stuff
//    // set texture pixels aka the colors via interpolation of the 3 texture points
//
//
//    // create canvas points and sort in terms of linked canvas points and texture points
//    std::array<CanvasPoint, 3> points = sortAndLinkTextures(triangle.vertices, texturePoints);
//
////    std::cout<<"printing points"<<std::endl;
////    for (CanvasPoint p : points) {
////        std::cout << p << std::endl;
////        std::cout << p.texturePoint << std::endl;
////    }
//
//
////     APPLY TEXTURE STUFF
////     split triangle into 2 from middle vertex
//    CanvasPoint middleV = points[1];
//    int topHeight = static_cast<int> (std::abs(middleV.y - points[0].y));
//    int bottomHeight = static_cast<int> (std::abs(points[2].y - middleV.y));
//
//    // extra vertex
//    std::vector<float> extraVInterpolatedX = interpolateSingleFloats(points[0].x, points[2].x,topHeight + bottomHeight);
//    float extraVx = extraVInterpolatedX[topHeight];
//    float ratio = (points[1].y - points[0].y)/(points[2].y-points[0].y);
//    float extraVz = ratio * (points[2].depth - points[0].depth) + points[0].depth;
//    CanvasPoint extraV = {extraVx, middleV.y, extraVz};
//
//    Colour color; bool depth = false;
//    std::vector<std::vector<float>> fakeDepthMatrix;
//
//    // top triangle
//    std::vector<CanvasPoint> topStartVertices = interpolateCanvasPoint(points[0], extraV, topHeight+1);
//    std::vector<CanvasPoint> topEndVertices = interpolateCanvasPoint(points[0], middleV, topHeight+1);
//
//    std::vector<CanvasPoint> topStartTextures = interpolateTexturePoint(points[0].texturePoint, extraV.texturePoint, topHeight+1);
//    std::vector<CanvasPoint> topEndTextures = interpolateTexturePoint(points[0].texturePoint, middleV.texturePoint, topHeight+1);
//    for (int i = 0; i <= topHeight; i++) {
//        color = textureMap[];
//        fakeDepthMatrix = drawLine(window, topStartVertices[i], topEndVertices[i], color, fakeDepthMatrix, depth);
//    }
//
//    // bottom triangle
//    std::vector<CanvasPoint> bottomStartVertices = interpolateCanvasPoint(extraV, points[2], bottomHeight+1);
//    std::vector<CanvasPoint> bottomEndVertices = interpolateCanvasPoint(middleV, points[2], bottomHeight+1);
//    for (int i = 0; i <= bottomHeight; i++) {
////        color =;
//        fakeDepthMatrix = drawLine(window, bottomStartVertices[i], bottomEndVertices[i], color, fakeDepthMatrix, depth);
//    }
//
//}

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