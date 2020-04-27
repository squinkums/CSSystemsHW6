#pragma once
#include <string>
#include <random>
#include "cache.hh"

struct Request_generator
{
	Request_generator();
	~Request_generator();
	std::string get_request();
	std::string get_random_key();
	char* get_random_value();
	void generate(int times);
	double report_hit_rate();
private:
	const float get_portion = 0.9;
	const float del_portion = 0.01;
	float set_portion = 1 - get_portion - del_portion;
	int key_pool_size = 1000;
	const float small_value_portion = 0.9;
	const float medium_value_portion = 0.09;
	const float big_value_portion = 0.01;
	std::unique_ptr<Cache> cache;
	float get_hit;
	float get_miss;
	
};
