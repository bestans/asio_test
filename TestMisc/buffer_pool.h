#pragma once
#include <mutex>
#include <vector>
#include <queue>
#include <memory>
#include <list>
#include <functional>

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
	SimpleStack(size_t capacity, CreateFunc create_func) :
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
	size_t capacity_;
	size_t len_;
	std::vector<std::unique_ptr<T>> data_list_;
	CreateFunc create_func_;
};

class SectionBuffer {
public:
	SectionBuffer(size_t max_count, size_t buffer_size) :
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
	size_t buffer_size_;
	size_t max_count_;
	SimpleStack<ByteBuf> data_list_;
};
class BufferPool {
	BufferPool(size_t max_memory) {

	}
	int CalcIndex(size_t buffer_size) {
		buffer_size = buffer_size >> start_index_;
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
	size_t start_index_;
	size_t start_index_size_;
	size_t max_buffer_size_;
};