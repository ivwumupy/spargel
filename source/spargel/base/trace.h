#pragma once

#include <spargel/base/allocator.h>
#include <spargel/base/types.h>
#include <spargel/config.h>

//
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

namespace spargel::base {

    enum class EventKind {
        enter_region,
        leave_region,
    };

    struct TraceEvent {
        char name[64];
        EventKind kind;
        u64 timestamp;
    };

    static_assert(sizeof(TraceEvent) == 80);

    class TraceEngine {
    public:
        static TraceEngine* getInstance();

        TraceEngine() {
            _file = fopen("trace.bin", "w");
            _buffer = reinterpret_cast<u8*>(default_allocator()->allocate(4096));
            _end = _buffer + 4096;
            _cur = _buffer;
        }

        ~TraceEngine() {
            flush();
            fclose(_file);
            default_allocator()->free(_buffer, 4096);
        }

        void enterRegion(char const* name) {
            emit(name, EventKind::enter_region, getTimestamp());
        }

        void leaveRegion(char const* name) {
            emit(name, EventKind::leave_region, getTimestamp());
        }

        void emit(char const* name, EventKind k, u64 t) {
            // if (_cnt >= 1000) return;

            usize l = strlen(name);
            if (l > 63) l = 63;
            TraceEvent e;
            memcpy(e.name, name, l);
            e.name[l] = 0;
            e.kind = k;
            e.timestamp = t;

            if (_cur + sizeof(e) > _end) {
                flush();
            }
            memcpy(_cur, &e, sizeof(e));
            _cur += sizeof(e);
        }

    private:
        u64 getTimestamp() {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return tv.tv_usec;
        }

        void flush() {
            fwrite(_buffer, _cur - _buffer, 1, _file);
            _cnt++;
            _cur = _buffer;
        }

        FILE* _file;
        u8* _buffer;
        u8* _end;
        u8* _cur;
        u32 _cnt = 0;
    };

    struct RegionTrace {
        RegionTrace(char const* n) : name{n} {
            TraceEngine::getInstance()->enterRegion(name);
        }

        ~RegionTrace() {
            TraceEngine::getInstance()->leaveRegion(name);
        }

        char const* name;
    };

}  // namespace spargel::base

#if SPARGEL_ENABLE_TRACING
#define spargel_trace_scope(name) \
    ::spargel::base::RegionTrace _trace_scope_##__LINE__(name)
#else
#define spargel_trace_scope(name)
#endif
