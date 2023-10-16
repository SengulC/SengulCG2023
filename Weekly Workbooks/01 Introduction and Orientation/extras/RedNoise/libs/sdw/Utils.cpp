#include <algorithm>
#include <sstream>
#include "Utils.h"

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

/*
splits a given input string (`line`) into a vector of substrings 
based on a specified delimiter character. Here's how it works:

1. It takes two parameters as input:
   - `line`: A constant reference to the input string that you want to split.
   - `delimiter`: A character that specifies where to split the input string.

2. It creates a copy of the input string, `haystack`, which will be modified during the splitting process.

3. It initializes an empty vector of strings, `tokens`, which will store the resulting substrings.

4. Inside a `while` loop, it repeatedly searches for the next occurrence of the `delimiter` character 
in the `haystack` string using the `find` method. If it finds the delimiter, it returns the position (index) 
of the first occurrence; otherwise, it returns `std::string::npos`, indicating that no more delimiters were found.

5. If a delimiter is found, it extracts the substring from the beginning of `haystack` up to 
(but not including) the delimiter position using `haystack.substr(0, pos)` and appends it to the `tokens` vector using `tokens.push_back()`. 
After that, it removes this part (along with the delimiter) from `haystack` 
using `haystack.erase(0, pos + 1)` to prepare for the next iteration.

6. The loop continues until no more delimiters are found, and the remaining characters in `haystack` 
(which do not contain any delimiters) are pushed onto the `tokens` vector.

7. Finally, the function returns the `tokens` vector, which contains all the substrings created by splitting the 
input string based on the specified delimiter.

This function is a common utility for splitting strings in C++. 
It's worth noting that it doesn't handle delimiters that appear consecutively 
(e.g., multiple spaces between words) or leading/trailing delimiters. 
Depending on your specific use case, you might want to add extra logic to handle such cases if necessary.
*/