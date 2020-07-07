#pragma once
#include <queue>
#include <vector>
#include <mutex>
#include <array>
#include <map>
#include <set>
#include <algorithm>

struct StatInfo {
	int weight = 0;
	int index = 0;
	StatInfo(int w, int i) : weight(w), index(i) {
	}
};
struct StatInfoKey {
	std::vector<std::shared_ptr<StatInfo>>* data;
	int index;
	StatInfoKey(std::vector<std::shared_ptr<StatInfo>>* dataArg, int i) : data(dataArg), index(i) {
	}
};
typedef StatInfoKey* StatInfoKeyPtr;
struct StatInfoCompare
{
	bool operator() (const StatInfoKeyPtr& p1, const StatInfoKeyPtr& p2) const
	{
		auto& info1 = (*(p1->data))[p1->index];
		auto& info2 = (*(p2->data))[p2->index];
		if (info1->weight != info2->weight) {
			return info1->weight < info2->weight;
		}
		return info1->index < info2->index;
	}
};
class StatBalance {
public:
	StatBalance(size_t size) : defaultKey(&stat_list_, 0){
		stat_list_.reserve(size);
		stat_key_holder_.reserve(size);
		for (size_t i = 0; i < size; i++) {
			stat_list_.emplace_back(std::make_shared<StatInfo>(0, (int)i));

			stat_key_holder_.emplace_back(std::move(StatInfoKey(&stat_list_, int(i))));
			stat_set_.insert(&stat_key_holder_[i]);
		}
	}
	int AllocIndex(int weight) {
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = stat_set_.begin();
		auto ptr = *it;
		auto index = ptr->index;
		stat_list_[index]->weight += weight;
		stat_set_.erase(it);
		stat_set_.insert(ptr);
		return index;
	}
	void FreeIndex(int index, int weight) {
		std::lock_guard<std::mutex> lock(mutex_);
		defaultKey.index = index;
		auto it = stat_set_.find(&defaultKey);
		if (it != stat_set_.end()) {
			auto ptr = *it;
			stat_list_[ptr->index]->weight -= weight;
			stat_set_.erase(it);
			stat_set_.insert(ptr);
		}
	}
private:
	std::vector<std::shared_ptr<StatInfo>> stat_list_;
	std::vector<StatInfoKey> stat_key_holder_;
	std::set<StatInfoKeyPtr, StatInfoCompare> stat_set_;
	StatInfoKey defaultKey;
	std::mutex mutex_;
};


template <class T>
class BalanceContextPool
{
public:
	BalanceContextPool(size_t n) : balance_(n) {
		balance_context_list_.reserve(n);
		for (size_t i = 0; i < n; i++) {
			balance_context_list_.emplace_back(std::make_shared<T>());
		}
	}
	std::pair<std::shared_ptr<T>, int> AllocContext(int weight) {
		auto index = balance_.AllocIndex(weight);
		return std::move(std::make_pair(balance_context_list_[index], index));
	}
	const std::vector<std::shared_ptr<T>>& GetContextList() {
		return balance_context_list_;
	}
private:
	std::vector<std::shared_ptr<T>> balance_context_list_;
	StatBalance balance_;
};
