#include "spargel/base/console.h"

//
#include "spargel/base/check.h"
#include "spargel/base/checked_convert.h"
#include "spargel/base/memory.h"
#include "spargel/base/panic.h"

#if SPARGEL_IS_POSIX
//
#include <unistd.h>
#endif

namespace spargel::base {
    namespace {
        // 4KB
        static constexpr usize BUFFER_SIZE = 4096;
    }  // namespace
    Console& Console::instance() {
        static Console inst;
        return inst;
    }
    Console::Console() {
        begin_ = new char[BUFFER_SIZE];
        end_ = begin_ + BUFFER_SIZE;
        cur_ = begin_;
    }
    Console::~Console() {
        flush();
        delete[] begin_;
    }
    void Console::write(char c) {
        if (cur_ >= end_) [[unlikely]] {
            flush();
        }
        *cur_ = c;
        cur_++;
    }
    void Console::write(char const* s, usize len) {
        if (usize(end_ - cur_) < len) [[unlikely]] {
            // There is not enough space.
            if (cur_ == begin_) {
                // The buffer is empty, and we don't need to copy to the
                // internal buffer. Just output a multiple of buffer size.
                usize bytes_to_output = len - len % BUFFER_SIZE;
                output(s, bytes_to_output);
                // The remaining data fits into the buffer.
                usize remain = len - bytes_to_output;
                copy_to_buffer(s, remain);
                return;
            }
            usize bytes_to_copy = usize(end_ - cur_);
            copy_to_buffer(s, bytes_to_copy);
            flush();
            write(s + bytes_to_copy, len - bytes_to_copy);
            return;
        }
        copy_to_buffer(s, len);
    }
    void Console::flush() {
        if (cur_ == begin_) return;
        usize len = usize(cur_ - begin_);
        output(begin_, len);
        cur_ = begin_;
    }
    void Console::copy_to_buffer(char const* p, usize len) {
        spargel_check(len <= usize(end_ - cur_));
        memcpy(cur_, p, len);
        cur_ += len;
    }
#if SPARGEL_IS_POSIX
    void Console::output(char const* p, usize len) {
        // TODO: check EINTR or EAGAIN
        auto ret = ::write(1, p, len);
        spargel_check(ret != -1 && checkedConvert<usize>(ret) == len);
    }
#elif SPARGEL_IS_WINDOWS
    void Console::output(char const* p, usize len) {
        // TODO:
        spargel_panic_here();
    }
#endif
}  // namespace spargel::base
