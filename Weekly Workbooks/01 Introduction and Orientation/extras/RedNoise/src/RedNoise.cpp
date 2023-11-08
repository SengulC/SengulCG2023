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
#include <OurLine.h>
#include <unistd.h>
#include "glm/ext.hpp"

#define WIDTH 320
#define HEIGHT 240

std::vector<CanvasTriangle> twodTriangles;
int indexcheck;
bool toggle = true;
bool colorToggle = true;
std::vector<std::vector<float>> depthMatrix(WIDTH, std::vector<float>(HEIGHT, 0.0f));
float focalLength = 1.0f;
float scale = 150.0f;
glm::vec3 cameraPosition {0.0, 0.0, 4.0};
glm::mat3 cameraOrientation(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
        );
// [right up forward]
glm::mat3 rotateX(
        1.0f, 0.0f, 0.0f,
        0.0f, cos(0.1), -sin(0.1),
        0.0f, sin(0.1), cos(0.1)
);
glm::mat3 rotateY(
        cos(0.1), 0.0f, sin(0.1),
        0.0f, 1.0f, 0.0f,
        -sin(0.1), 0.0f, cos(0.1)
);

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
        // translation: up/down/left/right
        if (event.key.keysym.sym == SDLK_UP) {
            cameraPosition += glm::vec3{0,0.1,0};
        }
        else if (event.key.keysym.sym == SDLK_DOWN) {
            cameraPosition -= glm::vec3{0,0.1,0};
        }
        else if (event.key.keysym.sym == SDLK_LEFT) {
            cameraPosition -= glm::vec3{0.1,0,0};
        }
		else if (event.key.keysym.sym == SDLK_RIGHT) {
            cameraPosition += glm::vec3{0.1,0,0};
        }
        // translation: z axis: ;/'
        else if (event.key.keysym.sym == SDLK_SEMICOLON) {
            cameraPosition -= glm::vec3{0,0,0.1};
        }
        else if (event.key.keysym.sym == SDLK_QUOTE) {
            cameraPosition += glm::vec3{0,0,0.1};
        }
        // rotation </>
        else if (event.key.keysym.sym == SDLK_COMMA) {
            cameraPosition = rotateX * cameraPosition;
        }
        else if (event.key.keysym.sym == SDLK_PERIOD) {
            cameraPosition = rotateY * cameraPosition;
        }
        // triangle stuff
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
        // toggle wireframe atop render
        else if (event.key.keysym.sym == 't') {
            window.clearPixels();
            std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
            std::vector<ModelTriangle> modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35);
            std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3> tuple;
            tuple = rasterize(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix);
            twodTriangles = std::get<0>(tuple);
            cameraPosition = std::get<1>(tuple);
            cameraOrientation = std::get<2>(tuple);
            indexcheck=0;
            if (toggle){
                toggle = false;
                for (const CanvasTriangle &tri: twodTriangles) {
                    drawStroked(window, tri, {255, 255, 255}, depthMatrix);
                }
            } else {
                toggle = true;
                window.clearPixels();
                std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3> tuple;
                tuple = rasterize(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix);
                twodTriangles = std::get<0>(tuple);
                cameraPosition = std::get<1>(tuple);
                cameraOrientation = std::get<2>(tuple);
                indexcheck=0;
            }
        }
        // draw triangles one by one
        else if (event.key.keysym.sym == 'x') {
            Colour color;
            if (indexcheck == 0) {
                window.clearPixels();
            }
            if (indexcheck == twodTriangles.size()) {
                indexcheck = 0;
                window.clearPixels();
            }
//            if (colorToggle) {
//                color = {255,0,0};
//                colorToggle = false;
//            } else {
//                color = {0,0,255};
//                colorToggle = true;
//            }
            depthMatrix = drawFilled(window, twodTriangles[indexcheck], randomColor(), depthMatrix);
            drawStroked(window, twodTriangles[indexcheck], {255,255,255}, depthMatrix);
            indexcheck++;
        }
        // clear
        else if (event.key.keysym.sym == 'c') {
            window.clearPixels();
            depthMatrix = std::vector<std::vector<float>> (WIDTH, std::vector<float>(HEIGHT, 0.0f));
            indexcheck = 0;
            cameraPosition = {0.0, 0.0, 4.0};
            cameraOrientation = glm::mat3 (
                1.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 1.0f
            );
        }
        // rasterise
        else if (event.key.keysym.sym == 'r') {
            depthMatrix = std::vector<std::vector<float>> (WIDTH, std::vector<float>(HEIGHT, 0.0f));
            std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
            std::vector<ModelTriangle> modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35);
            std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3> tuple;
            tuple = rasterize(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix);
            twodTriangles = std::get<0>(tuple);
            cameraPosition = std::get<1>(tuple);
            cameraOrientation = std::get<2>(tuple);
            indexcheck=0;
        }
        // print out depth of previously drawn triangle
        else if (event.key.keysym.sym == 'p') {
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

    for (auto &pair : mtls) {
        if (pair.first == "Grey") {
//            std::cout<< pack(unpack(pair.second)) << std::endl;
        }
    }

    std::vector<ModelTriangle> modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35);

    drawPoint(window, CanvasPoint(WIDTH/2, HEIGHT/2, 0), {255,0,0});

//    bool renderNextTriangle = false;

    // RASTERIZER
//    std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3> tuple;
//    tuple = rasterize(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix);
//    twodTriangles = std::get<0>(tuple);
//    cameraPosition = std::get<1>(tuple);
//    cameraOrientation = std::get<2>(tuple);

//    DEBUUUUG
//    CanvasPoint forward = getCanvasIntersectionPoint(glm::vec3(0, 0, 0), cameraPosition, cameraOrientation, focalLength, scale);
//    std::cout << "intersection of origin/forward: " << forward << std::endl;
//
//    glm::vec3 right = glm::cross(glm::vec3(0,1,0), glm::vec3(forward.x, forward.y, forward.depth));
//    std::cout << "right: " << right.x << " " << right.y << " " << right.z << " " << std::endl;
//
//    glm::vec3 up = glm::cross(glm::vec3(forward.x, forward.y, forward.depth), right);
//    std::cout << "up: " << up.x << " " << up.y << " " << up.z << " " << std::endl;
//
//    cameraPosition =
//            glm::mat3 (
//                    cos(0.01), 0.0f, sin(0.01),
//                    0.0f, 1.0f, 0.0f,
//                    -sin(0.01), 0.0f, cos(0.01)
//            )
//            * cameraPosition;
//
//    forward = getCanvasIntersectionPoint(glm::vec3(0, 0, 0), cameraPosition, cameraOrientation, focalLength, scale);
//    std::cout << "intersection of origin/forward: " << forward << std::endl;
//
//    right = glm::cross(glm::vec3(0,1,0), glm::vec3(forward.x, forward.y, forward.depth));
//    std::cout << "right: " << right.x << " " << right.y << " " << right.z << " " << std::endl;
//
//    up = glm::cross(glm::vec3(forward.x, forward.y, forward.depth), right);
//    std::cout << "up: " << up.x << " " << up.y << " " << up.z << " " << std::endl;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);

        std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3> tuple;
        tuple = rasterize(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix);
        twodTriangles = std::get<0>(tuple);
        cameraPosition = std::get<1>(tuple);
        cameraOrientation = std::get<2>(tuple);
        std::cout<<glm::to_string(cameraOrientation)<<std::endl;

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();

//        if (!renderNextTriangle) {
//            SDL_Delay(500);
//            renderNextTriangle = true;
//            indexcheck++;
//        } else {
//            if (indexcheck < twodTriangles.size()) {
//                SDL_Delay(500);
//                depthMatrix = drawFilled(window, twodTriangles[indexcheck], randomColor(), depthMatrix);
//                indexcheck++;
//            }
//        }
	}
}
