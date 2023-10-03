#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#define WIDTH 320
#define HEIGHT 240

// returns an evenly spaced list of size numberOfValues that contains floating point numbers between from and to. 
 std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
	float step = (to-from)/(numberOfValues-1);
	std::vector<float> vect;
	vect.push_back(from);
	float next = from;
	for (int i = 0; i < numberOfValues-2; i++) {
		float num = next+step;
		vect.push_back(num);
		next = next + step;
	}
	vect.push_back(to);
	return vect;
}

 std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
	std::vector<glm::vec3> vect;
	std::vector<float> zero = interpolateSingleFloats(from[0], to[0], numberOfValues);
	std::vector<float> one = interpolateSingleFloats(from[1], to[1], numberOfValues);
	std::vector<float> two = interpolateSingleFloats(from[2], to[2], numberOfValues);
	
	for (int i = 0; i < numberOfValues; i++) {
		glm::vec3 temp(zero[i], one[i], two[i]);
		vect.push_back(temp);
	}

	return vect;
}

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

void draw(DrawingWindow &window) {
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

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	// DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	// SDL_Event event;
	// std::vector<glm::vec3> result;
	// glm::vec3 from(1.0, 4.0, 9.2);
	// glm::vec3 to(4.0, 1.0, 9.8);
	// result = interpolateThreeElementValues(from, to, 4);
	// for(size_t i=0; i<result.size(); i++) std::cout << result[i] << " ";
	// std::cout << std::endl;
	// while (true) {
	// 	// We MUST poll for events - otherwise the window will freeze !
	// 	if (window.pollForInputEvents(event)) handleEvent(event, window);
	// 	draw(window);
	// 	// Need to render the frame at the end, or nothing actually gets shown on the screen !
	// 	window.renderFrame();
	// }
	glm::vec3 from(1.0, 4.0, 9.2);
    glm::vec3 to(4.0, 1.0, 9.8);
    int numberOfValues = 4; // You can change this to the desired number of interpolated values.

    std::vector<glm::vec3> interpolatedValues = interpolateThreeElementValues(from, to, numberOfValues);

    // Printing the interpolated values
    for (const glm::vec3& value : interpolatedValues) {
        std::cout << "(" << value.x << ", " << value.y << ", " << value.z << ")" << std::endl;
    }
}

//take these two functions in  c++, how do i check this works with example inputs from and to