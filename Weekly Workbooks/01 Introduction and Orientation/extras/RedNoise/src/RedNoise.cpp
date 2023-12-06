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
#include "glm/ext.hpp"

#define WIDTH 320
#define HEIGHT 240

std::vector<CanvasTriangle> twodTriangles;
int indexcheck;
bool toggle = true;
bool orbit = false;
std::vector<std::vector<float>> depthMatrix(WIDTH, std::vector<float>(HEIGHT, 0.0f));
float focalLength = 1.8;
float scale = 240.0f;
glm::vec3 cameraPosition {0.0, 0.0, 4.0};
std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
std::vector<ModelTriangle> sphereTriangles = readObj("models/sphere.obj", mtls, 0.35, true);

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
glm::vec3 lightPosition(0.0,/*0.5*/0.5,/*2.5*/0.5);

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
        // translation: up/down/left/right
        if (event.key.keysym.sym == SDLK_UP) {
            cameraPosition += glm::vec3{0,0.1,0};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        else if (event.key.keysym.sym == SDLK_DOWN) {
            cameraPosition -= glm::vec3{0,0.1,0};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        else if (event.key.keysym.sym == SDLK_LEFT) {
            cameraPosition -= glm::vec3{0.1,0,0};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
		else if (event.key.keysym.sym == SDLK_RIGHT) {
            cameraPosition += glm::vec3{0.1,0,0};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        // translation: z axis: ;/'
        else if (event.key.keysym.sym == SDLK_SEMICOLON) {
            cameraPosition -= glm::vec3{0,0,0.1};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        else if (event.key.keysym.sym == SDLK_QUOTE) {
            cameraPosition += glm::vec3{0,0,0.1};
            std::cout<< cameraPosition.x << " "<< cameraPosition.y << " "<< cameraPosition.z << std::endl;
        }
        // rotation </>
        else if (event.key.keysym.sym == SDLK_COMMA) {
            cameraPosition = rotateX * cameraPosition;
        }
        else if (event.key.keysym.sym == SDLK_PERIOD) {
            cameraPosition = rotateY * cameraPosition;
        }
        // toggle orbit
        else if (event.key.keysym.sym == 'o') {
            if (orbit) { orbit = false; } else { orbit = true; }
        }
        // lookAt origin
        else if (event.key.keysym.sym == 'l') {
            cameraOrientation = LookAt(cameraOrientation, glm::vec3(0,0,0), cameraPosition);
        }
        // tilting/panning
        else if (event.key.keysym.sym == 't') {
            cameraOrientation = rotateX * cameraOrientation;
        }
        else if (event.key.keysym.sym == 'p') {
            cameraOrientation = rotateY * cameraOrientation;
        }
        // triangle stuff
		else if (event.key.keysym.sym == 'u') {
			drawStroked(window, randomTriangle(), randomColor(), depthMatrix);
		}
		else if (event.key.keysym.sym == 'f') {
            drawFilled(window, randomTriangle(), randomColor(), depthMatrix);
        }
//        else if (event.key.keysym.sym == 'w') {
//            // draw wireframe
//            for (const CanvasTriangle& tri : twodTriangles) {
//                drawStroked(window, tri, {255,255,255}, depthMatrix);
//            }
//        }
        // toggle wireframe atop render
        else if (event.key.keysym.sym == 'y') {
            window.clearPixels();
            std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
            auto modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35, false);
            std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3, std::vector<std::vector<float>>> tuple;
            tuple = drawRasterizedScene(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale,
                                        depthMatrix, orbit);
            twodTriangles = std::get<0>(tuple);
            cameraPosition = std::get<1>(tuple);
            cameraOrientation = std::get<2>(tuple);
            depthMatrix = std::get<3>(tuple);
            indexcheck=0;
            if (toggle){
                toggle = false;
                for (const CanvasTriangle &tri: twodTriangles) {
                    drawStroked(window, tri, {255, 255, 255}, depthMatrix);
                }
            } else {
                toggle = true;
                window.clearPixels();
                std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3, std::vector<std::vector<float>>> tuple;
                tuple = drawRasterizedScene(window, modelTriangles, cameraPosition, cameraOrientation, focalLength,
                                            scale, depthMatrix, orbit);
                twodTriangles = std::get<0>(tuple);
                cameraPosition = std::get<1>(tuple);
                cameraOrientation = std::get<2>(tuple);
                depthMatrix = std::get<3>(tuple);
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
            depthMatrix = drawFilled(window, twodTriangles[indexcheck], randomColor(), depthMatrix);
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
        // print out depth of previously drawn triangle
        else if (event.key.keysym.sym == 'e') {
            std::cout << twodTriangles[indexcheck-1].v0().depth << ""
            << twodTriangles[indexcheck-1].v1().depth << ""
            << twodTriangles[indexcheck-1].v2().depth << std::endl;
        }
        // lighting. z+-
        else if (event.key.keysym.sym == 'a'){
            lightPosition -= glm::vec3(0,0,0.1);
            std::cout<<"light: " << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z <<std::endl;
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
        else if (event.key.keysym.sym == 'd'){
            lightPosition += glm::vec3(0,0,0.1);
            std::cout<<"light: " << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z <<std::endl;
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
        // lighting. y+-
        else if (event.key.keysym.sym == 'w'){
            lightPosition -= glm::vec3(0,0.1,0);
            std::cout<<"light: " << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z <<std::endl;
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
        else if (event.key.keysym.sym == 's'){
            lightPosition += glm::vec3(0,0.1,0);
            std::cout<<"light: " << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z <<std::endl;
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
        else if (event.key.keysym.sym == 'h'){
            drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

bool areColoursEqual(const Colour& c1, const Colour& c2) {
    return c1.red == c2.red && c1.green == c2.green && c1.blue == c2.blue;
}

// Function to filter triangles by colour
std::vector<ModelTriangle> filterTrianglesByColour(const std::vector<ModelTriangle>& triangles, const std::vector<Colour>& targetColours) {
    std::vector<ModelTriangle> filteredTriangles;

    for (const ModelTriangle& triangle : triangles) {
        for (const Colour& targetColour : targetColours) {
            if (areColoursEqual(triangle.colour, targetColour)) {
                filteredTriangles.push_back(triangle);
                break; // Break out of the inner loop once a match is found
            }
        }
    }

    return filteredTriangles;
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, true);
	SDL_Event event;

/*        for (auto &pair : mtls) {
        if (pair.first == "Grey") {
            std::cout<< "Grey" <<(pair.second) << std::endl;
        }
    }*/

    // auto modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35, false);

    Colour red (255,0,0); Colour blue (0,0,255); Colour cyan (0,255,255); Colour white (255,255,255);
    Colour gray(178,178,178), yellow(255,255,0), green(0,255,0), pink(255,0,255);
    std::vector<Colour> colors {red, blue, cyan, green, gray, yellow, pink};
    // std::vector<ModelTriangle> filteredTriangles = filterTrianglesByColour(modelTriangles, colors);

    // RASTERIZER
//    auto modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35, false);
//    auto tuple = drawRasterizedScene(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix, orbit);
//    tuple = drawRasterizedScene(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix, orbit);
//    twodTriangles = std::get<0>(tuple);
//    cameraPosition = std::get<1>(tuple);
//    cameraOrientation = std::get<2>(tuple);
//    depthMatrix = std::get<3>(tuple);

    // RAYTRACER
    auto modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35, false);
//    drawRaytracedScene(window, modelTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
//    drawGouraucedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
//    drawPhongdScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
//   VERTEX NORMALS DEBUGGING
//    std::cout<<sphereTriangles.size()<<std::endl;
//    int index = 0;
//    for (ModelTriangle ct : sphereTriangles) {
//        if (ct.vertexNormals.empty()) {
//            std::cout<<"empty at tri index: "<< index <<std::endl;
//        }
//        index++;
//    }


//    std::vector<glm::vec3> lightPositins
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
        // // RASTERIZER
//        std::tuple<std::vector<CanvasTriangle>, glm::vec3, glm::mat3, std::vector<std::vector<float>>> tuple;
//        tuple = drawRasterizedScene(window, sphereTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix, orbit);
//        twodTriangles = std::get<0>(tuple);
//        cameraPosition = std::get<1>(tuple);
//        cameraOrientation = std::get<2>(tuple);
        drawRaytracedScene(window, modelTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
//        lightPosition += glm::vec3{0.1,0,0};
         //drawRaytracedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
//		 Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
