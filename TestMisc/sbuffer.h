#pragma once
#include <string.h>
#include <unordered_map>
#include <shared_mutex>
#include <thread>

class SBuffer {
public:
	SBuffer(size_t size) {
		ptr_ = new char[size];
		size_ = size;
	}
	~SBuffer() {
		delete[] ptr_;
		ptr_ = nullptr;
		size_ = 0;
	}
	void Resize(size_t newSize) {
		if (newSize <= size_) {
			return;
		}
		auto newPtr = new char[newSize];
		memcpy(newPtr, ptr_, size_);
		delete[] ptr_;
		ptr_ = newPtr;
		size_ = newSize;
	}
	char* Data() {
		return ptr_;
	}
	size_t Size() {
		return size_;
	}
	void Swap(SBuffer* buffer) {
		std::swap(ptr_, buffer->ptr_);
		std::swap(size_, buffer->size_);
	}
private:
	char* ptr_;
	size_t size_;
};

class SBufferMap {
public:
	SBuffer& GetBuffer() {
		auto id = std::this_thread::get_id();
		{
			std::shared_lock<std::shared_mutex> lock(mutex_);
			auto it = bufer_map_.find(id);
			if (it != bufer_map_.end()) {
				return *it->second;
			}
		}
		std::unique_lock<std::shared_mutex> lock(mutex_);
		auto it = bufer_map_.find(id);
		if (it != bufer_map_.end()) {
			return *it->second;
		}
		auto buf = new SBuffer(default_size);
		bufer_map_[id] = buf;
		return *buf;
	}
	~SBufferMap() {
		for (auto it : bufer_map_) {
			delete it.second;
		}
		bufer_map_.clear();
	}
private:
	std::unordered_map<std::thread::id, SBuffer*> bufer_map_;
	std::shared_mutex mutex_;
	size_t default_size = 1024;
};