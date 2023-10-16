#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string> 

std::vector<std::string> split(const std::string &line, char delimiter) {
	auto haystack = line;
	std::vector<std::string> tokens;
	size_t pos;
	while ((pos = haystack.find(delimiter)) != std::string::npos) {
		tokens.push_back(haystack.substr(0, pos));
		haystack.erase(0, pos + 1);
	}
	// Push the remaining chars onto the vector
	tokens.push_back(haystack);
	return tokens;
}

// Weekly Workbooks/01 Introduction and Orientation/extras/RedNoise/src/temp.cpp
int main(int argc, char *argv[]) {
    // std::vector<int> ys = {2, 3, 5};
    // std::sort(ys.begin(), ys.end(), std::greater<int>());

    // // Using a range-based for loop to print each element
    // std::cout << "Contents of ys: ";
    // for (const int& element : ys) {
    //     std::cout << element << " ";
    // }
    // std::cout << std::endl;

    std::vector<std::string> xyz = split("v -0.64901096 2.739334 0.532032", ' ');
    // std::cout << "Contents of xyz: ";
    // for (const std::string& element : xyz) {
    //     std::cout << element << " ";
    // }
    std::cout << xyz[0];
    std::cout << "\n";
    std::cout << xyz[1];
    std::cout << "\n";
    std::cout << xyz[2];
    std::cout << "\n";
    std::cout << xyz[3];
    std::cout << std::endl;
    return 0;
}