#include "handler.hh"
#include <codecvt>
#include "stdafx.hh"

handler::handler(utility::string_t url, std::unique_ptr<Cache> my_cache):m_listener(url)
{
    m_listener.support(methods::GET, std::bind(&handler::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&handler::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&handler::handle_post, this, std::placeholders::_1));
    m_listener.support(methods::DEL, std::bind(&handler::handle_delete, this, std::placeholders::_1));
    m_listener.support(methods::HEAD, std::bind(&handler::handle_head, this, std::placeholders::_1));
    std::unique_ptr<Cache> ptr;
    this->my_cache_ = move(ptr);
}


handler::~handler()
{
    //dtor                                                                                                                                                            
}

void handler::handle_error(pplx::task<void>& t)
{
    try
    {
        t.get();
    }
    catch(...)
    {
        // Ignore the error, Log it if a logger is available                                                                                                          
    }
}


//                                                                                                                                                                    
// Get Request                                                                                                                                                        
//                                                                                                                                                                    
void handler::handle_get(http_request message)
{

    const auto mes_str = message.request_uri().to_string();
    json::value jsonObject;
    auto const found = mes_str.find('/', 1);
    auto key = mes_str.substr(1, found - 1);
    auto size = mes_str.substr(found + 1, mes_str.length() - found - 1);
    // Had to include HEAD request as a special GET request. Handler for HEAD is not allowed to return user-defined information in REST.                              
        if(key == U("HEAD")&& size == U("HEAD"))
        {
        utility::string_t response = key;

        response.append(" request received, HTTP 1.1, Accept, current space used: ");
        std::string s = std::to_string(my_cache_->space_used());
        jsonObject[U("size")] = json::value::string(s);
        response.append(s);
        response.append("\n");
        jsonObject["response"] = json::value::string(response);

        message.reply(status_codes::OK, jsonObject);
    }
    else {


        const std::string size_str = size;
        auto size_int = stoi(size_str);

        Cache::size_type size_real = size_int;
        auto val = my_cache_->get(key, size_real);
        if (val == nullptr)
        {

            utility::string_t response = key;

     response.append(" not found! \n");

            jsonObject["response"] = json::value::string(response);
            message.reply(status_codes::NotFound, jsonObject);
            return;
        }
        string_t s = "";
        for (auto i = 0; i < size_int; i++)
        {
            s += val[i];
        }

        utility::string_t response = key;

        response.append(" is available! \n");

        jsonObject["response"] = json::value::string(response);
        jsonObject["value"] = json::value::string(s);
        message.reply(status_codes::OK, jsonObject);
        return;
    }


}
//                                                                                                                                                                   
// A POST request                                                                                                                                                     
//                                                                                                                                                                    
void handler::handle_post(http_request message)
{
    const auto mes_str = message.request_uri().to_string();
    json::value jsonObject;
        if(mes_str != "/reset")
        {
        utility::string_t response = " Cannot reset the Cache! \n";
        jsonObject["response"] = json::value::string(response);
        message.reply(status_codes::NotFound, jsonObject);

        }
    else{
        my_cache_->reset();
        utility::string_t response = " Cache reset! \n";
        jsonObject["response"] = json::value::string(response);
        message.reply(status_codes::OK, jsonObject);
        }
}

//
// A DELETE request                                                                                                                                                   
//                                                                                                                                                                    
void handler::handle_delete(http_request message)
{
    const auto mes_str = message.request_uri().to_string();
    json::value jsonObject;
    auto key = mes_str.substr(1, mes_str.length() - 1);
    bool in_cache = my_cache_->del(key);
        if(!in_cache)
        {

        utility::string_t response = key;
        response.append(" is not in the cache! \n");
        jsonObject["response"] = json::value::string(response);
        jsonObject["in_cache"] = json::value::string("0");
        message.reply(status_codes::NotFound, jsonObject);
        }else
        {
        utility::string_t response = key;
        response.append(" is deleted! \n");
        jsonObject["response"] = json::value::string(response);
        jsonObject["in_cache"] = json::value::string("1");
        message.reply(status_codes::OK, jsonObject);
        }
}

//                                                                                                                                                                    
// A PUT request                                                                                                                                                      
//                                                                                                                                                                    
void handler::handle_put(http_request message)
{
    const auto mes_str = message.request_uri().to_string();

    auto const found = mes_str.find('/', 1);
    auto key = mes_str.substr(1, found - 1);
    auto value = mes_str.substr(found + 1, mes_str.length() - found - 1);

    const auto  n = value.length();

    // declaring character array                                                                                                                                      
    auto val = new char[n];
    for (long unsigned int i = 0; i < n; i++)
    {
        val[i] = value[i];
    }

    this->my_cache_->set(key, val, value.length());
    utility::string_t response = key;
    response.append(" inserted/modified! \n");
    json::value jsonObject;
    jsonObject["response"] = json::value::string(response);
        //KNOWN BUG::cannot figure out a way to distinguish insertion and modification based on current cache API. HENCE all GET would return 201 instead of 200.      
    message.reply(status_codes::Created, jsonObject);
    return;
}


void handler::handle_head(http_request message)
{   // Note:: We are not allowed to return a message body with HEAD request in REST sdk, HENCE HEAD REQUEST AS REQUIRED BY THE PROJECT IS IMPLEMENTED WITH GET!!!!!    
    message.reply(status_codes::OK, U("this message cannot reach the client"));

}





