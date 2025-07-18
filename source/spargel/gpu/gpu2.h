#pragma once

namespace spargel::gpu2 {
    class Device;
    class CommandQueue;
    class CommandBuffer;
    class CommandEncoder;
    class RenderEncoder;
    class ComputeEncoder;
    class Pipeline;
    class RenderPipeline;
    class ComputePipeline;

    struct RenderPipelineDescriptor;

    // A logical GPU device.
    class Device {
    public:
        virtual ~Device() = default;
        // Create a command queue.
        virtual CommandQueue* createCommandQueue() = 0;
        // Allocate a command buffer.
        //
        // Vulkan (and Metal 3) require that every command buffer can
        // only be submitted to the queue it was created with.
        // This restrction is removed in Metal 4.
        //
        // The strategy here is to ask the user for the usage of the
        // command buffer, e.g. rendering or async compute.
        //
        // TODO: Batch api.
        virtual CommandBuffer* createCommandBuffer() = 0;
        // TODO: Batch api.
        virtual RenderPipeline* createRenderPipeline() = 0;
        // TODO: Batch api.
        virtual ComputePipeline* createComputePipeline() = 0;
    };
    // An abstraction for submitting work to the device.
    class CommandQueue {
    public:
        virtual ~CommandQueue() = default;
        // Add the command buffer to the queue for GPU to execute.
        //
        // TODO: Batch api.
        virtual void submit(CommandBuffer* command_buffer) = 0;
    };
    // Several commands that are grouped together.
    //
    // There is NO guarantee on the order of execution of the commands.
    //
    // To record commands into the command buffer, use a command encoder.
    class CommandBuffer {
    public:
        virtual ~CommandBuffer() = default;
        // Start a render pass.
        virtual RenderEncoder* createRenderEncoder() = 0;
        // Start a compute pass.
        virtual ComputeEncoder* createComputeEncoder() = 0;
    };
    class CommandEncoder {
    public:
        virtual ~CommandEncoder() = default;
        // End the encoding.
        virtual void finish() = 0;
    };
    class RenderEncoder : public CommandEncoder {
    public:
        virtual ~RenderEncoder() = default;
    };
    class ComputeEncoder : public CommandEncoder {
    public:
        virtual ~ComputeEncoder() = default;
    };
    class Pipeline {
    public:
        virtual ~Pipeline() = default;
    };
    class RenderPipeline {
    public:
        virtual ~RenderPipeline() = default;
    };
    class ComputePipeline {
    public:
        virtual ~ComputePipeline() = default;
    };

    // Parameters for creating a render pipeline.
    struct RenderPipelineDescriptor {};
}  // namespace spargel::gpu2
