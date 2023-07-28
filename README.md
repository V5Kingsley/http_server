# http_server http服务器 C++

#编译:
mkdir build
cd build
cmake ..
make
cd .. & cp build/httpserver ./

#运行:
./httpserver


#ab test:
ab -k -r -c1000 -n1000 http://localhost:54321/index.html

ab test sample:

Server Software:        
Server Hostname:        localhost
Server Port:            54321

Document Path:          /index.html
Document Length:        10444 bytes

Concurrency Level:      1000
Time taken for tests:   0.239 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      10542000 bytes
HTML transferred:       10444000 bytes
Requests per second:    4176.81 [#/sec] (mean)
Time per request:       239.417 [ms] (mean)
Time per request:       0.239 [ms] (mean, across all concurrent requests)
Transfer rate:          42999.96 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   15  12.6     16      38
Processing:     5   44  17.6     44     198
Waiting:        5   43  17.5     44     197
Total:          6   59  23.7     70     219

Percentage of the requests served within a certain time (ms)
  50%     70
  66%     73
  75%     75
  80%     76
  90%     79
  95%     81
  98%     82
  99%     84
 100%    219 (longest request)
