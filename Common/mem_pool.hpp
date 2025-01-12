#pragma once

#include<vector>
#include<string>

#include "macros.hpp"

namespace Common {
    template<typename T>
    class Mempool{
        private:
            struct ObjBlock{
                T obj_;
                bool is_free_ = true;
            };

            std::vector<ObjBlock> obj_store_;

            size_t next_free_idx_ = 0;

            void updateNextFreeBlock(){
                const auto curr_idx = next_free_idx_;

                while(!obj_store_[next_free_idx_].is_free_){
                    next_free_idx_++;
                    if(next_free_idx_ == obj_store_.size()) [[unlikely]]
                        next_free_idx_ = 0;
                    if(next_free_idx_ == curr_idx) [[unlikely]]
                        FATAL("Memory pool out of memory");
                }
            }

        public:
            Mempool(size_t num_elem) : obj_store_(std::vector<ObjBlock>(num_elem, ObjBlock({T(), true}))) {
                ASSERT(reinterpret_cast<const ObjBlock*>(&(obj_store_[0].obj_)) == &(obj_store_[0]), "T object should be the first member of ObjBlock.");
            }
            
            template<typename... Args>
            T* allocate(Args&&... args) noexcept{
                auto obj_block_ = &(obj_store_[next_free_idx_]);
                ASSERT(obj_block_->is_free_, "Expected a free block at : " + std::to_string(next_free_idx_));
                T* ret = &(obj_block_->obj_);
                ret = new(ret) T((std::forward<Args>(args))...);
                obj_block_->is_free_ = false;
                updateNextFreeBlock();
                return ret;
            }

            void deallocate(const T* elem) noexcept {
                const auto idx = reinterpret_cast<const ObjBlock*>(elem) - &obj_store_[0];

                ASSERT(idx >= 0 && static_cast<size_t>(idx) < obj_store_.size(), "Element does not belong to this pool");
                ASSERT(!obj_store_[idx].is_free_, "Expected to be an allocated block");

                obj_store_[idx].is_free_ = true;
            }

            Mempool() = delete;
            Mempool(const Mempool &) = delete;
            Mempool(const Mempool &&) = delete;
            Mempool& operator=(const Mempool &) = delete;
            Mempool& operator=(const Mempool &&) = delete;
    };
}