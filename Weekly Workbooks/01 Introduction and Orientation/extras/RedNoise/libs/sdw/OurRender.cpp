#include "OurRender.h"
#include "OurTriangle.h"
#include "OurObject.h"

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

std::vector<CanvasTriangle> rasterize(DrawingWindow &window, std::vector<ModelTriangle> modelTriangles, glm::vec3 cameraPosition, float focalLength, float scale, std::vector<std::vector<int>> depthMatrix) {
    // draw wireframe
    std::vector<CanvasTriangle> twodTriangles;

    for (ModelTriangle &modelTriangle : modelTriangles) {
        CanvasTriangle canvasTriangle;
        for (int i = 0; i < 3; i++) {
            // populate each vertex - 0, 1, 2
            canvasTriangle.vertices[i] = getCanvasIntersectionPoint(modelTriangle.vertices[i], cameraPosition, focalLength, scale);
        }

//        std::sort(canvasTriangle.vertices.begin(), canvasTriangle.vertices.end(), sortByY); // sorted in ascending order of Ys
//        std::vector<float> leftDepths = interpolateSingleFloats(canvasTriangle.v0().depth, canvasTriangle.v2().depth, canvasTriangle.v2().y - canvasTriangle.v0().y);
//        std::vector<float> rightDepths = interpolateSingleFloats(canvasTriangle.v0().depth, canvasTriangle.v1().depth, canvasTriangle.v1().y - canvasTriangle.v0().y);

        twodTriangles.push_back(canvasTriangle);
        drawFilled(window, canvasTriangle, modelTriangle.colour, depthMatrix);
        drawStroked(window, canvasTriangle, {255,255,255}, depthMatrix);

//        // for the curr. Canvas triangle's area populate the depth matrix via canvasTriangle vertices
//        for (CanvasPoint &vertex : canvasTriangle.vertices) {
//            // if the currently stored pixel @ [x][y] is < than 1/z (init. w/ 0), overwrite:
//            if (depthMatrix[vertex.x][vertex.y] < 1/(vertex.depth)) {
//                depthMatrix[vertex.x][vertex.y] = 1/(vertex.depth);
//            }
//        }


    }
    return twodTriangles;
}