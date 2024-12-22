#pragma once

#include <spargel/base/allocator.h>
#include <spargel/base/meta.h>
#include <spargel/base/object.h>
#include <spargel/base/span.h>
#include <spargel/base/string_view.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>

namespace spargel::ui {
    class window;
}

namespace spargel::gpu {

    enum class TextureFormat {
        a8_unorm,
        bgra8_unorm,
        bgra8_srgb,
    };

    enum class LoadAction {
        dont_care,
        load,
        clear,
    };

    enum class StoreAction {
        dont_care,
        store,
    };

    enum class DeviceKind {
        directx,
        metal,
        vulkan,
    };

    // notes:
    //
    // msl = metal shading language specification
    //
    // [msl, v3.2, p84]
    // an address space attribute specifies the region of memory from where buffer memory objects
    // are allocated. these attributes describe disjoint address spaces:
    // - device
    // - constant
    // - thread
    // - threadgroup
    // - threadgroup_imageblock
    // - ray_data
    // - object_data
    // all arguments to a graphics or kernel function that are a pointer or reference to a type
    // needs to be declared with an address space attribute.
    //
    // the address space for a variable at program scope needs to be `constant`.
    //
    // [msl, v3.2, p98]
    // arguments to graphics and kernel functions can be any of the following:
    // - device buffer: a pointer or reference to any data type in the device address space
    // - constant buffer: a pointer or reference to any data type in the constant address space
    // - ...
    //
    // [msl, v3.2, p99]
    // for each argument, an attribute can be optionally specified to identify the location of a
    // buffer, texture, or sampler to use for this argument type.
    // - device and constant buffers: `[[buffer(index)]]`
    // - textures (including texture buffers): `[[texture(index)]]`
    // - samplers: `[[sampler(index)]]`
    //

    /// @brief triangles with particular facing will not be drawn
    ///
    /// directx: https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_cull_mode
    /// metal: https://developer.apple.com/documentation/metal/mtlcullmode
    /// vulkan: https://registry.khronos.org/vulkan/specs/latest/man/html/VkCullModeFlagBits.html
    ///
    enum class CullMode {
        /// @brief no triangles are discarded
        none,
        /// @brief front-facing triangles are discarded
        front,
        /// @brief back-facing triangles are discarded
        back,
    };

    /// @brief the front-facing orientation
    ///
    /// directx:
    /// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc
    /// metal: https://developer.apple.com/documentation/metal/mtlwinding
    /// vulkan: https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkFrontFace.html
    ///
    enum class Orientation {
        /// @brief clockwise triangles are front-facing
        clockwise,
        /// @brief counter-clockwise triangles are front-facing
        counter_clockwise,
    };

    /// @brief geometric primitive type for rendering
    ///
    /// directx:
    /// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_primitive_topology_type
    /// metal: https://developer.apple.com/documentation/metal/mtlprimitivetopologyclass
    /// vulkan: https://registry.khronos.org/vulkan/specs/latest/man/html/VkPrimitiveTopology.html
    ///
    enum class PrimitiveKind {
        point,
        line,
        triangle,
    };

    /// @brief the rate of fetching vertex attributes
    ///
    /// directx:
    /// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_input_classification
    /// metal: https://developer.apple.com/documentation/metal/mtlvertexstepfunction
    /// vulkan:
    /// https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkVertexInputRate.html
    ///
    enum class VertexStepMode {
        /// @brief attributes are fetched per vertex
        Vertex,
        /// @brief attributes are fetched per instance
        Instance,
    };

    enum class VertexAttributeFormat {
        uint8,
        uint8x2,
        uint8x4,
        sint8,
        sint8x2,
        sint8x4,
        unorm8,
        unorm8x2,
        unorm8x4,
        snorm8,
        snorm8x2,
        snorm8x4,
        uint16,
        uint16x2,
        uint16x4,
        sint16,
        sint16x2,
        sint16x4,
        unorm16,
        unorm16x2,
        unorm16x4,
        snorm16,
        snorm16x2,
        snorm16x4,
        float16,
        float16x2,
        float16x4,
        float32,
        float32x2,
        float32x4,
    };

    enum class BlendAction {
        min,
        max,
        add,
        subtract,
        reverse_subtract,
    };

    enum class BlendFactor {
        zero,
        one,
        src_color,
        dst_color,
        one_minus_src_color,
        one_minus_dst_color,
        src_alpha,
        dst_alpha,
        one_minus_src_alpha,
        one_minus_dst_alpha,
    };

    enum class DepthCompare {
        never,
        less,
        equal,
        less_equal,
        greater,
        not_equal,
        greater_equal,
        always,
    };

    struct Viewport {
        float x;
        float y;
        float width;
        float height;
        float z_near;
        float z_far;
    };

    struct vertex_buffer_location {
        struct {
            int buffer_index;
        } apple;
        struct {
            int vertex_buffer_index;
        } vulkan;
    };

    template <typename T>
    class ObjectPtr {
    public:
        ObjectPtr() = default;
        ObjectPtr(nullptr_t) {}

        template <typename U>
            requires(base::is_convertible<U*, T*>)
        explicit ObjectPtr(U* ptr) : _ptr{ptr} {}

        template <typename U>
            requires(base::is_convertible<U*, T*>)
        ObjectPtr(ObjectPtr<U> const ptr) : _ptr{ptr.get()} {}

        T* operator->() { return _ptr; }
        T const* operator->() const { return _ptr; }

        T* get() const { return _ptr; }

        template <typename U>
        ObjectPtr<U> cast() {
            return ObjectPtr<U>(static_cast<U*>(_ptr));
        }
        template <typename U>
        ObjectPtr<U> const cast() const {
            return ObjectPtr<U>(static_cast<U*>(_ptr));
        }

    private:
        T* _ptr = nullptr;
    };

    template <typename T, typename... Args>
    ObjectPtr<T> make_object(Args&&... args) {
        T* ptr = static_cast<T*>(base::default_allocator()->alloc(sizeof(T)));
        base::construct_at(ptr, base::forward<Args>(args)...);
        return ObjectPtr<T>(ptr);
    }

    template <typename T>
    void destroy_object(ObjectPtr<T> ptr) {
        ptr->~T();
        base::default_allocator()->free(ptr.get(), sizeof(T));
    }

    class ShaderLibrary {};
    class RenderPipeline {};
    class BindGroup {};
    class BindGroupLayout {};
    class Buffer {};
    class Texture {
    public:
        virtual void updateRegion(u32 x, u32 y, u32 width, u32 height, u32 bytes_per_row,
                                  base::span<u8> bytes) = 0;
    };
    class Surface {
    public:
        virtual ObjectPtr<Texture> nextTexture() = 0;
        virtual float width() = 0;
        virtual float height() = 0;
    };
    class RenderPassEncoder {
    public:
        virtual void setRenderPipeline(ObjectPtr<RenderPipeline> pipeline) = 0;
        virtual void setVertexBuffer(ObjectPtr<Buffer> buffer,
                                     vertex_buffer_location const& loc) = 0;
        virtual void setTexture(ObjectPtr<Texture> texture) = 0;
        virtual void setViewport(Viewport viewport) = 0;
        virtual void draw(int vertex_start, int vertex_count) = 0;
        virtual void draw(int vertex_start, int vertex_count, int instance_start,
                          int instance_count) = 0;
    };

    struct ClearColor {
        float r;
        float g;
        float b;
        float a;
    };

    struct ColorAttachmentBindDescriptor {
        ObjectPtr<Texture> texture;
        LoadAction load = LoadAction::clear;
        StoreAction store = StoreAction::store;
        ClearColor clear_color;
    };

    struct RenderPassDescriptor {
        base::span<ColorAttachmentBindDescriptor> color_attachments;
    };

    class CommandBuffer {
    public:
        virtual ObjectPtr<RenderPassEncoder> beginRenderPass(
            RenderPassDescriptor const& descriptor) = 0;
        virtual void endRenderPass(ObjectPtr<RenderPassEncoder> encoder) = 0;
        virtual void present(ObjectPtr<Surface> surface) = 0;
        virtual void submit() = 0;
    };
    class CommandQueue {
    public:
        virtual ObjectPtr<CommandBuffer> createCommandBuffer() = 0;
        virtual void destroyCommandBuffer(ObjectPtr<CommandBuffer>) = 0;
    };

    struct ShaderLibraryDescriptor {
        base::span<u8> bytes;
    };

    struct ShaderFunction {
        ObjectPtr<ShaderLibrary> library;
        char const* entry;
    };

    /// @brief
    ///
    /// directx:
    /// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_input_element_desc
    /// metal: https://developer.apple.com/documentation/metal/mtlvertexattributedescriptor
    /// vulkan:
    /// https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkVertexInputAttributeDescription.html
    ///
    struct VertexAttributeDescriptor {
        /// @brief index of the vertex buffer where the data of the attribute is fetched
        u32 buffer;

        /// @brief the location of the vertex attribute
        ///
        /// this is called input element in directx.
        ///
        /// synatx in shaders:
        /// - glsl: `layout (location = 0) in vec3 position;`
        /// - msl: `float3 position [[attribute(0)]];`
        /// - hlsl: this is specified via semantic name and semantic index.
        ///
        /// note:
        /// - both dawn and wgpu use dummy SemanticName, and use SemanticIndex for location.
        /// - dawn chooses "TEXCOORD", and wgpu/naga uses "LOC".
        /// - SPIRV-Cross uses "TEXCOORD" by default, and provides the options for remapping.
        /// - dxc relies on user declaration `[[vk::location(0)]]`.
        ///
        u32 location;

        /// @brief the format of the vertex attribute
        VertexAttributeFormat format;

        /// @brief the offset of the vertex attribute to the start of the vertex data
        ssize offset;
    };

    /// @brief description of one vertex buffer
    ///
    /// directx:
    /// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_input_layout_desc
    /// metal: https://developer.apple.com/documentation/metal/mtlvertexdescriptor
    /// vulkan:
    /// https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkPipelineVertexInputStateCreateInfo.html
    ///
    struct VertexBufferDescriptor {
        /// @brief the number of bytes between consecutive elements in the buffer
        ///
        /// directx:
        /// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_vertex_buffer_view
        /// metal:
        /// https://developer.apple.com/documentation/metal/mtlvertexbufferlayoutdescriptor/1515441-stride
        /// vulkan:
        /// https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkVertexInputBindingDescription.html
        ///
        usize stride;

        /// @brief the rate of fetching vertex attributes
        VertexStepMode step_mode;

        // TODO: step rate is not supported by vulkan.
    };

    struct ColorAttachmentDescriptor {
        TextureFormat format;
        bool enable_blend = false;
    };

    /// @brief description of a render pipeline
    ///
    /// directx:
    /// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_graphics_pipeline_state_desc
    /// metal: https://developer.apple.com/documentation/metal/mtlrenderpipelinedescriptor
    /// vulkan:
    /// https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkGraphicsPipelineCreateInfo.html
    ///
    struct RenderPipelineDescriptor {
        /// @brief the geometric primitive for this render pipeline
        PrimitiveKind primitive = PrimitiveKind::triangle;

        /// @brief the orientation of front-facing triangles
        Orientation front_face = Orientation::counter_clockwise;

        /// @brief which triangles to discard
        CullMode cull = CullMode::none;

        /// @brief the vertex function
        ShaderFunction vertex_shader;

        /// @brief the vertex buffers used in this pipeline
        base::span<VertexBufferDescriptor> vertex_buffers;

        /// @brief the vertex attributes used in this pipeline
        base::span<VertexAttributeDescriptor> vertex_attributes;

        /// @brief the fragment function
        ShaderFunction fragment_shader;

        base::span<ColorAttachmentDescriptor> color_attachments;
    };

    class Device {
    public:
        virtual ~Device() = default;

        DeviceKind kind() const { return _kind; }

        virtual ObjectPtr<ShaderLibrary> createShaderLibrary(base::span<u8> bytes) = 0;
        virtual ObjectPtr<RenderPipeline> createRenderPipeline(
            RenderPipelineDescriptor const& descriptor) = 0;
        virtual ObjectPtr<Surface> createSurface(ui::window* w) = 0;
        virtual void destroyShaderLibrary(ObjectPtr<ShaderLibrary> library) = 0;
        virtual void destroyRenderPipeline(ObjectPtr<RenderPipeline> pipeline) = 0;

        virtual ObjectPtr<Buffer> createBuffer(base::span<u8> bytes) = 0;
        virtual void destroyBuffer(ObjectPtr<Buffer> buffer) = 0;

        // todo: format, 2d
        virtual ObjectPtr<Texture> createTexture(u32 width, u32 height) = 0;
        virtual void destroyTexture(ObjectPtr<Texture> texture) = 0;

        virtual ObjectPtr<CommandQueue> createCommandQueue() = 0;
        virtual void destroyCommandQueue(ObjectPtr<CommandQueue> queue) = 0;

    protected:
        explicit Device(DeviceKind k) : _kind{k} {}

    private:
        DeviceKind _kind;
    };

    base::unique_ptr<Device> makeDevice(DeviceKind kind);

    class RenderPipelineBuilder {
    public:
        void setPrimitive(PrimitiveKind primitive) { _desc.primitive = primitive; }
        void setVertexShader(ObjectPtr<ShaderLibrary> library, char const* entry) {
            _desc.vertex_shader.library = library;
            _desc.vertex_shader.entry = entry;
        }
        void setFragmentShader(ObjectPtr<ShaderLibrary> library, char const* entry) {
            _desc.fragment_shader.library = library;
            _desc.fragment_shader.entry = entry;
        }
        void addVertexBuffer(usize stride) { _vertex_buffers.push(stride, VertexStepMode::Vertex); }
        void addVertexBuffer(usize stride, VertexStepMode step_mode) {
            _vertex_buffers.push(stride, step_mode);
        }
        void addVertexAttribute(VertexAttributeFormat format, u32 location, u32 buffer,
                                u32 offset) {
            _vertex_attributes.push(buffer, location, format, offset);
        }
        void addColorAttachment(TextureFormat format, bool enable_blend) {
            _color_attachments.push(format, enable_blend);
        }

        ObjectPtr<RenderPipeline> build(Device* device) {
            _desc.vertex_buffers = _vertex_buffers.toSpan();
            _desc.vertex_attributes = _vertex_attributes.toSpan();
            _desc.color_attachments = _color_attachments.toSpan();
            return device->createRenderPipeline(_desc);
        }

    private:
        RenderPipelineDescriptor _desc;
        base::vector<VertexBufferDescriptor> _vertex_buffers;
        base::vector<VertexAttributeDescriptor> _vertex_attributes;
        base::vector<ColorAttachmentDescriptor> _color_attachments;
    };

}  // namespace spargel::gpu

// ## Normalized Device Coordinates
//
// ### Vulkan
//
// Clip coordinates for a vertex result from shader execution, which yields a vertex coordinate
// Position.
//
// Perspective division on clip coordinates yields normalized device coordinates, followed by a
// viewport transformation to convert these coordinates into framebuffer coordinates.
//
// Rasterizing a primitive begins by determining which squares of an integer grid in framebuffer
// coordinates are occupied by the primitive, and assigning one or more depth values to each such
// square.
