#pragma once

#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>
#include <spargel/gpu/gpu.h>

// metal
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

namespace spargel::gpu {

    class ShaderLibraryMetal final : public ShaderLibrary {
    public:
        explicit ShaderLibraryMetal(dispatch_data_t data, id<MTLLibrary> library);
        ~ShaderLibraryMetal();

        auto library() const { return _library; }

    private:
        dispatch_data_t _data;
        id<MTLLibrary> _library;
    };

    class RenderPipelineMetal final : public RenderPipeline {
    public:
        explicit RenderPipelineMetal(id<MTLFunction> vertex_function,
                                     id<MTLRenderPipelineState> pipeline);
        ~RenderPipelineMetal();

        auto pipeline() { return _pipeline; }

    private:
        id<MTLFunction> _vertex;
        id<MTLRenderPipelineState> _pipeline;
    };

    class BufferMetal final : public Buffer {
    public:
        explicit BufferMetal(id<MTLBuffer> b);
        ~BufferMetal();

        void* mapAddr() override { return _buffer.contents; }

        auto buffer() { return _buffer; }

    private:
        id<MTLBuffer> _buffer;
    };

    class TextureMetal final : public Texture {
    public:
        explicit TextureMetal(id<MTLTexture> t) : _texture{t} {}
        ~TextureMetal() {}

        auto texture() const { return _texture; }

        void updateRegion(u32 x, u32 y, u32 width, u32 height, u32 bytes_per_row,
                          base::span<base::Byte> bytes) override;

    private:
        id<MTLTexture> _texture;
    };

    class SurfaceMetal final : public Surface {
    public:
        explicit SurfaceMetal(CAMetalLayer* layer) : _layer{layer}, _texture(nil) {}
        ~SurfaceMetal() {}

        auto layer() { return _layer; }

        ObjectPtr<Texture> nextTexture() override {
            _drawable = [_layer nextDrawable];
            base::construct_at(&_texture, _drawable.texture);
            return ObjectPtr<Texture>(&_texture);
        }

        auto drawable() { return _drawable; }

        float width() override { return _layer.drawableSize.width; }
        float height() override { return _layer.drawableSize.height; }

    private:
        CAMetalLayer* _layer;
        id<CAMetalDrawable> _drawable;
        TextureMetal _texture;
    };

    class CommandQueueMetal final : public CommandQueue {
    public:
        explicit CommandQueueMetal(Device* device, id<MTLCommandQueue> queue)
            : _device{device}, _queue{queue} {}
        ~CommandQueueMetal() { [_queue release]; }

        id<MTLCommandQueue> commandQueue() { return _queue; }

        ObjectPtr<CommandBuffer> createCommandBuffer() override;
        void destroyCommandBuffer(ObjectPtr<CommandBuffer>) override;

    private:
        Device* _device;
        id<MTLCommandQueue> _queue;
    };

    class CommandBufferMetal final : public CommandBuffer {
    public:
        // command buffer is autoreleased
        explicit CommandBufferMetal(Device* device, id<MTLCommandBuffer> cmdbuf)
            : _device{device}, _cmdbuf{cmdbuf} {}

        id<MTLCommandBuffer> commandBuffer() { return _cmdbuf; }

        ObjectPtr<RenderPassEncoder> beginRenderPass(
            RenderPassDescriptor const& descriptor) override;
        void endRenderPass(ObjectPtr<RenderPassEncoder> encoder) override;
        void present(ObjectPtr<Surface> surface) override;
        void submit() override;

        ObjectPtr<ComputePassEncoder> beginComputePass() override;
        void endComputePass(ObjectPtr<ComputePassEncoder> encoder) override;
        void wait() override { [_cmdbuf waitUntilCompleted]; }

    private:
        Device* _device;
        id<MTLCommandBuffer> _cmdbuf;
    };

    class RenderPassEncoderMetal final : public RenderPassEncoder {
    public:
        // enocder is autoreleased
        explicit RenderPassEncoderMetal(Device* device, id<MTLRenderCommandEncoder> encoder)
            : RenderPassEncoder(device), _encoder{encoder} {}

        id<MTLRenderCommandEncoder> encoder() { return _encoder; }

        void setRenderPipeline(ObjectPtr<RenderPipeline> pipeline) override;
        void setVertexBuffer(ObjectPtr<Buffer> buffer, VertexBufferLocation const& loc) override;
        void setFragmentBuffer(ObjectPtr<Buffer> buffer, VertexBufferLocation const& loc) override {
            [_encoder setFragmentBuffer:buffer.cast<BufferMetal>()->buffer()
                                 offset:0
                                atIndex:loc.apple.buffer_index];
        }
        void setTexture(ObjectPtr<Texture> texture) override;
        void setViewport(Viewport viewport) override;
        void draw(int vertex_start, int vertex_count) override;
        void draw(int vertex_start, int vertex_count, int instance_start,
                  int instance_count) override;

    private:
        id<MTLRenderCommandEncoder> _encoder;
    };

    class ComputePipelineMetal final : public ComputePipeline {
    public:
        explicit ComputePipelineMetal(id<MTLFunction> func, id<MTLComputePipelineState> pipeline)
            : _func{func}, _pipeline{pipeline} {
            spargel_log_info("%lu", static_cast<unsigned long>(_pipeline.threadExecutionWidth));
        }
        ~ComputePipelineMetal() {
            [_func release];
            [_pipeline release];
        }

        u32 maxGroupSize() override { return _pipeline.maxTotalThreadsPerThreadgroup; }

        auto pipeline() const { return _pipeline; }

    private:
        id<MTLFunction> _func;
        id<MTLComputePipelineState> _pipeline;
    };

    class ComputePassEncoderMetal final : public ComputePassEncoder {
    public:
        explicit ComputePassEncoderMetal(id<MTLComputeCommandEncoder> encoder)
            : _encoder{encoder} {}

        void setComputePipeline(ObjectPtr<ComputePipeline> pipeline) override {
            [_encoder setComputePipelineState:pipeline.cast<ComputePipelineMetal>()->pipeline()];
        }

        void setBuffer(ObjectPtr<Buffer> buffer, VertexBufferLocation const& loc) override {
            [_encoder setBuffer:buffer.cast<BufferMetal>()->buffer()
                         offset:0
                        atIndex:loc.apple.buffer_index];
        }

        void dispatch(DispatchSize grid_size, DispatchSize group_size) override {
            [_encoder dispatchThreadgroups:MTLSizeMake(grid_size.x, grid_size.y, grid_size.z)
                     threadsPerThreadgroup:MTLSizeMake(group_size.x, group_size.y, group_size.z)];
        }

        auto encoder() const { return _encoder; }

    private:
        id<MTLComputeCommandEncoder> _encoder;
    };

    class DeviceMetal final : public Device {
    public:
        DeviceMetal();
        ~DeviceMetal() override;

        ObjectPtr<ShaderLibrary> createShaderLibrary(base::span<u8> bytes) override;
        ObjectPtr<RenderPipeline> createRenderPipeline(
            RenderPipelineDescriptor const& descriptor) override;
        ObjectPtr<Buffer> createBuffer(BufferUsage usage, base::span<u8> bytes) override;
        ObjectPtr<Buffer> createBuffer(BufferUsage usage, u32 size) override;
        ObjectPtr<Surface> createSurface(ui::window* w) override;

        ObjectPtr<Texture> createTexture(u32 width, u32 height) override;
        void destroyTexture(ObjectPtr<Texture> texture) override;

        void destroyShaderLibrary(ObjectPtr<ShaderLibrary> library) override;
        void destroyRenderPipeline(ObjectPtr<RenderPipeline> pipeline) override;
        void destroyBuffer(ObjectPtr<Buffer> b) override;

        ObjectPtr<CommandQueue> createCommandQueue() override;
        void destroyCommandQueue(ObjectPtr<CommandQueue> q) override;

        ObjectPtr<ComputePipeline> createComputePipeline(ObjectPtr<ShaderLibrary> library,
                                                         char const* entry) override {
            NSError* err;
            auto func = [library.cast<ShaderLibraryMetal>()->library()
                newFunctionWithName:[NSString stringWithUTF8String:entry]];
            return make_object<ComputePipelineMetal>(
                func, [_device newComputePipelineStateWithFunction:func error:&err]);
        }

        auto device() { return _device; }

    private:
        id<MTLDevice> _device;
    };

}  // namespace spargel::gpu
