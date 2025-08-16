# Language Design

We need a better language for shaders, scripts, and the engine itself.

Roadmap:

- Implement frontend.
- C++ backend

## Tour

```
// This is a comment.

// A module consists of all the sources in a directory.
open bar // Introduce all the names in the module `bar` into the current scope.
// Modules are resolved in the compiler args.
// compiler --module bar=/path/to/build/dir/foo/bar/bar.xxxmod

// Defining a function.
func main() {
    // There is no need for semicolons.
    print("hello") // calling a function

    if 1 + 1 > 2 {
        print("1 + 1 > 2")
    } else {
        print("1 + 1 <= 2")
    }

    // a nested definition
    func my_if_else(cond, succ, fail) {
        // pattern matching
        match cond() {
            true => succ()
            false => fail()
        }
    }
}

func fib(n: nat) {
    match n {
        0 => 0
        1 => 0
        n => fib(n - 1) + fib(n - 2)
    }
}

struct VertexInput {
    @shader.location(0)
    position: vec2(float)

    @shader.location(1)
    color: vec4(float)
}

struct RasterInput {
    @shader.position
    position: vec3(float)

    color: vec4(float)
}

@shader(stage: vertex)
func vertex_shader(@shader.input input: VertexInput,
                   @shader.vertex_id vid: uint,
                   @shader.instance_id iid: uint,
                   @shader.binding(group: 0, location: 0) viewport: &float2) {
    var pos = input.position / ( viewport / 2.0 )
    RasterInput { // position: &mut vec3(float), color: &mut vec4(float)
        position = vec3(float)(pos)
        color = input.color
    }
}

@shader(stage = pixel)
func pixel_shader(@shader.input input: RasterInput) {
    input.color
}
```

## C++ Interop

Use annotations to instruct code generation (for the C++ backend only).

- Mark a function as defined in C++.

```
@cpp.extern_func
func fib(n: int) -> int
```

Renaming and namespaces are supported.

```
@cpp.extern_func(header: "spargel/base/panic.h",
                 name: "spargel::base::panic_at")
func panic(msg: *const char, file: *const char, func: *const char, line: u32) -> never
```
