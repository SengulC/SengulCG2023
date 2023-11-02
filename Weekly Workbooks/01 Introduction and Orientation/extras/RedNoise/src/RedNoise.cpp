#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <ModelTriangle.h>
#include <iostream>
#include <string>
#include <map>
#include <OurTriangle.h>
#include <OurObject.h>
#include <OurRender.h>

#define WIDTH 320
#define HEIGHT 240
std::vector<CanvasTriangle> twodTriangles;
int indexcheck;
glm::vec3 cameraPosition {0.0, 0.0, 4.0};
float focalLength = 1.5;
float scale = 240.0f;
bool toggle = true;
std::vector<std::vector<float>> depthMatrix(WIDTH, std::vector<float>(HEIGHT, 0.0f));

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == 'u') {
			drawStroked(window, randomTriangle(), randomColor(), depthMatrix);
		}
		else if (event.key.keysym.sym == 'f') {
            drawFilled(window, randomTriangle(), randomColor(), depthMatrix);
        }
        else if (event.key.keysym.sym == 'w') {
            // draw wireframe
            for (const CanvasTriangle& tri : twodTriangles) {
                drawStroked(window, tri, {255,255,255}, depthMatrix);
            }
        }
        else if (event.key.keysym.sym == 't') {
            // toggle wireframe atop render
            window.clearPixels();
            std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
            std::vector<ModelTriangle> modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35);
            twodTriangles = rasterize(window, modelTriangles, cameraPosition, focalLength, scale, depthMatrix);
            indexcheck=0;
            if (toggle){
                toggle = false;
                for (const CanvasTriangle &tri: twodTriangles) {
                    drawStroked(window, tri, {255, 255, 255}, depthMatrix);
                }
            } else {
                toggle = true;
                window.clearPixels();
                twodTriangles = rasterize(window, modelTriangles, cameraPosition, focalLength, scale, depthMatrix);
                indexcheck=0;
            }
        }
        else if (event.key.keysym.sym == 'x') {
            // draw triangles one by one
            if (indexcheck == 0) {
                window.clearPixels();
            }
            if (indexcheck == twodTriangles.size()) {
                indexcheck = 0;
                window.clearPixels();
            }
            drawPoint(window, CanvasPoint(WIDTH/2, HEIGHT/2), {255,255,255});
            depthMatrix = drawFilled(window, twodTriangles[indexcheck], randomColor(), depthMatrix);
            indexcheck++;
        }
        else if (event.key.keysym.sym == 'c') {
            window.clearPixels();
        }
        else if (event.key.keysym.sym == 'r') {
            window.clearPixels();
            std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
            std::vector<ModelTriangle> modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35);
            twodTriangles = rasterize(window, modelTriangles, cameraPosition, focalLength, scale, depthMatrix);
            indexcheck=0;
        }
        else if (event.key.keysym.sym == 'p') {
            // print out depth of previously drawn triangle
            std::cout << twodTriangles[indexcheck-1].v0().depth << ""
            << twodTriangles[indexcheck-1].v1().depth << ""
            << twodTriangles[indexcheck-1].v2().depth << std::endl;
        }

	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, true);
	SDL_Event event;

    std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
    std::vector<ModelTriangle> modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35);

    // RASTERIZER
    indexcheck = 0;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
        twodTriangles = rasterize(window, modelTriangles, cameraPosition, focalLength, scale, depthMatrix);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}

// SengulCG2023/Weekly Workbooks/01 Introduction and Orientation/extras/RedNoise/src/RedNoise.cpp