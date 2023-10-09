#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>

#define WIDTH 320
#define HEIGHT 240

std::vector<int> unpack(Colour color) {
	std::vector<int> colour = {color.red, color.green, color.blue};
	return colour;
}

uint32_t pack(std::vector<int> colorgb) {
	return (255 << 24) + (int(colorgb[0]) << 16) + (int(colorgb[1]) << 8) + int(colorgb[2]);
}

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

void drawLine(DrawingWindow &window, float fromX, float fromY, float toX, float toY, Colour color) {
	float xDiff = toX-fromX;
	float yDiff = toY-fromY;
	float steps = std::max(std::abs(xDiff), std::abs(yDiff));
	float xSteps = xDiff / steps;
	float ySteps = yDiff / steps;

	std::vector<int> colorgb = unpack(color);
	uint32_t fincolor = pack(colorgb);
	
	for (float i = 0.0; i < steps; i++) {
		float x = fromX + (xSteps*i);
		float y = fromY + (ySteps*i);
		if (y < 240) {
			window.setPixelColour(std::round(x), std::round(y), fincolor);
		}
	}
	return;
}

void drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
	drawLine(window, triangle.v0().x, triangle.v0().y, triangle.v1().x, triangle.v1().y, color);
	drawLine(window, triangle.v1().x, triangle.v1().y, triangle.v2().x, triangle.v2().y, color);
	drawLine(window, triangle.v0().x, triangle.v0().y, triangle.v2().x, triangle.v2().y, color);
	return;
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

CanvasTriangle randomTriangle() {
	CanvasPoint v0 = {static_cast<float>(rand()%320),static_cast<float>(rand()%240),0,0};
	CanvasPoint v1 = {static_cast<float>(rand()%320),static_cast<float>(rand()%240),0,0};
	CanvasPoint v2 = {static_cast<float>(rand()%320),static_cast<float>(rand()%240),0,0};
	CanvasTriangle triangle = {v0,v1,v2};
	return triangle;
}

Colour randomColor() {
	Colour color = {rand()%256, rand()%256, rand()%256};
	return color;
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == 'u') {
			drawStroked(window, randomTriangle(), randomColor());
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		float centreX = (WIDTH-1)/2;
		float centreY = (HEIGHT-1)/2;
		float third = WIDTH/3;
		Colour red = {"red", 255, 0, 0};
		
		// drawLine(window, 0, 0, centreX, centreY, red); 
		// drawLine(window, (WIDTH)-1, 0, centreX, centreY, red); 
		// drawLine(window, centreX, 0, centreX, (HEIGHT)-1, red); 
		// drawLine(window, third, centreY, third+third, centreY, red); 
		// CanvasPoint(float xPos, float yPos, float pointDepth, float pointBrightness) :
		CanvasPoint v0 = {(WIDTH)-1,0,0,0};
		CanvasPoint v1 = {third,centreY,0,0};
		CanvasPoint v2 = {third+third,(HEIGHT)-1,0,0};
		CanvasTriangle triangle = {v0,v1,v2};
		drawStroked(window, triangle, red);

		if (SDL_PollEvent(&event)) {
			if (event.type == SDLK_u) {
				drawStroked(window, randomTriangle(), randomColor());
			}
		}

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}

	// interpolateThreeElementValues test
	// glm::vec3 from(1.0, 4.0, 9.2);
    // glm::vec3 to(4.0, 1.0, 9.8);
    // int numberOfValues = 4; // You can change this to the desired number of interpolated values.
    // std::vector<glm::vec3> interpolatedValues = interpolateThreeElementValues(from, to, numberOfValues);
    // // Printing the interpolated values
    // for (const glm::vec3& value : interpolatedValues) {
    //     std::cout << "(" << value.x << ", " << value.y << ", " << value.z << ")" << std::endl;
    // }
}