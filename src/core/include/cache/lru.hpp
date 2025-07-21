#ifndef PLIB_CORE_CACHE_LRU_HPP
#define PLIB_CORE_CACHE_LRU_HPP

#include <list>
#include <unordered_map>

namespace plib::core::cache {

template <typename Entry>
class LRU {
public:
	void up(Entry entry);
	void remove(Entry entry);
	void clear();

	Entry take_lowest();

private:
	std::list<Entry> _queue;
	std::unordered_map<Entry, typename std::list<Entry>::iterator> _map;

};

template <typename Entry>
void LRU<Entry>::up(Entry entry) {
	if (!_queue.empty() && _queue.back() == entry) {
		return;
	}
	const auto i = _map.find(entry);
	if (i != end(_map)) {
		_queue.splice(end(_queue), _queue, i->second);
	} else {
		_map.emplace(entry, _queue.insert(end(_queue), entry));
	}
}

template <typename Entry>
void LRU<Entry>::remove(Entry entry) {
	const auto i = _map.find(entry);
	if (i != end(_map)) {
		_queue.erase(i->second);
		_map.erase(i);
	}
}

template <typename Entry>
void LRU<Entry>::clear() {
	_queue.clear();
	_map.clear();
}

template <typename Entry>
Entry LRU<Entry>::take_lowest() {
	if (_queue.empty()) {
		return Entry();
	}
	auto result = std::move(_queue.front());
	_queue.erase(begin(_queue));
	_map.erase(result);
	return result;
}

} // namespace plib::core::cache

#endif // PLIB_CORE_CACHE_LRU_HPP