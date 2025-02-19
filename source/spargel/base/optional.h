#pragma once

#include <spargel/base/algorithm.h>
#include <spargel/base/meta.h>
#include <spargel/base/object.h>
#include <spargel/base/tag_invoke.h>
#include <spargel/base/types.h>

namespace spargel::base {

    namespace _optional {

        template <typename T>
        class Optional {
        public:
            Optional() = default;

            Optional(Optional const& other) : _status{other._status} {
                if (other.hasValue()) {
                    construct_at(getPtr(), other.value());
                }
            }
            Optional& operator=(Optional const& other) {
                Optional tmp(other);
                swap(*this, tmp);
                return *this;
            }

            Optional(Optional&& other) { swap(*this, other); }
            Optional& operator=(Optional&& other) {
                Optional tmp(move(other));
                swap(*this, tmp);
                return *this;
            }

            ~Optional() {
                if (hasValue()) {
                    destroyObject();
                }
            }

            bool hasValue() const { return _status == Status::value; }

            T& value() & { return *getPtr(); }
            T const& value() const& { return *getPtr(); }
            T&& value() && { return move(*getPtr()); }

            friend void tag_invoke(tag<swap>, Optional& lhs, Optional& rhs) {
                if (lhs.hasValue() && rhs.hasValue()) {
                    swap(*(lhs.getPtr()), *(rhs.getPtr()));
                } else if (lhs.hasValue() && !rhs.hasValue()) {
                    construct_at(rhs.getPtr(), move(*(lhs.getPtr())));
                    swap(lhs._status, rhs._status);
                } else if (!lhs.hasValue() && rhs.hasValue()) {
                    construct_at(lhs.getPtr(), move(*(rhs.getPtr())));
                    swap(lhs._status, rhs._status);
                }
            }

            template <typename S, typename... Arg>
            friend Optional<S> makeOptional(Arg&&... args);

        private:
            struct InPlaceTag {};

            enum class Status {
                empty,
                value,
            };

            // Attempt to construct a value of type `T`.
            template <typename... Arg>
            Optional(InPlaceTag, Arg&&... args) : _status{Status::value} {
                construct_at(getPtr(), forward<Arg>(args)...);
            }

            T* getPtr() { return reinterpret_cast<T*>(_bytes); }
            T const* getPtr() const { return reinterpret_cast<T const*>(_bytes); }

            void destroyObject() { destruct_at(getPtr()); }

            Status _status = Status::empty;

            // `_bytes` provides the storage of an object of `T`
            //
            // Note: see "type-accessible"
            alignas(alignof(T)) Byte _bytes[sizeof(T)];
        };

        template <typename S, typename... Arg>
        Optional<S> makeOptional(Arg&&... args) {
            return Optional<S>(typename Optional<S>::InPlaceTag{}, forward<Arg>(args)...);
        }

    }  // namespace _optional

    using _optional::makeOptional;
    using _optional::Optional;

}  // namespace spargel::base
