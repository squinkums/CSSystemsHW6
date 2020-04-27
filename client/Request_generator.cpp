#include "Request_generator.h"
#include <iostream>


Request_generator::Request_generator():

    cache(new Cache("http://127.0.0.1:", "34568"))

{
    get_hit = 0;
    get_miss = 0;
}

Request_generator::~Request_generator()
{

}

std::string Request_generator::get_request()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    int precision = 10000;
    std::uniform_int_distribution<> dis(1, precision);
    int rand_num = dis(gen);
	if(rand_num<= precision*get_portion)
	{
        return "GET";
	}
    if(rand_num<= precision * (get_portion+set_portion)){
        return "SET";
    }
    return "DEL";
}

std::string Request_generator::get_random_key()
{
    return std::to_string(rand() % key_pool_size);
}

char* Request_generator::get_random_value()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    int precision = 10000;
    std::uniform_int_distribution<> dis(1, precision);
    int rand_num = dis(gen);
    if (rand_num <= precision * small_value_portion)
    {
        auto const my_value = new char('s');
        return my_value;
    }
    if (rand_num <= precision * (small_value_portion + medium_value_portion)){
        auto const my_value = new char[10];
    	for (auto i = 0;i<10;i++)
    	{
            my_value[i] = 'm';
    	}
        return my_value;
    }
    auto const my_value = new char[100];
    for (auto i = 0; i < 100; i++)
    {
        my_value[i] = 'b';
    }
    return my_value;
}

void Request_generator::generate(int times)
{
    for (auto i = 0; i < times; i++) {
        auto request = get_request();
        if (request == "GET")
        {
            auto key = get_random_key();
            Cache::size_type size = 1;
            auto return_val = cache->get(key, size);
            if (return_val == nullptr)
            {
                this->get_miss++;
            }
            else
            {
                this->get_hit++;
                delete[] return_val;
            }
        }
        else if (request == "SET")
        {
            auto key = get_random_key();
            auto val = get_random_value();
            if (val[0] == 's')
            {
                cache->set(key, val, 1);
            }
            else if (val[0] == 'm')
            {
                cache->set(key, val, 10);
            }
            else
            {
                cache->set(key, val, 100);
            }
            delete[] val;
        }
        else
        {
            auto key = get_random_key();
            cache->del(key);
        }
    }
}

double Request_generator::report_hit_rate()
{
    return get_hit/(get_miss + get_hit);
}
