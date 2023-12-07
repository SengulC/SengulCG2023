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
bool orbit = true;
std::vector<std::vector<float>> depthMatrix(WIDTH, std::vector<float>(HEIGHT, 0.0f));
float focalLength = 2.0; // sphere
//float focalLength = 1.8; //cornell
float scale = 240.0f;
glm::vec3 cameraPosition {0.0, 0.5, 4.0}; /*sphere*/
//glm::vec3 cameraPosition {0.0, 0.0, 4.0}; // cornell
glm::vec3 lightPosition {0.0, 0.6, 0.0}; // cornell reg
//glm::vec3 lightPosition(-0.5, -0.5,-0.5); // cornell blue box specular
//glm::vec3 lightPosition(1.5, -1.5,0.5); // cornell RED box specular
std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
std::vector<ModelTriangle> sphereTriangles = readObj("models/sphere.obj", mtls, 0.35, true);

glm::mat3 cameraOrientation(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
        );
glm::mat3 origin(
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
//glm::vec3 lightPosition(0.0,0.5,2.5); sphere

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
            drawRaytracedScene(window, modelTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
        }
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}


int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false); //fullscreen
	SDL_Event event;

     auto modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35, false);

    // RASTERIZER
//    auto modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35, false);
//    auto tuple = drawRasterizedScene(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix, orbit, false);
//    tuple = drawRasterizedScene(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, scale, depthMatrix, orbit);
//    twodTriangles = std::get<0>(tuple);
//    cameraPosition = std::get<1>(tuple);
//    cameraOrientation = std::get<2>(tuple);
//    depthMatrix = std::get<3>(tuple);

    // RAYTRACER
//    drawRaytracedScene(window, modelTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
//    drawGouraucedScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);
//    drawPhongdScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lightPosition);

    std::vector<glm::vec3> lights = {{-0.1,0.5,2.5}, {0.0,0.5,2.5}, {0.1,0.5,2.5},
                                     {-0.1,0.6,2.5}, {0.0,0.6,2.5}, {0.1,0.6,2.5},
                                     {-0.1,0.5,2.5}, {0.0,0.5,2.5}, {0.1,0.5,2.5}};
//    std::vector<glm::vec3> lights = {{-0.6, -0.5,-0.5}, {1.5, -1.5,0.5}}; // cornell blue box specular
    int count = 0;

    while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);

        if (count < lights.size()) {
            printVec3("light", lights[count]);
            drawPhongdScene(window, sphereTriangles, scale, focalLength, cameraPosition, cameraOrientation, lights[count]);
//            window.savePPM("./CornellLighting/phong" + std::to_string(count) + ".ppm") ;
//            window.saveBMP("./CornellLighting/phong" + std::to_string(count) + ".bmp") ;
        } else {
            std::cout<<"done"<<std::endl;
        }
        count++;

//		 Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
