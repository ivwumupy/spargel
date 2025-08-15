# UI

## MacOS

- https://www.colincornaby.me/2025/08/your-mac-game-is-probably-rendering-blurry/#problem

Note: On my MacBook Pro the "full screen area" is `3024x1890` which is 16:10, obtained as follows.

```cpp
auto modes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), nullptr);
CFArrayApplyFunction(
    modes, CFRangeMake(0, CFArrayGetCount(modes)),
    [](const void* value, void*) {
        auto mode = (CGDisplayModeRef)value;
        auto width = CGDisplayModeGetWidth(mode);
        auto height = CGDisplayModeGetHeight(mode);
        printf("mode: %zu x %zu\n", width, height);
    },
    nullptr);
```
