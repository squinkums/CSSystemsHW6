#include "cache.hh"
#include <unordered_map>
#include "null_evictor.hh"
#include <iostream>
#include <algorithm>
#include "fifo_evictor.hh"

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
    Impl(size_type maxmem,
        float max_load_factor = 0.75,
        Evictor* evictor = nullptr,
        hash_func hasher = std::hash<key_type>())
    {
        this->maxmem_ = maxmem;
        this->hasher_ = hasher;
    	//If the caller does not give any evictor, we create a new null-evictor;
        if(evictor == nullptr){
            this->evictor_ = new Null_evictor();
        }else
        {
            this->evictor_ = evictor;
        }
        this->max_load_factor_ = max_load_factor;
    	//initialize the hashtable with all null pointers
    	for(unsigned int i = 0;i<cur_table_size_;i++)
    	{
            hashtable.push_back(nullptr);
    	}


    }
	//The main thing to deal with is to clear all the data from hashtable. We don't have chained deletion for Link_list, so we need to go
	//deep into each entry to clear things up.
    ~Impl()
    {
        for(auto i: hashtable)
        {
            if (i!= nullptr) {
                while (i->next != nullptr)
                {
                    const auto j = i;
                    i = i->next;
                    delete j;
                }
                //delete nullptr has no effect.
                delete i;
            }
        }
        delete evictor_;
        hashtable.clear();
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
    void set(key_type &key, const val_type val, const size_type size,const bool reinsert = false)
    {
    	// If the hashtable is too crowded, resize it
    	if(loaded_pairs+1>max_load_factor_*cur_table_size_)
    	{
            resize_hashtable(3);
            std::cout << "Hashtable Resized! Current table size is "<<this->get_tablesize()<<'\n';
    	}

        evictor_->touch_key(key);
        auto const hash_value = hasher_(key);
        auto const table_entry = hash_value % cur_table_size_;
        auto cur = hashtable[table_entry];
        //If the table entry is unfilled, the key is not in the cache. Then we check for max memory and evict if necessary.
        if (cur == nullptr)
        {
        	// Since they key is not in the cache, we check for memory capacity. If exceeded call the evictor.
            if (curmem_ + size > maxmem_) {
                auto const evicted = evictor_->evict();
                //If the evictor returns an empty string, we do nothing
                if (evicted.empty())
                {
                    std::cout << "Warning, unable to insert " << key << '\n';
                    return;
                }
                else {
                    //If the evictor returns a key, we delete it and subtract from cur_mem and cur_pair
                    del(evicted);
                    std::cout << evicted << " evicted!" << '\n';
                    hashtable[table_entry] = new Link_list(key, val, size);
                    if (reinsert == false) {
                        std::cout << key << " inserted, max memory not exceeded." << '\n';
                    }
                    else
                    {
                        std::cout << key << " reinserted in the process of resizing hashtable" << '\n';
                    }
                    loaded_pairs += 1;
                    curmem_ += size;
                }
            }
            else{
                // If capacity not exceeded, simply create a new link_list.
                hashtable[table_entry] = new Link_list(key, val, size);
                if (reinsert == false) {
                    std::cout << key << " inserted, max memory not exceeded." << '\n';
                }
                else
                {
                    std::cout << key << " reinserted in the process of resizing hashtable" << '\n';
                }
                loaded_pairs += 1;
                curmem_ += size;
                return;
            }

        }
        else
        {
            // If the table entry is already occupied, see if the key is already in the cache.
            while (*cur->key != key)
            {
                //If we reach the end of the list without seeing a match, the key is not in the cache. Then we check for max memory and evict if necessary.
                if (cur->next == nullptr)
                {
                    if (curmem_ + size > maxmem_) {
                        auto const evicted = evictor_->evict();
                        //If the evictor returns an empty string, we do nothing
                        if (evicted.empty())
                        {
                            std::cout << "Warning, unable to insert " << key << '\n';
                            return;
                        }
                        else {
                            //If the evictor returns a key, we delete it and subtract from cur_mem and cur_pair
                            del(evicted);
                            std::cout << evicted << " evicted!" << '\n';
                            cur->next = new Link_list(key, val, size);
                            if (reinsert == false) {
                                std::cout << key << " inserted, max memory not exceeded. Collision handled." << '\n';
                            }
                            else
                            {
                                std::cout << key << " reinserted in the process of resizing hashtable.Collision handled." << '\n';
                            }
                            loaded_pairs += 1;
                            curmem_ += size;
                            return;
                        }
                    }else
                    {
	                    //If max memory not exceeded and key is not in the cache, attach it to the end of current link_list
                        cur->next = new Link_list(key, val, size);
                        if (reinsert == false) {
                            std::cout << key << " inserted, max memory not exceeded. Collision handled." << '\n';
                        }
                        else
                        {
                            std::cout << key << " reinserted in the process of resizing hashtable.Collision handled." << '\n';
                        }
                        loaded_pairs += 1;
                        curmem_ += size;
                        return;
                    }
                }
                cur = cur->next;
            }
            // If we ever jump out of the loop we have found the key, then copy the new value and replace the old one.

            delete[] cur->val;
            cur->val = new byte_type[size];
            for (unsigned int i = 0; i < size; i++)
            {
                cur->val[i] = 0;
            }
            std::copy(val, val + size * sizeof(byte_type), cur->val);
            if (reinsert == false) {
                std::cout << key << " modified, max memory not exceeded." << '\n';
            }
            else
            {
                std::cout << key << " reinserted in the process of resizing hashtable" << '\n';
            }
        }


    	
    	
       

    	
    }
    // Retrieve a pointer to the value associated with key in the cache,
	// or nullptr if not found.
	// Sets the actual size of the returned value (in bytes) in val_size.
    val_type get(key_type &key, size_type& val_size) const
    {
        auto const hash_value = hasher_(key);
        auto const table_entry = hash_value % cur_table_size_;
        auto cur = hashtable[table_entry];
    	// Look into that entry, if it's empty, the key is not there
    	if(cur == nullptr)
    	{
            return nullptr;
    	}
        while (*cur->key != key)
        {
        	// IF we reach the end, the key is not in the list
	        if(cur->next == nullptr)
	        {
                return nullptr;
	        }
            cur = cur->next;
        }
    	// If we ever jump out of the loop we have found the key
        evictor_->touch_key(key);
        auto const return_val = new byte_type[val_size];
        for (unsigned int i = 0; i < val_size; i++)
        {
            return_val[i] = 0;
        }
        std::copy(cur->val, cur->val + val_size * sizeof(byte_type), return_val);
        return return_val;
    }

    // Delete an object from the cache, if it's still there and return the value size. If returns zero, it means deletion failed.
    size_type del(const key_type &key)
    {
        size_type val_size = 0;
        auto const hash_value = hasher_(key);
        auto const table_entry = hash_value % cur_table_size_;
        auto cur = hashtable[table_entry];
        //If there's no pair in the entry, return false
        if (cur == nullptr)
        {
            return 0;
        }
        //If there's only one element, check if it's key and delete it if yes.
        else if (cur->next == nullptr) {
            if (*cur->key == key)
            {
                val_size = cur->size;
                std::cout << key << " deleted! " << '\n';
                delete cur;
                hashtable[table_entry] = nullptr;
                curmem_ -= val_size;
                loaded_pairs -= 1;
                return val_size;

            }
            else
            {
                return 0;
            }
        }
    	// If there are two or more pairs stored in the entry;
        else {
        	//Check the first entry;
        	if(*cur->key == key)
        	{
                val_size = cur->size;
                hashtable[table_entry] = cur->next;
                std::cout << key << " deleted! " << '\n';
                delete cur;
                curmem_ -= val_size;
                loaded_pairs -= 1;
                return val_size;
        	}
            auto pre = cur;
            cur = cur->next;
        	//check until we get a match, (but cur is still the link_list before the match so that we could relink the list)
            while (*cur->key != key)
            {
                if(cur->next == nullptr)
                {
                    return 0;
                }
                pre = cur;
                cur = cur->next;
            }
        	//If we ever jump out the loop without returning we get a match
            std::cout << key << " deleted! " << '\n';
            val_size = cur->size;
            pre->next = cur->next;
            curmem_ -= val_size;
            loaded_pairs -= 1;
            return val_size;
        	
        }


    }

    // Compute the total amount of memory used up by all cache values (not keys)
    size_type space_used() const
    {
        return curmem_;
    }

    // Delete all data from the cache
    void reset()
    {
	    for(auto i:hashtable)
	    {
            if (i != nullptr) {

                while (i->next != nullptr)
                {
                    auto j = i;
                    i = i->next;
                    delete j;
                }
            }
            //delete nullptr has no effect.
            delete i;
            // Note: Setting i = nullptr would not reset the table entries.
	    }

    	for(unsigned int i = 0;i<hashtable.size();i++)
    	{
            hashtable[i] = nullptr;
    	}
        curmem_ = 0;
        loaded_pairs = 0;
    	

    }

	int  get_tablesize() const
    {
        return hashtable.size();
    }


private:
	
	struct Link_list
	{
        key_type* key;
		//The cache can't tell the input value type from just a stream of characters. 
        char* val;
        size_type size;
        Link_list* next;
		Link_list(key_type &key, val_type val, size_type size ,Link_list* next = nullptr)
		{
			//This is where things get deep-copied. For std::string things automatically get deep-copied.
            this->key = new key_type(key);
			//To deep copy the value, initialize the val parameter with the same size as the input value
            this->val = new byte_type[size];
			for (unsigned int i = 0;i<size;i++)
			{
                this->val[i] = 0;
			}
			//Copy the input to val.
            std::copy(val,val+size*sizeof(byte_type), this->val);
			// Point to the next key-val pair.
            this->next = next;
            this->size = size;
		}
        //Our destructor does not delete the subsequent lists.
		~Link_list(){
            delete[] val;
            delete key;
		}
        Link_list(const Link_list&) = delete;  // noncopiable
        Link_list& operator=(const Link_list&) = delete;
	};
    size_type maxmem_;
    size_type curmem_ = 0;
    hash_func hasher_;
    Evictor* evictor_;
    float max_load_factor_;
    // We want a relatively small hash table to start with. We will resize it as our value increases. We do not set a bound
    // for the table size because it will be implicitly controlled by the max memory.
    size_type cur_table_size_ = 100;
    size_type loaded_pairs = 0;
    std::vector<Link_list*> hashtable;

	
    // The function multiplied the hashtable size by multiplier and reinsert all the key-value pairs.
	void resize_hashtable(int multiplier)
	{
		
        this->cur_table_size_ *= multiplier;
		// Remap all the key-value pairs.
        std::vector<Link_list*> temp;
        for (size_type i = 0; i < cur_table_size_; i++)
        {
            temp.push_back(nullptr);
        }
        auto copy = hashtable;
        hashtable = temp;
        curmem_ = 0;
        loaded_pairs = 0;
		for(auto i:copy)
		{
			while(i != nullptr)
				{
                    this->set(*i->key, i->val, i->size,true);
                    const auto j = i;
                    i = i->next;
                    delete j;
				}
			
		}
        copy.clear();
        
	}
    
};







//------------------- This is the Class Cache Implementation-------------------------------------------------------










    Cache::Cache(size_type maxmem,
        float max_load_factor,
        Evictor* evictor,
        hash_func hasher):
    
        pImpl_(new Impl(maxmem, max_load_factor,evictor,hasher))
    	{}
    

    Cache::~Cache() = default;

    // Add a <key, value> pair to the cache.
    // If key already exists, it will overwrite the old value.
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
    Cache::val_type Cache::get(key_type key, size_type& val_size) const
    {

        return pImpl_->get(key, val_size);
    	
    }

    // Delete an object from the cache, if it's still there
    bool Cache::del(key_type key)
    {
        size_type size =  pImpl_->del(key);
    	if(size == 0)
    	{
            return false;
    	}
        return true;
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

//Cache::Cache(std::string host, std::string port)
//{}
