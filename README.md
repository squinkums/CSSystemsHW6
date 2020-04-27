
# HW6
Weihang Qin
Liam Goodwin

## Multi-threaded benchmark

The latency per request is increasing steadily as we increase the number of threads pass 3. 

![Histogram of 100 measures](https://github.com/squinkums/CSSystemsHW6/blob/master/latency.png?raw=true)

The throughput, as expected, keeps increasing until the number of threads reaches 8. Then it just keeps fluctuating around 600 requests/sec. The initial boost was impressive without sacrificing any latency performance. 
![Histogram of 100 measures](https://github.com/squinkums/CSSystemsHW6/blob/master//throughput.png?raw=true)


