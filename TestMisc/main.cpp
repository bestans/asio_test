#include <iostream>
#include <array>
#include "test_misc.h"
#include <queue>
#include <vector>
#include "stat_balance.h"
#include <string>
#include <sstream>
#include <stdlib.h>

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
void test1() {
	std::array<ArrElement, 5> arr{};
	for (auto& it : arr) {
		std::cout << it.value << std::endl;
	}
}
std::string ToString(std::array<int, 5>& arr) {
	std::stringstream ss;
	for (int i = 0; i < arr.size(); i++) {
		ss << arr[i] << " ";
	}
	ss << std::endl;
	return ss.str();
}
void test3() {
	StatBalance info(5);
	std::array<int, 5> arr({ 0 });
	std::array<int, 5> weight({ 0 });
	for (int i = 0; i < 10000; i++) {
		auto w = rand() % 20;
		auto index = info.AllocIndex(w);
		arr[index]++;
		weight[index] += w;
	}
	std::cout << ToString(arr) << "," << ToString(weight) << std::endl;
	for (int i = 0; i < 5; i++) {
		auto w = rand() % weight[i];
		weight[i] -= w;
		info.FreeIndex(i, w);
	}
	std::cout << ToString(arr) << "," << ToString(weight) << std::endl;
	for (int i = 0; i < 3000; i++) {
		auto w = rand() % 20;
		auto index = info.AllocIndex(w);
		arr[index]++;

		weight[index] += w;
	}
	std::cout << std::endl;
	std::cout << ToString(arr) << "," << ToString(weight)  << std::endl;
}
class tlData {
public:
	//tlData() : tlData(100) {

	//}
	tlData(size_t size) {
		ptr_ = new char[size];
		size_ = size;
		std::cout << "tlData:" << std::this_thread::get_id() << std::endl;
	}
	~tlData() {
		delete ptr_;
		ptr_ = nullptr;
		size_ = 0;
		std::cout << "~tlData:" << std::this_thread::get_id() << std::endl;
	}
	size_t Size() {
		return size_;
	}
	public:
	char* ptr_;
	size_t size_;
};
void test_thread_local() {
	thread_local tlData value(100);
	std::vector<std::thread> threads;
	std::cout << "begin\n";
	for (int i = 0; i < 10; i++) {
		threads.emplace_back([](){
			std::cout << &value << "," << value.Size() << std::endl;
		});
	}
	for (auto& it : threads) {
		it.join();
	}
}
int main(int argc, char* argv[]) {
	srand(time(0));
	test_thread_local();
	return 0;
}