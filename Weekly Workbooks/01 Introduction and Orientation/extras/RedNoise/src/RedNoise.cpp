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
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

    std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
    std::vector<ModelTriangle> modelTriangles = readObj("models/cornell-box.obj", mtls, 0.35);

//    for (const ModelTriangle& value : modelTriangles) {
//        std::cout << value << std::endl;
//    }

    glm::vec3 cameraPosition {0.0, 0.0, 4.0};
    float focalLength = 1.5;
    float scale = 240.0f;
	// Loop over model triangles in the obj file
	// Point cloud render
	for (const ModelTriangle& triangle : modelTriangles) {
		// Loop over vec3s in the current model triangle
		std::array<glm::vec3, 3> currVertices = triangle.vertices;
		for (const glm::vec3 currVertex : currVertices) {
			// Calculate the CanvasPoint for the vertex
			CanvasPoint currIntersection = getCanvasIntersectionPoint(currVertex, cameraPosition, focalLength, scale);
			// Draw the point on the window
			drawPoint(window, currIntersection, {255, 255, 255});
		}
	}


    //	 Wireframe render: create a 2D CanvasTriangle for each 3D ModelTriangle
    std::vector<CanvasTriangle> twodTriangles;
    for (ModelTriangle &modelTriangle : modelTriangles) {
        // Define canvas tri.
        CanvasTriangle canvasTriangle;
        for (int i = 0; i < 3; i++) {
        // Loop over: 0, 1, 2 -- vertices of current tri.
            canvasTriangle.vertices[i] = getCanvasIntersectionPoint(modelTriangle.vertices[i], cameraPosition, focalLength, scale);
        }
        // Finished w/ def~n of tri, draw tri.
        twodTriangles.push_back(canvasTriangle);
        drawFilled(window, canvasTriangle, modelTriangle.colour);
    }

//    for (CanvasTriangle &tri : twodTriangles) {
//        drawFilled(window, tri, {255,255,255});
//    }

    //	 Print model triangle vertices
    //	 for (const ModelTriangle& value : modelTriangles) {
    //     	std::cout << value << std::endl;
    //	 }

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		
		/* TESTING
		 float centreX = (WIDTH-1)/2;
		 float centreY = (HEIGHT-1)/2;
		 float third = WIDTH/3;
		 Colour red = {"red", 255, 0, 0};
		 drawLine(window, 0, 0, centreX, centreY, red);
		 drawLine(window, (WIDTH)-1, 0, centreX, centreY, red);
		 drawLine(window, centreX, 0, centreX, (HEIGHT)-1, red);
		 drawLine(window, third, centreY, third+third, centreY, red);
		 CanvasPoint v0 = {100,100,0,0};
		 CanvasPoint v1 = {200,50,0,0};
		 CanvasPoint v2 = {150,(HEIGHT)-1,0,0};
		 CanvasTriangle triangle = {v0,v1,v2};
		 drawFilled(window, triangle, red);
		 */

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}

	/* interpolateThreeElementValues test
	 glm::vec3 from(1.0, 4.0, 9.2);
     glm::vec3 to(4.0, 1.0, 9.8);
     int numberOfValues = 4; // You can change this to the desired number of interpolated values.
     std::vector<glm::vec3> interpolatedValues = interpolateThreeElementValues(from, to, numberOfValues);
     // Printing the interpolated values
     for (const glm::vec3& value : interpolatedValues) {
         std::cout << "(" << value.x << ", " << value.y << ", " << value.z << ")" << std::endl;
     } */
}