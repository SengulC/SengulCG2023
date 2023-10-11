#include <vector>
#include <iostream> 

int main(int argc, char *argv[]) {
    std::vector<int> ys = {2, 3, 5};
    std::sort(ys.begin(), ys.end(), std::greater<int>());

    // Using a range-based for loop to print each element
    std::cout << "Contents of ys: ";
    for (const int& element : ys) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    return 0;
}