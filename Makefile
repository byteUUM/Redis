.PHONY:all
all:generic hello string list hash set zset
string:string.cc
	g++ -std=c++17 -o $@ $^ /usr/local/lib/libredis++.a /usr/lib/x86_64-linux-gnu/libhiredis.a -pthread
generic:generic.cc
	g++ -std=c++17 -o $@ $^ /usr/local/lib/libredis++.a /usr/lib/x86_64-linux-gnu/libhiredis.a -pthread
hello:hello.cc
	g++ -std=c++17 -o $@ $^ /usr/local/lib/libredis++.a /usr/lib/x86_64-linux-gnu/libhiredis.a -pthread
list:list.cc
	g++ -std=c++17 -o $@ $^ /usr/local/lib/libredis++.a /usr/lib/x86_64-linux-gnu/libhiredis.a -pthread
hash:hash.cc
	g++ -std=c++17 -o $@ $^ /usr/local/lib/libredis++.a /usr/lib/x86_64-linux-gnu/libhiredis.a -pthread
set:set.cc
	g++ -std=c++17 -o $@ $^ /usr/local/lib/libredis++.a /usr/lib/x86_64-linux-gnu/libhiredis.a -pthread
zset:zset.cc
	g++ -std=c++17 -o $@ $^ /usr/local/lib/libredis++.a /usr/lib/x86_64-linux-gnu/libhiredis.a -pthread
.PHONY:clean
clean:
	rm hello
	rm generic
	rm string
	rm list
	rm hash
	rm set
	rm zset
