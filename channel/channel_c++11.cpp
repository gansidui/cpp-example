#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <windows.h>

template<class item>
class channel {
private:
  std::list<item> queue;
  std::mutex m;
  std::condition_variable cv;
  bool closed;
public:
  channel() : closed(false) { }
  
  void close() {
    std::unique_lock<std::mutex> lock(m);
    closed = true;
    cv.notify_all();
  }
  
  bool is_closed() {
    std::unique_lock<std::mutex> lock(m);
    return closed;
  }
  
  void put(const item &i) {
    std::unique_lock<std::mutex> lock(m);
    if(closed)
      throw std::logic_error("put to closed channel");
    queue.push_back(i);
    cv.notify_one();
  }
  
  bool get(item &out, bool wait = true) {
    std::unique_lock<std::mutex> lock(m);
    if(wait)
      cv.wait(lock, [&](){ return closed || !queue.empty(); });
    if(queue.empty())
      return false;
    out = queue.front();
    queue.pop_front();
    return true;
  }
};

channel<int> c;
int id = 0;
	
void f() {
	while (1) {
		c.put(id++);
		Sleep(1000);
	}
}

void g(std::string s) {
	int d;
	while (c.get(d)) {
		std::cout << s << "  " << d << std::endl;
	}
}

int main() {

	std::thread produce(f);
	std::thread consume1(g, "consume1");
	std::thread consume2(g, "consume2");

	Sleep(100000);

	return 0;
}
