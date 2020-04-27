#pragma once
#include "evictor.hh"
#include <queue>
#include <memory>

class Fifo_evictor : public Evictor
{
public:
	Fifo_evictor();
	~Fifo_evictor();
	void touch_key(const key_type& key);
	const key_type evict();

private:
	// Can't use a queue structure because we will remove elements in the middle when a key is touched. In other words, we will not allow multiple copies of a key in the queue.
	std::unique_ptr<std::deque<key_type>> key_queue_;
	
};
