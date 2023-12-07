#pragma once



std::vector<std::vector<float>> drawStroked(DrawingWindow &window, CanvasTriangle triangle, Colour color, std::vector<std::vector<float>> depthMatrix);
std::vector<std::vector<float>> drawFilled(DrawingWindow &window, CanvasTriangle triangle, Colour color, std::vector<std::vector<float>> depthMatrix);
CanvasTriangle randomTriangle();
Colour randomColor();
CanvasTriangle modelToCanvasTriangle(ModelTriangle mTri);
// void drawTextureFilled(DrawingWindow &window, CanvasTriangle triangle, const TextureMap& textureMap, const std::vector<TexturePoint>& texturePoints);