#include <iostream>
#include <array>
#include "test_misc.h"
#include <queue>
#include <vector>
#include "stat_balance.h"
#include <string>
#include <sstream>
#include <stdlib.h>
#include "buffer_pool.h"

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
	for (size_t i = 0; i < arr.size(); i++) {
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

std::shared_ptr<tlData> GetValue(int len) {
	thread_local std::shared_ptr<tlData> value = std::make_shared<tlData>(len);
	return value;
}
std::mutex gmutex;
void test_thread_local() {
	//thread_local tlData value(100);
	std::vector<std::thread> threads;
	std::cout << "begin\n";
	for (int i = 0; i < 10; i++) {
		threads.emplace_back([](){
			for (int j = 0; j < 3; j++) {
				std::cout << j << ":" << std::this_thread::get_id() << ":" << (int64_t)(GetValue(100 + j)->ptr_) << "," << GetValue(100 + j)->Size() << std::endl;
			}
		});
	}
	for (auto& it : threads) {
		it.join();
	}
}

class IValue {
public:
	IValue() : value_(11) {
		cout << "IValue 0\n";
	}
	IValue(int i) : value_(i) {
		cout << "IValue\n";
	}
	IValue(IValue& i) : value_(i.value_) {
		cout << "IValue&\n";
	}
	IValue(IValue&& i) : value_(i.value_) {
		cout << "IValue&&\n";
	}
	void operator=(IValue& i) {
		cout << "IValue operator=\n";
		value_ = i.value_;
	}
	~IValue() {
		cout << "~IValue\n";
	}
	int value_;
};
void foo1() {
	auto value = make_shared<IValue>(1);
	cout << value.use_count() << !value << endl;
	auto ptr = &value;
	auto value2 = std::move(*ptr);
	cout << value.use_count() << !value << endl;

	cout << value2->value_ << endl;
}
void foo2() {
	shared_ptr<IValue> value;

	cout << value.use_count() << endl;
}
void foo3() {
	vector<IValue> vec(10);
	cout << vec[5].value_ << endl;
}
void foo4() {
	SimpleStack<IValue> value(5, []() {
			return std::make_unique<IValue>(111);
		});
	auto val = value.Pop();
	cout << "empty:" << !val << "," << val->value_ << endl;
	auto newv = make_unique<IValue>(12);
	cout << "empty newv:" << !newv << endl;
	value.Push(std::move(newv));
	cout << "empty newv:" << !newv << endl;
	auto val2 = value.Pop();
	cout << val2->value_ << endl;
}
IValue GetValue(IValue& v) {
	cout << "22222\n";
	return std::move(v);
}
void foo5() {
	IValue v(100);
	cout << "1111\n";
	auto a = GetValue(v);
	cout << a.value_ << endl;
}

int MyCalcIndex(size_t buffer_size, int start_index_) {
	auto last_value = buffer_size;

	buffer_size >>= start_index_;
	if (buffer_size == 0) {
		return -1;
	}
	int index = 0;
	while (buffer_size != 1) {
		last_value = buffer_size;
		buffer_size >>= 1;
		index++;
	}
	if (last_value == 2) {
		index--;
	}
	return index;
}
const unsigned int tabel[32] = { 0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };

inline unsigned int log2_int_test(register unsigned int x) {
	register unsigned int ans = 0;
	auto srcx = x;
	if (x >= 65536) { ans += 16; x >>= 16; }
	if (x >= 256) { ans += 8; x >>= 8; }
	if (x >= 16) { ans += 4; x >>= 4; }
	auto n = ans + tabel[x];
	auto left = (1 << n) != srcx;
	cout << "log2:" << srcx << "," << n << "," << left << endl;
	return n;
}
void foo6() {
	for (int i = 0; i < 100; i++) {
		log2_int_test(i);
		//cout << i << "," << log2_int_test(i) << endl;
	}
	cout << ((size_t)1 >> 1) << endl;
}
int main(int argc, char* argv[]) {
	srand(time(0));
	foo6();
	return 0;
}
