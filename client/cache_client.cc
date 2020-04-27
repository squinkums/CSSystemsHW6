#include "cache.hh"
#include <unordered_map>
#include "null_evictor.hh"
#include <iostream>
#include <algorithm>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <codecvt>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;


//------------------------------------------------------------------This is the Impl Class Implementation------------------------------------------


class Cache::Impl
{
public:
    using byte_type = Cache::byte_type;
    using val_type = Cache::val_type;   // Values for K-V pairs
    using size_type = Cache::size_type;         // Internal indexing to K-V elements
    // A function that takes a key and returns an index to the internal data
    using hash_func = Cache::hash_func;
    // Create a new impl object with the following parameters:
    // maxmem: The maximum allowance for storage used by values.
    // max_load_factor: Maximum allowed ratio between buckets and table rows.
    // evictor: Eviction policy implementation (if nullptr, no evictions occur
    // and new insertions fail after maxmem has been exceeded).
    // hasher: Hash function to use on the keys. Defaults to C++'s std::hash.

    Impl(std::string host, std::string port)
    {
 

        //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)

        this->address = host;
        this->address.append(port);

    }
    //The main thing to deal with is to clear all the data from hashtable. We don't have chained deletion for Link_list, so we need to go
    //deep into each entry to clear things up.
    ~Impl()
    {

    }

    //Not copyable
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    // Add a <key, value> pair to the cache.
// If key already exists, it will overwrite the old value.
// Both the key and the value are to be deep-copied (not just pointer copied).
// If maxmem capacity is exceeded, enough values will be removed
// from the cache to accomodate the new value. If unable, the new value
// isn't inserted to the cache.
    void set(key_type& key, const val_type val, const size_type size)
    {


        http_client client(address);

        // Build request URI and start the request.
    	
        uri_builder builder("/"+key);
        std::string s = "/";
    	for(unsigned int i = 0;i<size;i++)
    	{
            s = s + val[i];
    	}
        builder.append(s);

        client.request(methods::PUT, builder.to_string())



            .then([](web::http::http_response response)
                {
                    return response.extract_json();


                })


            .then([](web::json::value content)
                {

                    std::cout << content["response"].as_string() << std::endl;
                })


                    .wait();




    }
    // Retrieve a pointer to the value associated with key in the cache,
    // or nullptr if not found.
    // Sets the actual size of the returned value (in bytes) in val_size.
    val_type get(key_type& key, size_type& val_size) 
    {
        
        http_client client(address);
        //BUG:: I will pass the val_size as well even though the API does not require so. Otherwise the APIs' does not match. 
        // Build request URI and start the request.

        uri_builder builder("/" + key);
        std::string s = "/";
        s = s+std::to_string(val_size);
        builder.append(s);
        std::string val;
        bool in_cache;
        client.request(methods::GET, builder.to_string())



            .then([&](web::http::http_response response)
                {
                    if (response.status_code() == status_codes::OK) {
                        in_cache = true;
                        return response.extract_json();

                    }else
                    {
                        in_cache = false;
                        return response.extract_json();
                    }

                })


            .then([&](web::json::value content)
                {

                    std::cout << content["response"].as_string() << std::endl;
                    if (in_cache) {
                        val = content[U("value")].as_string();
                    }
        			
                })


                    .wait();

                if (in_cache) {
                    auto res = new char[val.length()];
                    for (long unsigned int i = 0; i < val.length(); i++)
                    {
                        res[i] = val[i];
                    }
                    return res;
                }else
                {
                    return nullptr;
                }
    	
 
    }

    // Delete an object from the cache, if it's still there. 
    bool del(const key_type& key)
    {

        http_client client(address);
        string_t in_cache;
        uri_builder builder("/" + key);
        client.request(methods::DEL, builder.to_string())



            .then([&](web::http::http_response response)
                {

                        return response.extract_json();

                })


            .then([&](web::json::value content)
                {

                    std::cout << content["response"].as_string() << std::endl;
                    in_cache = content["in_cache"].as_string();

                })


                    .wait();
    	if(in_cache == "0")
    	{
            return false;
    	}
        return true;

    }

    // Compute the total amount of memory used up by all cache values (not keys)
    size_type space_used() const
    {
    	//Note we are using $GET /HEAD/HEAD$ instead of the actual HEAD request
        http_client client(address);
        auto builder = "/HEAD/HEAD";
        std::string res;

        client.request(methods::GET, builder)

            .then([&](web::http::http_response response)
                {
                        return response.extract_json();
                })


            .then([&](web::json::value content)
                {

                    std::cout << content["response"].as_string() << std::endl;
                    res = content["size"].as_string();

                })


                    .wait();
                return stoi(res);
    }

    // Delete all data from the cache
    void reset()
    {
        http_client client(address);
        string_t message = "/reset";

        client.request(methods::POST,message)

            .then([&](web::http::http_response response)
                {
                    return response.extract_json();
                })


            .then([&](web::json::value content)
                {

                    std::cout << content["response"].as_string() << std::endl;

                })


                    .wait();
    }



private:
    utility::string_t address;


   
};







//------------------- This is the Class Cache Implementation-------------------------------------------------------










//Cache::Cache(size_type maxmem,
//	     float max_load_factor,
//	     Evictor* evictor,
//	     hash_func hasher)

   
//{
	
//}


Cache::~Cache() = default;

// Add a <key, value> pair to the cache.
// If key already exists, it will overwrite the old value.
// 
// Both the key and the value are to be deep-copied (not just pointer copied).
// If maxmem capacity is exceeded, enough values will be removed
// from the cache to accomodate the new value. If unable, the new value
// isn't inserted to the cache.
void Cache::set(key_type key, val_type val, size_type size)
{
    pImpl_->set(key, val, size);

}

// Retrieve a pointer to the value associated with key in the cache,
// or nullptr if not found.
// Sets the actual size of the returned value (in bytes) in val_size.
Cache::val_type Cache::get(key_type key, size_type & val_size) const
{

    return pImpl_->get(key, val_size);

}

// Delete an object from the cache, if it's still there
bool Cache::del(key_type key)
{
    return pImpl_->del(key);

}

// Compute the total amount of memory used up by all cache values (not keys)
Cache::size_type Cache::space_used() const
{

    return pImpl_->space_used();
}

// Delete all data from the cache
void Cache::reset()
{
    pImpl_->reset();

}

Cache::Cache(std::string host, std::string port) :

    pImpl_(new Impl(host, port))
{}
