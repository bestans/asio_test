#pragma once
#include <memory>
#include <mutex>
#include <functional>

class BufferN {
public:
	BufferN(size_t size) {
		ptr_ = new char[size];
		size_ = size;
	}
	~BufferN() {
		delete ptr_;
		ptr_ = nullptr;
		size_ = 0;
	}
private:
	char* ptr_;
	size_t size_;
};

class NetBufferHold {
public:
	NetBufferHold(std::shared_ptr<BufferN> buffer, std::mutex& mutex) : mutex_(mutex_) {
		mutex_.lock();
		buffer_ = std::move(buffer);
	}
	~NetBufferHold() {
		mutex_.unlock();
	}
	std::shared_ptr<BufferN>& GetBuffer() {
		return buffer_;
	}
private:
	std::shared_ptr<BufferN> buffer_;
	std::mutex& mutex_;
};
typedef std::function<void()> WriteFinishFunc;
typedef std::function<void(std::shared_ptr<BufferN>, WriteFinishFunc)> WriteFunc;
class NetBuffer {
public:
	NetBuffer(size_t size, size_t mini_size) : mini_size_(mini_size){
		write_buffer_ = std::make_shared<BufferN>(size);
		write_buffer_cache_ = std::make_shared<BufferN>(size);
		read_buffer_ = std::make_shared<BufferN>(size);
	}
	BufferN* GetMiniBuffer() {
		thread_local static BufferN g_thread_buffer(mini_size_);
		return &g_thread_buffer;
	}
	size_t GetMiniBufferSize() {
		return mini_size_;
	}

	NetBufferHold GetWriteBuffer(int maxSize) {
		return std::move(NetBufferHold(write_buffer_cache_, write_buffer_cache_mutex_));
	}
	
private:
	std::shared_ptr<BufferN> write_buffer_;
	std::shared_ptr<BufferN> write_buffer_cache_;
	std::mutex write_buffer_cache_mutex_;
	std::shared_ptr<BufferN> read_buffer_;
	size_t mini_size_;
	WriteFunc write_func_;
};