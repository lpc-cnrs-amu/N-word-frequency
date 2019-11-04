#ifndef QUEUE_SAFE_HPP
#define QUEUE_SAFE_HPP

#include <mutex>
#include <queue>

template<typename T>

class QueueSafe
{
	private:
		std::queue<T> _queue;
		std::mutex _mutex;
	public:
		void push_front(T const& data){ _queue.push(data); }

		bool empty()
		{
			std::lock_guard<std::mutex> guard(_mutex);
			return _queue.empty();
		}

		bool try_pop(T& popped_value)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			if(_queue.empty())
				return false;
			
			popped_value = _queue.front();
			_queue.pop();
			return true;
		}
		
		void print_items()
		{
			std::cout << "Items in queue : \n";
			for(unsigned i=0; i<_queue.size(); ++i)
				std::cout << _queue[i] << std::endl;
		}
};

#endif
