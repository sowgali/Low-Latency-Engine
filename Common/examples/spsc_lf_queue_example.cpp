#include "spsc_lf_queue.hpp"
#include "thread_utils.hpp"

using namespace Common;

struct myStruct {
    int d_[3];
};

void consumeFunction(LFQueue<myStruct>* lfq){
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(5s);

    while(lfq->size()){
        const auto d = lfq->getNextReadLocation();
        lfq->updateNextToRead();

        std::cout << "consumeFunction read elem: {" << d->d_[0] << ", " << d->d_[1] << ", " << d->d_[2] << "} lfq-size: " << lfq->size() << std::endl;

        std::this_thread::sleep_for(1s);
    }

    std::cout << "consumeFunction exiting." << std::endl;
}


int main(int, char**){
    LFQueue<myStruct> lfq(20);
    auto ct = setAndCreateThread(-1, "Consumer Thread", consumeFunction, &lfq);

    for(int i = 0; i < 50; i++){
        const myStruct d{i, i+1, i+2};
        *(lfq.getNextWriteLocation()) = d;
        lfq.updateNextToWrite();
        std::cout << "main constructed elem: {" << d.d_[0] << ", " << d.d_[1] << ", " << d.d_[2] << "} lfq-size: " << lfq.size() << std::endl;
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    ct->join();

    std::cout << "main exiting." << std::endl;

    return 0;
}