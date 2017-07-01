//
//  main.cpp
//  TestCpp
//
//  Created by lijie on 16/5/21.
//  Copyright © 2016年 gansidui. All rights reserved.
//

#include <iostream>
#include <thread>
#include <future>
#include <functional>

#include "TXCGradeBlockingQueue.h"

int main() {
    TXCGradeBlockingQueue<int> q(2);
    
    auto fut1 = std::async(std::launch::async, [&q]() {
        for (int i = 0; i < 5; ++i) {
            q.push(i, 1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    
    auto fut2 = std::async(std::launch::async, [&q]() {
        for (int i = 5; i < 10; ++i) {
            q.push(i, 2);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    
    auto fut3 = std::async(std::launch::async, [&q]() {
        int item = 0;
        while (q.pop(item)) {
            std::cout << "item: " << item << std::endl;
        }
        std::cout << "blocking queue is closed" << std::endl;
    });
    
    fut1.wait();
    fut2.wait();
    
    q.close();
    
    fut3.wait();

    return 0;
}


