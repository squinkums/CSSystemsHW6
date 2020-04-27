#include "Request_generator.h"
#include <iostream>
#include <chrono>
#include <numeric>
#include <algorithm>

std::vector<double> baseline_latencies(int nreq, Request_generator* req)
{
	std::vector<double> measures = {};
	for (auto i = 0; i < nreq; i++)
	{
		auto const t1 = std::chrono::high_resolution_clock::now();
		req->generate(1);
		auto const t2 = std::chrono::high_resolution_clock::now();

		//Third Part: compute and output
		double const duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000;
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

int main()
{
	auto my_generator = new Request_generator();
	my_generator->generate(50000);
	auto perform = baseline_performance(my_generator);
	std::cout << perform[0] << '\n' << perform[1];
}

