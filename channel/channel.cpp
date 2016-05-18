#include <iostream>
#include <list>
#include <string>
#include <pthread.h>
#include <unistd.h>

template<typename item>
class channel {
public:
	channel(): closed(false) {
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, NULL);
	}

	virtual ~channel() {
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
		queue.clear();
	}

	void close() {
		pthread_mutex_lock(&mutex);
		closed = true;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
	}

	bool is_closed() {
		pthread_mutex_lock(&mutex);
		bool ret = closed;
		pthread_mutex_unlock(&mutex);
		return ret;
	}

	void put(const item &in) {
		pthread_mutex_lock(&mutex);
		if (closed) {
			throw std::string("put to closed channel");
		}
		queue.push_back(in);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}

	bool get(item &out, bool wait = true) {
		pthread_mutex_lock(&mutex);
		while (!closed && queue.empty()) {
			pthread_cond_wait(&cond, &mutex);
		}
		if (queue.empty()) {
			pthread_mutex_unlock(&mutex);
			return false;
		}

		out = queue.front();
		queue.pop_front();
		pthread_mutex_unlock(&mutex);
		return true;
	}

private:
	std::list<item> queue;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool closed;
};


channel<int> c;
int id = 0;

void *produce(void *arg) {
	while (1) {
		c.put(id++);
		sleep(1);
	}
	return 0;
}

void *consume(void *arg) {
	int d;
	while (c.get(d)) {
		std::cout << " " << d << std::endl;
	}
	return 0;
}


int main() {
	pthread_t p1, p2, p3;
	pthread_create(&p1, NULL, produce, NULL);
	pthread_create(&p2, NULL, consume, NULL);
	pthread_create(&p3, NULL, consume, NULL);

	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	pthread_join(p3, NULL);

	return 0;
}