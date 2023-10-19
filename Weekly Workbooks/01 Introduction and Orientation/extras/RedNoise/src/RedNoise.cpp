#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <ModelTriangle.h>
#include <iostream>
#include <string>
#include <map>

#define WIDTH 320
#define HEIGHT 240

std::vector<int> unpack(const Colour& color) {
	std::vector<int> colour = {color.red, color.green, color.blue};
	return colour;
}

uint32_t pack(std::vector<int> colorgb) {
	return (255 << 24) + (int(colorgb[0]) << 16) + (int(colorgb[1]) << 8) + int(colorgb[2]);
}

bool sortByY(const CanvasPoint& a, const CanvasPoint& b) {
    return a.y < b.y;
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

		int xval = std::round(x);
		int yval = std::round(y);
		if (xval > WIDTH || yval > HEIGHT) {
			continue;
		} else {
			window.setPixelColour(xval, yval, fincolor);
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

void drawFilled(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
	std::vector<CanvasPoint> points = {triangle.v0(), triangle.v1(), triangle.v2()};
	std::sort(points.begin(), points.end(), sortByY); // sorted in ascending order of Ys
	
	// split triangle into 2 from middle vertex
	CanvasPoint middleV = points[1];
	int topHeight = std::abs(middleV.y - points[0].y);
	int bottomHeight = std::abs(points[2].y - middleV.y);

	// interpolate from 1st point to last point to find extra vertex
	std::vector<float> extraVInterpolatedX = interpolateSingleFloats(points[0].x, points[2].x, topHeight+bottomHeight);
	float extraVx = extraVInterpolatedX[topHeight];
	CanvasPoint extraV = {extraVx, middleV.y};

	// define triangles for verification
	// CanvasTriangle topTriangle = {points[0], middleV, extraV};
	// CanvasTriangle bottomTriangle = {points[2], middleV, extraV};

	int count = 0;
	for (int i=points[0].y; i<(extraV.y); i++) {
		std::vector<float> topTriangleLeft = interpolateSingleFloats(points[0].x, extraV.x, topHeight);
		std::vector<float> topTriangleRight = interpolateSingleFloats(points[0].x, middleV.x, topHeight);
		drawLine(window, topTriangleLeft[count], i, topTriangleRight[count], i, color);
		count++;
	}
	
	count = 0;
	for (int i = middleV.y + 1; i <= points[2].y; i++) {
		std::vector<float> bottomTriangleLeft = interpolateSingleFloats(extraV.x + 1, points[2].x, bottomHeight);
		std::vector<float> bottomTriangleRight = interpolateSingleFloats(middleV.x, points[2].x, bottomHeight);
		drawLine(window, bottomTriangleLeft[count], i, bottomTriangleRight[count], i, color);
		count++;
	}

	drawLine(window, extraV.x, extraV.y, middleV.x, middleV.y, color);
	drawStroked(window, triangle, {255, 255, 255});
	return;
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
		else if (event.key.keysym.sym == 'f') {
			drawFilled(window, randomTriangle(), randomColor());
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

std::vector<ModelTriangle> readObj(std::string file, float scale) {
    // remember that vertices in OBJ files are indexed from 1 (whereas vectors are indexed from 0).

    // modelTriangle: const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, Colour trigColour
    std::vector<ModelTriangle> modelTriangles;
    ModelTriangle tempTriangle;
    std::vector<glm::vec3> vertices;
    Colour trigColour = {255,0,0};

    std::string myObj; // init string
    std::ifstream theObjFile(file); // read file

    // read the file line by line
    // make a list of vec3s for the vertices
    while (getline (theObjFile, myObj)) {
        if (myObj[0] == 'v') {
            std::vector<std::string> xyz = split(myObj, ' ');
            glm::vec3 currVector{std::stof(xyz[1])*scale, std::stof(xyz[2])*scale, std::stof(xyz[3])*scale}; // xyz[0] = 'v'
            vertices.push_back(currVector);
        }
        else if (myObj[0] == 'f') {
            // e.g. myObj = "f 2/ 3/ 4/"
            std::vector<std::string> facet = split(myObj, ' '); // ["f", "2/", "3/", "4/"]
            // facet[x][0] gets vertex num. convert that to int. look up that vec3 in vertices list (-1 bc of indexing).
            glm::vec3 v0 = vertices[std::stoi(facet[1])-1];
            glm::vec3 v1 = vertices[std::stoi(facet[2])-1];
            glm::vec3 v2 = vertices[std::stoi(facet[3])-1];
            tempTriangle = {v0,v1,v2, trigColour};
            modelTriangles.push_back(tempTriangle);
        }
    }
    theObjFile.close();
    return modelTriangles;
}

std::map<std::string, Colour> readMaterial(std::string file) {
	std::map<std::string, Colour> palette;
	std::string myObj; // init string
	std::ifstream theObjFile(file); // read file
	std::string name;
	
	// read the file line by line
	// add a colour to the palette for each newmtl
	while (getline (theObjFile, myObj)) {
		if (myObj[0] == 'n') {
			name = split(myObj, ' ')[1];
		}
		if (myObj[0] == 'K') {
			std::vector<std::string> nrgb = split(myObj, ' ');
			// [Kd, 0.700000, 0.700000, 0.700000]
			float r = std::stof(nrgb[1])*255;
			float g = std::stof(nrgb[2])*255;
			float b = std::stof(nrgb[3])*255;
			Colour currColor = {name, int(r), int(g), int(b)};
			palette.insert({name,currColor});
		}
	}

	theObjFile.close();
	return palette;
}

CanvasPoint getCanvasIntersectionPoint(glm::vec3 vertexPosition, glm::vec3 cameraPosition, float focalLength, float scale) {
    float u, v;
    CanvasPoint intersection{0.0, 0.0, 0.0}; // Initialize depth to 0.0

    // Calculate the relative position of the vertex in camera coordinates
    glm::vec3 relativePosition = vertexPosition - cameraPosition;

    // Apply scaling to the relative position
    relativePosition = relativePosition * glm::vec3{scale, scale, scale};

    // Calculate the 2D coordinates on the image plane
    u = (focalLength * (relativePosition.x / relativePosition.z)) + (WIDTH / 2);
    v = (focalLength * (relativePosition.y / relativePosition.z)) + (HEIGHT / 2);

    intersection = {u, v, relativePosition.z}; // Include the z-coordinate
    return intersection;
}

void drawPoint(DrawingWindow &window, CanvasPoint point, Colour color) {
	window.setPixelColour(point.x, point.y, pack(unpack(color)));
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	std::map<std::string, Colour> mtls = readMaterial("models/cornell-box.mtl");
	// for (const auto& pair : mtls) {
    // std::cout << "Material Name: " << pair.first << ", Colour: ("
    //           << pair.second.red << ", " << pair.second.green << ", " << pair.second.blue << ")\n";
	// }

	std::vector<ModelTriangle> modelTriangles = readObj("models/cornell-box.obj", 1);
//    for (const ModelTriangle& value : modelTriangles) {
//        std::cout << value << std::endl;
//    }

	// Loop over model triangles in the obj file
	// Point cloud render
	for (const ModelTriangle& triangle : modelTriangles) {
		// Loop over vec3s in the current model triangle
		std::array<glm::vec3, 3> currVertices = triangle.vertices;
		for (const glm::vec3 currVertex : currVertices) {
			// Calculate the CanvasPoint for the vertex
            glm::vec3 cameraPosition {0.0, 0.0, 4.0};
			float focalLength = 8.0;
            float scale = 240.0f;
			CanvasPoint currIntersection = getCanvasIntersectionPoint(currVertex, cameraPosition, focalLength, scale);

			// Draw the point on the window
			drawPoint(window, currIntersection, {255, 255, 255});
		}
	}

//	 std::vector<CanvasTriangle> twodTriangles;
//	  Wireframe render: create a 2D CanvasTriangle for each 3D ModelTriangle
    glm::vec3 cameraPosition {0.0,0.0,2.0};
    float focalLength= 2.0f;
    float scale = 240.0f;
    for (ModelTriangle &modelTriangle : modelTriangles) {
        // Convert the model triangle to a canvas triangle
        CanvasTriangle canvasTriangle;
        for (int i = 0; i < 3; i++) {
            canvasTriangle.vertices[i] = getCanvasIntersectionPoint(modelTriangle.vertices[i], cameraPosition, focalLength, scale);
        }
        // Draw the canvas triangle
        drawStroked(window, canvasTriangle, modelTriangle.colour);
    }



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