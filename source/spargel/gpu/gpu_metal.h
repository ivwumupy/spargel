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

        ObjectPtr<ComputePassEncoder> beginComputePass() override;
        void endComputePass(ObjectPtr<ComputePassEncoder> encoder) override;
        void wait() override { [_cmdbuf waitUntilCompleted]; }

    private:
        id<MTLCommandBuffer> _cmdbuf;
    };

    class RenderPassEncoderMetal final : public RenderPassEncoder {
    public:
        // enocder is autoreleased
        explicit RenderPassEncoderMetal(id<MTLRenderCommandEncoder> encoder) : _encoder{encoder} {}

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
            : _func{func}, _pipeline{pipeline} {}
        ~ComputePipelineMetal() {
            // [_func release];
            [_pipeline release];
        }

        // u32 maxGroupSize() override { return _pipeline.maxTotalThreadsPerThreadgroup; }

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
        // `index` is the index in the `PipelineProgram`.
        void setBindGroup(u32 index, ObjectPtr<BindGroup> group) override;
        void setBuffer(ObjectPtr<Buffer> buffer, VertexBufferLocation const& loc) override {
            [_encoder setBuffer:buffer.cast<BufferMetal>()->buffer()
                         offset:0
                        atIndex:loc.apple.buffer_index];
        }
        void dispatch(DispatchSize grid_size, DispatchSize group_size) override {
            [_encoder dispatchThreadgroups:MTLSizeMake(grid_size.x, grid_size.y, grid_size.z)
                     threadsPerThreadgroup:MTLSizeMake(group_size.x, group_size.y, group_size.z)];
        }

        void useBuffer(ObjectPtr<Buffer> buffer, bool write) override {
            [_encoder useResource:buffer.cast<BufferMetal>()->buffer()
                            usage:(write ? MTLResourceUsageWrite : MTLResourceUsageRead)];
        }

        auto encoder() const { return _encoder; }

    private:
        id<MTLComputeCommandEncoder> _encoder;
    };

    class BindGroupLayoutMetal final : public BindGroupLayout {};

    struct ArgumentInfoMetal {
        // The index of the group in the program.
        u32 id;
        // The index of the group in shader.
        u32 buffer_id;
        // The index of the argument in the group.
        u32 binding;
        BindEntryKind kind;
    };

    struct ArgumentGroupInfoMetal {
        // The index of the group in the program.
        u32 id;
        // The index of the group in shader.
        u32 buffer_id;
    };

    class PipelineProgramMetal final : public PipelineProgram {
    public:
        PipelineProgramMetal(ShaderFunction compute, base::span<PipelineArgumentGroup> groups) {
            for (usize i = 0; i < groups.count(); i++) {
                auto const& group = groups[i];
                if (group.stage.has(ShaderStage::compute)) {
                    _compute_groups.push(i, group.loc.metal.buffer_id);
                    for (auto const& arg : group.args) {
                        _compute_args.push(i, group.loc.metal.buffer_id, arg.id, arg.kind);
                    }
                }
            }
            auto lib = compute.library.cast<ShaderLibraryMetal>();
            _compute_func =
                [lib->library() newFunctionWithName:[NSString stringWithUTF8String:compute.entry]];
        }

        // Get the MTLFunction which contains the `id`-th argument group.
        id<MTLFunction> getFunction(u32 id) {
            for (auto const& group : _compute_groups) {
                if (group.id == id) {
                    return _compute_func;
                }
            }
            return nullptr;
        }

        // Get the buffer id of the argument group of index `id`.
        u32 getBufferId(u32 id) {
            for (auto const& group : _compute_groups) {
                if (group.id == id) {
                    return group.buffer_id;
                }
            }
            spargel_panic_here();
        }

        auto compute() const { return _compute_func; }

    private:
        id<MTLFunction> _compute_func;
        base::vector<ArgumentGroupInfoMetal> _compute_groups;
        base::vector<ArgumentInfoMetal> _compute_args;
    };

    class BindGroupMetal final : public BindGroup {
    public:
        BindGroupMetal(id<MTLBuffer> buffer, id<MTLArgumentEncoder> encoder,
                       PipelineProgramMetal* program)
            : _buffer{buffer}, _encoder{encoder}, _program{program} {}

        // `id` is the index of the buffer in this group.
        void setBuffer(u32 id, ObjectPtr<Buffer> buffer) override {
            [_encoder setBuffer:buffer.cast<BufferMetal>()->buffer() offset:0 atIndex:id];
        }

        auto buffer() const { return _buffer; }
        auto program() const { return _program; }

    private:
        // The argument buffer.
        id<MTLBuffer> _buffer;
        id<MTLArgumentEncoder> _encoder;
        PipelineProgramMetal* _program;
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

        ObjectPtr<ComputePipeline> createComputePipeline(
            ShaderFunction f, base::span<ObjectPtr<BindGroupLayout>> layouts) override {
            NSError* err;
            auto func = [f.library.cast<ShaderLibraryMetal>()->library()
                newFunctionWithName:[NSString stringWithUTF8String:f.entry]];
            return make_object<ComputePipelineMetal>(
                func, [_device newComputePipelineStateWithFunction:func error:&err]);
        }
        ObjectPtr<ComputePipeline> createComputePipeline2(
            ObjectPtr<PipelineProgram> program) override {
            NSError* err;
            auto func = program.cast<PipelineProgramMetal>()->compute();
            return make_object<ComputePipelineMetal>(
                func, [_device newComputePipelineStateWithFunction:func error:&err]);
        }

        ObjectPtr<BindGroupLayout> createBindGroupLayout(ShaderStage stage,
                                                         base::span<BindEntry> entries) override {
            return make_object<BindGroupLayoutMetal>();
        }
        ObjectPtr<BindGroup> createBindGroup(ObjectPtr<BindGroupLayout> layout) override {
            return nullptr;
        }

        ObjectPtr<PipelineProgram> createPipelineProgram(
            PipelineProgramDescriptor const& desc) override {
            return make_object<PipelineProgramMetal>(desc.compute, desc.groups);
        }

        // `id` is the index of the argument group in the program.
        ObjectPtr<BindGroup> createBindGroup2(ObjectPtr<PipelineProgram> p, u32 id) override {
            auto program = p.cast<PipelineProgramMetal>();
            auto func = program->getFunction(id);
            auto encoder = [func newArgumentEncoderWithBufferIndex:program->getBufferId(id)];
            auto buffer = [_device newBufferWithLength:encoder.encodedLength
                                               options:MTLResourceStorageModeShared];
            [encoder setArgumentBuffer:buffer offset:0];
            return make_object<BindGroupMetal>(buffer, encoder, program.get());
        }

        auto device() { return _device; }

    private:
        id<MTLDevice> _device;
    };

}  // namespace spargel::gpu
