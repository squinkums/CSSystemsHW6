#include "fifo_evictor.hh"
#include <algorithm>

Fifo_evictor::Fifo_evictor():
	key_queue_(new std::deque<key_type>())
	{}


Fifo_evictor::~Fifo_evictor() = default;

void Fifo_evictor::touch_key(const key_type& key )
{
	// If the key is already in the queue, delete it.
	auto it = std::find(key_queue_->begin(), key_queue_->end(), key);
	if(it != key_queue_->end())
	{
		it = key_queue_->erase(it);
	}
	// Push the key to the end of the queue.
	key_queue_->push_back(key);
}
const key_type Fifo_evictor::evict()
{

	if(key_queue_->empty())
	{
		return "";
	}
	// Get a copy of the evicted key
	auto const next = key_queue_->front();
	// Pop the key in the queue
	key_queue_->pop_front();
	return next;
	
}
