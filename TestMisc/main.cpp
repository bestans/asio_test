#include <iostream>
#include <array>

using namespace std;
struct ArrElement {
	int value;
	ArrElement() {
		value = 0;
		cout << "construct 1111\n";
	}
	ArrElement(const ArrElement& other) {
		value = other.value;
		cout << "construct 22222\n";
	}
};
int main(int argc, char* argv[]) {
	std::array<ArrElement, 5> arr{};
	for (auto& it : arr) {
		std::cout << it.value << std::endl;
	}
	return 0;
}