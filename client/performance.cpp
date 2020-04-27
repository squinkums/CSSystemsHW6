#include "Request_generator.h"
#include <iostream>
#include <chrono>
#include <numeric>
#include <future>

std::vector<double> baseline_latencies(int nreq, Request_generator* req)
{
	std::vector<double> measures = {};
	for (auto i = 0; i < nreq; i++)
	{
		auto const t1 = std::chrono::high_resolution_clock::now();
		req->generate(1);
		auto const t2 = std::chrono::high_resolution_clock::now();

		//Third Part: compute and output
		double const duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0;
		measures.push_back(duration);
	}
	return measures;
}

std::vector<double> baseline_performance(Request_generator* req)
{
	auto measures  = baseline_latencies(100, req);
	std::sort(measures.begin(), measures.end());
	std::vector<double> param = {};
	param.push_back(measures[94]);
	param.push_back(100000.0 /std::accumulate(measures.begin(), measures.end(), 0.0));
	return param;
}

std::vector<double> threaded_performance(Request_generator* req, int nthreads)
{
	std::vector<std::future<std::vector<double>>> threads = {};
	auto const t1 = std::chrono::high_resolution_clock::now();
	for (auto i = 0; i < nthreads; i++)
	{
		threads.push_back( std::async(baseline_latencies,1000,req));
	}
	for(auto i =0; i < nthreads; i++)
	{
		threads[i].wait();
	}
	auto const t2 = std::chrono::high_resolution_clock::now();

	//Third Part: compute and output
	double const duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0;
	std::vector<double> param = {};
	std::vector<double> aggregated;
	aggregated.reserve(nthreads * 1000);
	for (auto i = 0; i < nthreads; i++)
	{
		auto result = threads[i].get();
		aggregated.insert(aggregated.end(), result.begin(), result.end());
	}
	std::sort(aggregated.begin(), aggregated.end());
	param.push_back(aggregated[950*nthreads]);
	param.push_back( 1000*nthreads/ (duration/1000));
	return param;
}

int main()
{
	auto my_generator = new Request_generator();
	//my_generator->generate(5000);
	auto perform = threaded_performance(my_generator, 14);
	std::cout << perform[0] << '\n' << perform[1];
}

