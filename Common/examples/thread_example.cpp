#include "../thread_utils.hpp"

auto dummyFunction(int a, int b, bool sleep){
    std::cout << "dummyFunction (" << a << ", " << b << ")" << std::endl;
    std::cout << "dummyFunction output: " << a+b << std::endl;

    if(sleep){
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(5s);
    }

    std::cout << "dummyFunction done." << std::endl;
}

int main(int, char **){
    using namespace Common;
    auto t1 = setAndCreateThread(-1, "dummyFunction1", dummyFunction, 5, 10, false);
    auto t2 = setAndCreateThread(1, "dummyFunction2", dummyFunction, 15, 20, true);

    std::cout << "main is waiting for the threads to be done." << std::endl;
    t1->join();
    t2->join();
    std::cout << "main exiting." << std::endl;

    return 0;
}