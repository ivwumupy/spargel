#pragma once

#include "spargel/base/algorithm.h"
#include "spargel/base/allocator.h"
#include "spargel/base/check.h"
#include "spargel/base/object.h"
#include "spargel/base/types.h"

namespace spargel::base {
    namespace ref_ptr_ {
        template <typename T>
        class RefCount {
        public:
            ~RefCount() { spargel_check(count_ == 0); }

            void retain() { count_++; }
            void release() {
                spargel_check(count_ > 0);
                count_--;
                if (count_ == 0) {
                    delete static_cast<T*>(this);
                }
            }

        private:
            // how many RefPtr point to this object
            usize count_ = 0;
        };
        // T should inherit from RefCount
        template <typename T>
        class RefPtr {
        public:
            RefPtr() = default;
            RefPtr(nullptr_t) {}
            RefPtr(T* p) : ptr_{p} {
                if (ptr_) {
                    ptr_->retain();
                }
            }

            RefPtr(RefPtr const& other) : ptr_{other.ptr_} {
                if (ptr_) {
                    ptr_->retain();
                }
            }
            RefPtr& operator=(RefPtr const& other) {
                reset(other.ptr_);
                return *this;
            }
            RefPtr(RefPtr&& other) : ptr_{other.ptr_} { other.ptr_ = nullptr; }
            RefPtr& operator=(RefPtr&& other) {
                if (ptr_) {
                    ptr_->release();
                }
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
                return *this;
            }

            ~RefPtr() {
                if (ptr_) {
                    ptr_->release();
                }
            }

            T* operator->() { return ptr_; }
            T const* operator->() const { return ptr_; }

            T& operator*() { return *ptr_; }
            T const& operator*() const { return *ptr_; }

            T* get() { return ptr_; }
            T const* get() const { return ptr_; }

            void reset(T* p) {
                if (ptr_) {
                    ptr_->release();
                }
                ptr_ = p;
                if (ptr_) {
                    ptr_->retain();
                }
            }

        private:
            friend void tag_invoke(tag<swap>, RefPtr& lhs, RefPtr& rhs) {
                swap(lhs.ptr_, rhs.ptr_);
            }

            T* ptr_ = nullptr;
        };
    }  // namespace ref_ptr_
    using ref_ptr_::RefCount;
    using ref_ptr_::RefPtr;
    template <typename T, typename... Args>
    RefPtr<T> makeRef(Args&&... args) {
        return RefPtr<T>(new T(forward<Args>(args)...));
    }
}  // namespace spargel::base
