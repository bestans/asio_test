#pragma once
#include <mutex>
#include <vector>
#include <queue>
#include <memory>
#include <list>
#include <functional>


const uint32_t log2Table[32] = { 0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3 };
inline uint32_t Log2Int(uint32_t x, bool& has_left) {
	uint32_t ans = 0;
	auto srcx = x;
	if (x >= 65536) { ans += 16; x >>= 16; }
	if (x >= 256) { ans += 8; x >>= 8; }
	if (x >= 16) { ans += 4; x >>= 4; }
	auto n = ans + log2Table[x];
	has_left = (1 << n) != srcx;
	return n;
}

//128M 64M 32M 16M 8M 4M 2M 1M 512K 256K 128K 64K
//

class ByteBuf {
public:
	ByteBuf(size_t size) {

	}
};

template <class T>
class SimpleStack {
public:
	typedef std::function<std::unique_ptr<T>()> CreateFunc;
	SimpleStack(uint32_t capacity, CreateFunc create_func) :
		capacity_(capacity), len_(0), data_list_(4),
		create_func_(std::move(create_func))
	{
	}
	~SimpleStack() {
		data_list_.clear();
	}
	std::unique_ptr<T> Pop() {
		if (len_ > 0) {
			len_--;
			return std::move(data_list_[len_]);
		}
		return std::move(create_func_());
	}
	void Push(std::unique_ptr<T> data) {
		if (len_ < data_list_.size()) {
			data_list_[len_++] = std::move(data);
		}
		else if (data_list_.size() < capacity_) {
			data_list_.resize(std::min(data_list_.size() * 2, capacity_));
			data_list_[len_++] = std::move(data);
		}
	}
private:
	uint32_t capacity_;
	uint32_t len_;
	std::vector<std::unique_ptr<T>> data_list_;
	CreateFunc create_func_;
};

class SectionBuffer {
public:
	SectionBuffer(uint32_t max_count, uint32_t buffer_size) :
		max_count_(max_count),
		buffer_size_(buffer_size),
		data_list_(max_count, [=]() { return std::make_unique<ByteBuf>(buffer_size); }) {

	}
	std::unique_ptr<ByteBuf> GetBuffer() {
		std::lock_guard<std::mutex> lock(mutex_);
		return data_list_.Pop();
	}
	void RecycleBuffer(std::unique_ptr<ByteBuf> buffer) {
		std::lock_guard<std::mutex> lock(mutex_);
		data_list_.Push(std::move(buffer));
	}
private:
	std::mutex mutex_;
	uint32_t buffer_size_;
	uint32_t max_count_;
	SimpleStack<ByteBuf> data_list_;
};
class BufferPool {
	BufferPool(uint32_t max_index, uint32_t min_index) {
		if (min_index < 1)
			throw new std::exception("invalid min_index:must >= 1");
		if (max_index >= 32)
			throw new std::exception("invalid max_index:must < 32");
	}
	int CalcIndex(uint32_t buffer_size) {
		buffer_size = buffer_size >> min_index_;
		int index = -1;
		auto last_value = buffer_size;
		while (buffer_size != 0) {
			buffer_size >>= 2;
			last_value = buffer_size;
			index++;
		}
		if (last_value == 1) {
			index--;
		}
		return index;
	}
private:
	uint32_t max_index_;
	uint32_t min_index_;
	uint32_t max_buffer_size_;
};