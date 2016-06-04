#include <iostream>
#include <list>
#include <string>
#include <pthread.h>
#include <unistd.h>


template<typename item>
class block_list {
public:
    typedef void (*ItemCleanFunc)(item &it);
    typedef typename std::list<item>::iterator Iterator;
    
	block_list(): closed(false), cleanFunc(NULL) {
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, NULL);
	}

	virtual ~block_list() {
        reset();
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
	}
    
    void setCleanFunc(ItemCleanFunc _cleanFunc) {
        cleanFunc = _cleanFunc;
    }
    
    void reset() {
        close();
        clear();
    }
    
    void clear() {
        pthread_mutex_lock(&mutex);
        for (Iterator it = itemList.begin(); it != itemList.end(); it++) {
            if (cleanFunc) {
                cleanFunc(*it);
            }
        }
        itemList.clear();
        pthread_mutex_unlock(&mutex);
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
    
    bool push_back(const item &in) {
        pthread_mutex_lock(&mutex);
        if (closed) {
            pthread_mutex_unlock(&mutex);
            return false;
        }
        itemList.push_back(in);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        return true;
    }
    
    bool push_front(const item &in) {
        pthread_mutex_lock(&mutex);
        if (closed) {
            pthread_mutex_unlock(&mutex);
            return false;
        }
        itemList.push_front(in);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        return true;
    }
    
    bool pop_front() {
        bool ret = false;
        pthread_mutex_lock(&mutex);
        if (!itemList.empty()) {
            itemList.pop_front();
            ret = true;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }
    
    bool pop_back() {
        bool ret = false;
        pthread_mutex_lock(&mutex);
        if (!itemList.empty()) {
            itemList.pop_back();
            ret = true;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }
    
    bool front(item &out, bool del = false, bool wait = true) {
        pthread_mutex_lock(&mutex);
        if (wait) {
            while (!closed && itemList.empty()) {
                pthread_cond_wait(&cond, &mutex);
            }
        }
        if (itemList.empty()) {
            pthread_mutex_unlock(&mutex);
            return false;
        }
        out = itemList.front();
        if (del) {
            itemList.pop_front();
        }
        pthread_mutex_unlock(&mutex);
        return true;
    }
    
    bool back(item &out, bool del = false, bool wait = true) {
        pthread_mutex_lock(&mutex);
        if (wait) {
            while (!closed && itemList.empty()) {
                pthread_cond_wait(&cond, &mutex);
            }
        }
        if (itemList.empty()) {
            pthread_mutex_unlock(&mutex);
            return false;
        }
        out = itemList.back();
        if (del) {
            itemList.pop_back();
        }
        pthread_mutex_unlock(&mutex);
        return true;    }
    
    size_t size() {
        pthread_mutex_lock(&mutex);
        size_t size = itemList.size();
        pthread_mutex_unlock(&mutex);
        return size;
    }
    
private:
	std::list<item> itemList;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
    ItemCleanFunc cleanFunc;
    bool closed;
};


block_list<int> c;
int id = 0;
void *produce(void *arg) {
	while (1) {
		c.push_back(id++);
		sleep(1);
	}
	return 0;
}

void *consume(void *arg) {
	int d;
	while (c.front(d, true)) {
		std::cout << " " << d << std::endl;
	}
	return 0;
}

void test1() {
    pthread_t p1, p2, p3;
    pthread_create(&p1, NULL, produce, NULL);
    pthread_create(&p2, NULL, consume, NULL);
    pthread_create(&p3, NULL, consume, NULL);
    
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
}


typedef struct Node {
    int v;
    Node(int v): v(v) { std::cout << "construct: " << v << std::endl; }
    ~Node() { std::cout << "destruct: " << v << std::endl; }
} Node;

void clean(Node* &n) {
    std::cout << "clean: " << n->v << std::endl;
}

void test2() {
    block_list<Node*> list;
    
    list.setCleanFunc(clean);
    
    list.push_back(new Node(1));
    list.push_back(new Node(2));
    list.push_back(new Node(3));

    Node *b = NULL;
    bool ret = list.front(b);
    std::cout << ret << " " << b->v << std::endl;
    
    ret = list.back(b);
    std::cout << ret << " " << b->v << std::endl;
    
    list.clear();
}

int main() {
    
    test2();
    
	return 0;
}