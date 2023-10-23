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
#include <OurLine.h>
#include <OurTriangle.h>
#include <OurObject.h>

#define WIDTH 320
#define HEIGHT 240
std::vector<CanvasTriangle> twodTriangles;
int indexcheck;

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == 'u') {
			drawStroked(window, randomTriangle(), randomColor());
		}
		else if (event.key.keysym.sym == 'f') {
			drawFilled(window, randomTriangle(), randomColor());
		}
        else if (event.key.keysym.sym == 'x') {
            if(indexcheck == twodTriangles.size()){
                indexcheck =0;
            }
            drawFilled(window, twodTriangles[indexcheck], randomColor());
            indexcheck++;
        }
        else if (event.key.keysym.sym == 'c') {
            window.clearPixels();
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

//    for (const ModelTriangle& value : modelTriangles) {
//        std::cout << value << std::endl;
//    }

    glm::vec3 cameraPosition {0.0, 0.0, 4.0};
    float focalLength = 1.5;
    float scale = 240.0f;

    //	 Wireframe render: create a 2D CanvasTriangle for each 3D ModelTriangle
    for (ModelTriangle &modelTriangle : modelTriangles) {
        CanvasTriangle canvasTriangle;
        for (int i = 0; i < 3; i++) {
            canvasTriangle.vertices[i] = getCanvasIntersectionPoint(modelTriangle.vertices[i], cameraPosition, focalLength, scale);
        }
        twodTriangles.push_back(canvasTriangle);
        indexcheck=0;
        drawFilled(window, canvasTriangle, modelTriangle.colour);
    }
    CanvasTriangle flatbottom;
    flatbottom.vertices= {CanvasPoint{WIDTH/2,50}, CanvasPoint{50,150}, CanvasPoint{200,150}};
//    drawFilled(window,flatbottom, {255,40,20});


    CanvasTriangle flattop;
    flattop.vertices= {CanvasPoint{50,50}, CanvasPoint{75,50}, CanvasPoint{100,100}};
//    drawFilled(window,flattop, {100,40,20});
//    drawFilled(window, twodTriangles[6], {255,0,0});

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}