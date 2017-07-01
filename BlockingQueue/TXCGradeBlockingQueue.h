//
//  TXCGradeBlockingQueue.h
//  TestCpp
//
//  Created by lijie on 2017/7/1.
//  Copyright © 2017年 gansidui. All rights reserved.
//

#ifndef TXCGradeBlockingQueue_h
#define TXCGradeBlockingQueue_h

#include <queue>
#include <mutex>
#include <condition_variable>
#include <assert.h>

// TXCGradeBlockingQueue内部最多拥有_MAX_QUEUE_NUM(10)个队列（编号分别为1, 2, ... _MAX_QUEUE_NUM）
// 在定义TXCGradeBlockingQueue对象时需要指定最大队列数量，并且在push item时需要指定队列编号，
// 每次pop都会从编号为1的队列开始依次读取
// 作用：用于区分Item的优先级，编号为1的队列优先级最高，编号为_MAX_QUEUE_NUM的队列优先级最低
// 只有高优先级的队列为空时，才有机会读取低优先级的队列
template<typename T>
class TXCGradeBlockingQueue {
public:
    TXCGradeBlockingQueue(int max_queue_num): _closed(false), _items_size(0) {
        assert(max_queue_num >= 1 && max_queue_num <= _MAX_QUEUE_NUM);
        _max_queue_num = max_queue_num;
    }
    virtual ~TXCGradeBlockingQueue() { }
    TXCGradeBlockingQueue(const TXCGradeBlockingQueue &rhs) = delete;
    TXCGradeBlockingQueue(const TXCGradeBlockingQueue &&rhs) = delete;
    TXCGradeBlockingQueue& operator = (const TXCGradeBlockingQueue &rhs) = delete;
    TXCGradeBlockingQueue& operator = (const TXCGradeBlockingQueue &&rhs) = delete;
    
    // close后将只能读取数据
    void close() {
        std::lock_guard<std::mutex> lock(_mutex);
        _closed = true;
        _cond.notify_all();
    }
    
    bool is_closed() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _closed;
    }
    
    void push(const T &item, int queue_index) {
        if (queue_index < 1 || queue_index > _max_queue_num) {
            return;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        if (_closed) return;
        _queue[queue_index-1].push(item);
        _items_size ++;
        _cond.notify_one();
    }
    
    void push(T &&item, int queue_index) {
        if (queue_index < 1 || queue_index > _max_queue_num) {
            return;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        if (_closed) return;
        _queue[queue_index-1].push(std::move(item));
        _items_size ++;
        _cond.notify_one();
    }
    
    // 若closed为true, pop将不再阻塞
    // 读取数据成功返回true，否则返回false
    bool pop(T &item) {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock, [this]{return _items_size || _closed;});
        if (_items_size) {
            for (int i = 0; i < _max_queue_num; ++i) {
                if (!_queue[i].empty()) {
                    item = std::move(_queue[i].front());
                    _queue[i].pop();
                    _items_size --;
                    break;
                }
            }
        } else {
            assert(_closed);
            return false;
        }
        return true;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }
    
private:
    static const int        _MAX_QUEUE_NUM = 10;
    mutable std::mutex      _mutex;
    std::condition_variable _cond;
    std::queue<T>           _queue[_MAX_QUEUE_NUM];
    size_t                  _items_size;
    bool                    _closed;
    int                     _max_queue_num;
};


#endif /* TXCGradeBlockingQueue_h */
