#pragma once

#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>
#include <spargel/config.h>
#include <spargel/gpu/gpu.h>

// vulkan

#define VK_NO_PROTOTYPES
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#if SPARGEL_IS_ANDROID
#include <vulkan/vulkan_android.h>
#endif

#if SPARGEL_IS_LINUX
#include <xcb/xcb.h>
/* after xcb */
#include <vulkan/vulkan_wayland.h>
#include <vulkan/vulkan_xcb.h>
#endif

#if SPARGEL_IS_MACOS
#include <vulkan/vulkan_metal.h>
#endif

#if SPARGEL_IS_WINDOWS
#include <windows.h>
/* after windows.h */
#include <vulkan/vulkan_win32.h>
#endif

namespace spargel::gpu {

    class ShaderLibraryVulkan : public ShaderLibrary {
    public:
        explicit ShaderLibraryVulkan();
        ~ShaderLibraryVulkan();

        auto library() const { return _library; }

    private:
        VkShaderModule _library;
    };

    class RenderPipelineVulkan : public RenderPipeline {
    public:
        explicit RenderPipelineVulkan();
        ~RenderPipelineVulkan();

        auto pipeline() { return _pipeline; }

    private:
        VkPipeline _pipeline;
    };

    class BufferVulkan : public Buffer {
    public:
        explicit BufferVulkan();
        ~BufferVulkan();

        auto buffer() { return _buffer; }

    private:
        VkBuffer _buffer;
    };

    class TextureVulkan final : public Texture {
    public:
        explicit TextureVulkan(VkImage t) : _texture{t} {}
        ~TextureVulkan() {}

        auto texture() const { return _texture; }

        void updateRegion(u32 x, u32 y, u32 width, u32 height, u32 bytes_per_row,
                          base::span<u8> bytes) override;

    private:
        VkImage _texture;
    };

    class SurfaceVulkan final : public Surface {
    public:
        explicit SurfaceVulkan(VkSurfaceKHR s) : _surface{s} {}
        ~SurfaceVulkan() {}

        auto surface() const { return _surface; }

        ObjectPtr<Texture> nextTexture() override;

        float width() override;
        float height() override;

    private:
        VkSurfaceKHR _surface;
    };

    class CommandQueueVulkan final : public CommandQueue {
    public:
        explicit CommandQueueVulkan(VkQueue queue) : _queue{queue} {}
        ~CommandQueueVulkan() {}

        auto commandQueue() { return _queue; }

        ObjectPtr<CommandBuffer> createCommandBuffer() override;
        void destroyCommandBuffer(ObjectPtr<CommandBuffer>) override;

    private:
        VkQueue _queue;
    };

    class CommandBufferVulkan final : public CommandBuffer {
    public:
        explicit CommandBufferVulkan(VkCommandBuffer cmdbuf) : _cmdbuf{cmdbuf} {}

        auto commandBuffer() { return _cmdbuf; }

        ObjectPtr<RenderPassEncoder> beginRenderPass(
            RenderPassDescriptor const& descriptor) override;
        void endRenderPass(ObjectPtr<RenderPassEncoder> encoder) override;
        void present(ObjectPtr<Surface> surface) override;
        void submit() override;

    private:
        VkCommandBuffer _cmdbuf;
    };

    class RenderPassEncoderVulkan final : public RenderPassEncoder {
    public:
        explicit RenderPassEncoderVulkan() {}

        void setRenderPipeline(ObjectPtr<RenderPipeline> pipeline) override;
        void setVertexBuffer(ObjectPtr<Buffer> buffer, vertex_buffer_location const& loc) override;
        void setTexture(ObjectPtr<Texture> texture) override;
        void setViewport(Viewport viewport) override;
        void draw(int vertex_start, int vertex_count) override;
        void draw(int vertex_start, int vertex_count, int instance_start,
                  int instance_count) override;

    private:
    };

    class DeviceVulkan final : public Device {
    public:
        DeviceVulkan();
        ~DeviceVulkan() override;

        ObjectPtr<ShaderLibrary> createShaderLibrary(base::span<u8> bytes) override;
        ObjectPtr<RenderPipeline> createRenderPipeline(
            RenderPipelineDescriptor const& descriptor) override;
        ObjectPtr<Buffer> createBuffer(base::span<u8> bytes) override;
        ObjectPtr<Surface> createSurface(ui::window* w) override;

        ObjectPtr<Texture> createTexture(u32 width, u32 height) override;
        void destroyTexture(ObjectPtr<Texture> texture) override;

        void destroyShaderLibrary(ObjectPtr<ShaderLibrary> library) override;
        void destroyRenderPipeline(ObjectPtr<RenderPipeline> pipeline) override;
        void destroyBuffer(ObjectPtr<Buffer> b) override;

        ObjectPtr<CommandQueue> createCommandQueue() override;
        void destroyCommandQueue(ObjectPtr<CommandQueue> q) override;

    private:
        VkDevice _device;
    };

}  // namespace spargel::gpu
