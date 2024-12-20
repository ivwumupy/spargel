#pragma once

#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>
#include <spargel/gpu/gpu3.h>

// metal
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

namespace spargel::gpu {

    class ShaderLibraryMetal : public ShaderLibrary {
    public:
        explicit ShaderLibraryMetal(dispatch_data_t data, id<MTLLibrary> library);
        ~ShaderLibraryMetal();

        auto library() const { return _library; }

    private:
        dispatch_data_t _data;
        id<MTLLibrary> _library;
    };

    class RenderPipelineMetal : public RenderPipeline {
    public:
        explicit RenderPipelineMetal(id<MTLFunction> vertex_function,
                                     id<MTLRenderPipelineState> pipeline);
        ~RenderPipelineMetal();

        id<MTLRenderPipelineState> pipeline() { return _pipeline; }

    private:
        id<MTLFunction> _vertex;
        id<MTLRenderPipelineState> _pipeline;
    };

    class BufferMetal : public Buffer {
    public:
        explicit BufferMetal(id<MTLBuffer> b);
        ~BufferMetal();

        id<MTLBuffer> buffer() { return _buffer; }

    private:
        id<MTLBuffer> _buffer;
    };

    class TextureMetal final : public Texture {
    public:
        explicit TextureMetal(id<MTLTexture> t) : _texture{t} {}
        ~TextureMetal() {}

        id<MTLTexture> texture() const { return _texture; }

        void updateRegion(u32 x, u32 y, u32 width, u32 height, u32 bytes_per_row,
                          base::span<u8> bytes) override;

    private:
        id<MTLTexture> _texture;
    };

    class SurfaceMetal final : public Surface {
    public:
        explicit SurfaceMetal(CAMetalLayer* layer) : _layer{layer}, _texture(nil) {}
        ~SurfaceMetal() {}

        CAMetalLayer* layer() { return _layer; }

        ObjectPtr<Texture> nextTexture() override {
            _drawable = [_layer nextDrawable];
            base::construct_at(&_texture, _drawable.texture);
            return ObjectPtr<Texture>(&_texture);
        }

        id<CAMetalDrawable> drawable() { return _drawable; }

        float width() override { return _layer.drawableSize.width; }
        float height() override { return _layer.drawableSize.height; }

    private:
        CAMetalLayer* _layer;
        id<CAMetalDrawable> _drawable;
        TextureMetal _texture;
    };

    class CommandQueueMetal final : public CommandQueue {
    public:
        explicit CommandQueueMetal(id<MTLCommandQueue> queue) : _queue{queue} {}
        ~CommandQueueMetal() { [_queue release]; }

        id<MTLCommandQueue> commandQueue() { return _queue; }

        ObjectPtr<CommandBuffer> createCommandBuffer() override;
        void destroyCommandBuffer(ObjectPtr<CommandBuffer>) override;

    private:
        id<MTLCommandQueue> _queue;
    };

    class CommandBufferMetal final : public CommandBuffer {
    public:
        // command buffer is autoreleased
        explicit CommandBufferMetal(id<MTLCommandBuffer> cmdbuf) : _cmdbuf{cmdbuf} {}

        id<MTLCommandBuffer> commandBuffer() { return _cmdbuf; }

        ObjectPtr<RenderPassEncoder> beginRenderPass(
            RenderPassDescriptor const& descriptor) override;
        void endRenderPass(ObjectPtr<RenderPassEncoder> encoder) override;
        void present(ObjectPtr<Surface> surface) override;
        void submit() override;

    private:
        id<MTLCommandBuffer> _cmdbuf;
    };

    class RenderPassEncoderMetal final : public RenderPassEncoder {
    public:
        // enocder is autoreleased
        explicit RenderPassEncoderMetal(id<MTLRenderCommandEncoder> encoder) : _encoder{encoder} {}

        id<MTLCommandEncoder> encoder() { return _encoder; }

        void setRenderPipeline(ObjectPtr<RenderPipeline> pipeline) override;
        void setVertexBuffer(ObjectPtr<Buffer> buffer, vertex_buffer_location const& loc) override;
        void setTexture(ObjectPtr<Texture> texture) override;
        void setViewport(Viewport viewport) override;
        void draw(int vertex_start, int vertex_count) override;
        void draw(int vertex_start, int vertex_count, int instance_start,
                  int instance_count) override;

    private:
        id<MTLRenderCommandEncoder> _encoder;
    };

    class DeviceMetal final : public Device {
    public:
        DeviceMetal();
        ~DeviceMetal() override;

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
        id<MTLDevice> _device;
    };

}  // namespace spargel::gpu
