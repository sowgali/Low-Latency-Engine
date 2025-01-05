#include "../mem_pool.hpp"

struct myStruct {
    double d_[3];
};

int main(int,char**){
    using namespace Common;

    Mempool<double> doublePool(100);
    Mempool<myStruct> structPool(100);

    for(int i = 0; i < 100; i++){
        auto d_ret = doublePool.allocate(i);
        auto s_ret = structPool.allocate(myStruct({static_cast<double>(i),static_cast<double>(i+1),static_cast<double>(i+2)}));

        std::cout << "double elem: " << *d_ret << " allocated at:" << d_ret << std::endl;
        std::cout << "struct elem: {" << s_ret->d_[0] << "," << s_ret->d_[1] << "," << s_ret->d_[2] << "} allocated at:" << s_ret << std::endl;

        if(i % 5 == 0){
            std::cout << "deallocating double elem: " << *d_ret << " from " << d_ret << std::endl;
            std::cout << "deallocating struct elem: {" << s_ret->d_[0] << "," << s_ret->d_[1] << "," << s_ret->d_[2] << "} from " << s_ret << std::endl;

            doublePool.deallocate(d_ret);
            structPool.deallocate(s_ret);
        }

    }

    return 0;
}