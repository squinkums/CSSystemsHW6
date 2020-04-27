#ifndef HANDLER_H
#define HANDLER_H
#include <iostream>
#include "stdafx.hh"
#include <cpprest/asyncrt_utils.h>
#include "cache.hh"

//#include "../dbms/include/Dbms.h"                                                                                                                                    



namespace web {
        namespace http {
                class http_request;
        }
}

using namespace std;
using namespace web;
using namespace http;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using namespace utility;
using namespace web::http::experimental::listener;
using namespace web::json;

class handler
{
    public:
        handler(utility::string_t url,
                std::unique_ptr<Cache> my_cache);
        virtual ~handler();

        pplx::task<void>open(){return m_listener.open();}
        pplx::task<void>close(){return m_listener.close();}


    private:
        void handle_get(http_request message);
        void handle_put(http_request message);
        void handle_post(http_request message);
        void handle_delete(http_request message);
        void handle_error(pplx::task<void>& t);
        void handle_head(http_request message);
        http::experimental::listener::http_listener m_listener;
        std::unique_ptr<Cache> my_cache_;
};

#endif // HANDLER_H
