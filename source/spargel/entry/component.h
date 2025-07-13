#pragma once

namespace spargel::entry {

    struct LaunchData;

    class Component {
    public:
        virtual ~Component() = default;
        explicit Component(LaunchData* l) : _launch_data{l} {}

        virtual void onLoad() = 0;

        LaunchData* getLaunchData() { return _launch_data; }

    private:
        LaunchData* _launch_data;
    };

}  // namespace spargel::entry
