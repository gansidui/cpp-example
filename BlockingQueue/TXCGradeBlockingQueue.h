//
//  TXCGradeBlockingQueue.h
//  TestCpp
//
//  Created by lijie on 2017/7/1.
//  Copyright © 2017年 gansidui. All rights reserved.
//

#ifndef TXCGradeBlockingQueue_h
#define TXCGradeBlockingQueue_h

#include <list>
#include <mutex>
#include <condition_variable>
#include <assert.h>

// TXCGradeBlockingQueue内部最多拥有_MAX_QUEUE_NUM(10)个队列（编号分别为1, 2, ... _MAX_QUEUE_NUM）
// 在定义TXCGradeBlockingQueue对象时需要指定最大队列数量，并且在push item时需要指定队列编号，
// 每次pop都会从编号为1的队列开始依次读取
// 作用：用于区分Item的优先级，编号为1的队列优先级最高，编号为_MAX_QUEUE_NUM的队列优先级最低
// 只有高优先级的队列为空时，才有机会读取低优先级的队列
//
// 注意：如果T的类型是普通指针，需要调用close，再通过pop遍历来delete；如果是对象或者智能指针，则调用clear即可释放内存
//
template<typename T>
class TXCGradeBlockingQueue {
public:
    TXCGradeBlockingQueue(): _closed(false), _items_size(0) {
        _max_queue_num = 1;
    }
    explicit TXCGradeBlockingQueue(int max_queue_num): _closed(false), _items_size(0) {
        assert(max_queue_num >= 1 && max_queue_num <= _MAX_QUEUE_NUM);
        _max_queue_num = max_queue_num;
    }
    virtual ~TXCGradeBlockingQueue() { }
    TXCGradeBlockingQueue(const TXCGradeBlockingQueue &rhs) = delete;
    TXCGradeBlockingQueue(TXCGradeBlockingQueue &&rhs) = delete;
    TXCGradeBlockingQueue& operator = (const TXCGradeBlockingQueue &rhs) = delete;
    TXCGradeBlockingQueue& operator = (TXCGradeBlockingQueue &&rhs) = delete;
    
    void setMaxGrade(int max_grade) {
        std::lock_guard<std::mutex> lock(_mutex);
        _max_queue_num = max_grade;
    }
    
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
    
    void reuse() {
        std::lock_guard<std::mutex> lock(_mutex);
        _closed = false;
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _closed = true;
        _items_size = 0;
        _cond.notify_all();
        
        for (int i = 0; i < _MAX_QUEUE_NUM; ++i) {
            while (!_queue[i].empty()) {
                _queue[i].pop_front();
            }
        }
    }
    
    template <typename TT>
    bool push(TT &&item, int queue_index) {
        if (queue_index < 1 || queue_index > _max_queue_num) {
            return false;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        if (_closed) return false;
        _queue[queue_index-1].emplace_back(std::forward<TT>(item));
        _items_size ++;
        _cond.notify_one();
        return true;
    }
    
    // 若closed为true, pop将不再阻塞
    // 读取数据成功返回true，否则返回false
    // timeout单位为毫秒, -1表示不设置超时
    bool pop(T &item, int timeout = -1) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (-1 == timeout) {
            _cond.wait(lock, [this]{return _items_size || _closed;});
        } else {
            if (!_cond.wait_for(lock, std::chrono::milliseconds(timeout), [this]{return _items_size || _closed;})) {
                return false;
            }
        }
        
        if (_items_size) {
            for (int i = 0; i < _max_queue_num; ++i) {
                if (!_queue[i].empty()) {
                    item = std::move(_queue[i].front());
                    _queue[i].pop_front();
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
        return _items_size;
    }
    
private:
    static const int        _MAX_QUEUE_NUM = 10;
    mutable std::mutex      _mutex;
    std::condition_variable _cond;
    std::list<T>            _queue[_MAX_QUEUE_NUM];
    size_t                  _items_size;
    bool                    _closed;
    int                     _max_queue_num;
};


#endif /* TXCGradeBlockingQueue_h */
