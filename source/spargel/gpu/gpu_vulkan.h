#pragma once

#include <spargel/base/platform.h>
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

    class DeviceVulkan;

    struct VulkanProcTable {
#define VULKAN_PROC_DECL(name) PFN_##name name;
#define VULKAN_LIBRARY_PROC(name) VULKAN_PROC_DECL(name)
#define VULKAN_GENERAL_PROC(name) VULKAN_PROC_DECL(name)
#define VULKAN_INSTANCE_PROC(name) VULKAN_PROC_DECL(name)
#define VULKAN_DEVICE_PROC(name) VULKAN_PROC_DECL(name)
#include <spargel/gpu/vulkan_procs.inc>
#undef VULKAN_DEVICE_PROC
#undef VULKAN_INSTANCE_PROC
#undef VULKAN_GENERAL_PROC
#undef VULKAN_LIBRARY_PROC
#undef VULKAN_PROC_DECL
    };

    class ShaderLibraryVulkan final : public ShaderLibrary {
    public:
        explicit ShaderLibraryVulkan(VkShaderModule shader) : _library{shader} {}
        ~ShaderLibraryVulkan();

        auto library() const { return _library; }

    private:
        VkShaderModule _library;
    };

    class RenderPipelineVulkan final : public RenderPipeline {
    public:
        explicit RenderPipelineVulkan();
        ~RenderPipelineVulkan();

        auto pipeline() { return _pipeline; }

    private:
        VkPipeline _pipeline;
    };

    class ComputePipelineVulkan final : public ComputePipeline {
    public:
        ComputePipelineVulkan(VkPipeline pipeline, VkPipelineLayout layout)
            : _pipeline{pipeline}, _layout{layout} {}

        auto pipeline() const { return _pipeline; }
        auto layout() const { return _layout; }

    private:
        VkPipeline _pipeline;
        VkPipelineLayout _layout;
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
                          base::span<base::Byte> bytes) override;

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
        VkSwapchainKHR _swapchain;
    };

    class CommandQueueVulkan final : public CommandQueue {
    public:
        explicit CommandQueueVulkan(VkQueue queue, DeviceVulkan* device);
        ~CommandQueueVulkan() {}

        auto commandQueue() { return _queue; }

        ObjectPtr<CommandBuffer> createCommandBuffer() override;
        void destroyCommandBuffer(ObjectPtr<CommandBuffer>) override;

    private:
        void createCommandPool();

        DeviceVulkan* _device;
        VulkanProcTable const* _procs;
        VkQueue _queue;
        VkCommandPool _pool;
    };

    class CommandBufferVulkan final : public CommandBuffer {
    public:
        CommandBufferVulkan(DeviceVulkan* device, VkCommandBuffer cmdbuf);

        auto commandBuffer() { return _cmdbuf; }

        ObjectPtr<RenderPassEncoder> beginRenderPass(
            RenderPassDescriptor const& descriptor) override;
        void endRenderPass(ObjectPtr<RenderPassEncoder> encoder) override;
        ObjectPtr<ComputePassEncoder> beginComputePass() override;
        void endComputePass(ObjectPtr<ComputePassEncoder> encoder) override;
        void present(ObjectPtr<Surface> surface) override;
        void submit() override;
        void wait() override;

    private:
        void beginCommandBuffer();

        DeviceVulkan* _device;
        VulkanProcTable const* _procs;
        VkCommandBuffer _cmdbuf;
    };

    class RenderPassEncoderVulkan final : public RenderPassEncoder {
    public:
        explicit RenderPassEncoderVulkan(Device* device) : RenderPassEncoder(device) {}

        void setRenderPipeline(ObjectPtr<RenderPipeline> pipeline) override;
        void setVertexBuffer(ObjectPtr<Buffer> buffer, VertexBufferLocation const& loc) override;
        void setFragmentBuffer(ObjectPtr<Buffer> buffer, VertexBufferLocation const& loc) override {
        }
        void setTexture(ObjectPtr<Texture> texture) override;
        void setViewport(Viewport viewport) override;
        void draw(int vertex_start, int vertex_count) override;
        void draw(int vertex_start, int vertex_count, int instance_start,
                  int instance_count) override;

    private:
    };

    class ComputePassEncoderVulkan final : public ComputePassEncoder {
    public:
        ComputePassEncoderVulkan(DeviceVulkan* device, VkCommandBuffer cmdbuf);

        void setComputePipeline(ObjectPtr<ComputePipeline> pipeline) override;
        void setBindGroup(u32 index, ObjectPtr<BindGroup> group) override {}
        void setBuffer(ObjectPtr<Buffer> buffer, VertexBufferLocation const& loc) override;
        void dispatch(DispatchSize grid_size, DispatchSize group_size) override;

    private:
        DeviceVulkan* _device;
        VulkanProcTable const* _procs;
        VkCommandBuffer _cmdbuf;
    };

    // One DeviceMemoryBlock corresponds to one VkDeviceMemory, i.e. one allocation.
    class DeviceMemoryBlock {
    public:
        explicit DeviceMemoryBlock(VkDeviceMemory memory) : _memory{memory} {}

    private:
        VkDeviceMemory _memory;
    };

    class BindGroupLayoutVulkan final : public BindGroupLayout {
    public:
        explicit BindGroupLayoutVulkan(VkDescriptorSetLayout layout) : _layout{layout} {}

        auto layout() const { return _layout; }

    private:
        VkDescriptorSetLayout _layout;
    };

    class DeviceMemoryAllocatorVulkan {
    public:
        explicit DeviceMemoryAllocatorVulkan(DeviceVulkan* device) : _device{device} {}

    private:
        DeviceVulkan* _device;
        base::vector<DeviceMemoryBlock> _blocks;
    };

    class DeviceVulkan final : public Device {
    public:
        DeviceVulkan();
        ~DeviceVulkan() override;

        // Device
        ObjectPtr<ShaderLibrary> createShaderLibrary(base::span<u8> bytes) override;
        void destroyShaderLibrary(ObjectPtr<ShaderLibrary> library) override;
        ObjectPtr<RenderPipeline> createRenderPipeline(
            RenderPipelineDescriptor const& descriptor) override;
        void destroyRenderPipeline(ObjectPtr<RenderPipeline> pipeline) override;
        ObjectPtr<Buffer> createBuffer(BufferUsage usage, base::span<u8> bytes) override;
        ObjectPtr<Buffer> createBuffer(BufferUsage usage, u32 size) override;
        void destroyBuffer(ObjectPtr<Buffer> b) override;
        ObjectPtr<Surface> createSurface(ui::window* w) override;
        ObjectPtr<Texture> createTexture(u32 width, u32 height) override;
        void destroyTexture(ObjectPtr<Texture> texture) override;
        ObjectPtr<CommandQueue> createCommandQueue() override;
        void destroyCommandQueue(ObjectPtr<CommandQueue> q) override;
        ObjectPtr<ComputePipeline> createComputePipeline(
            ShaderFunction func, base::span<ObjectPtr<BindGroupLayout>> layouts) override;

        ObjectPtr<BindGroupLayout> createBindGroupLayout(ShaderStage stage,
                                                         base::span<BindEntry> entries) override;
        ObjectPtr<BindGroup> createBindGroup(ObjectPtr<BindGroupLayout> layout) override {
            return nullptr;
        }

        auto device() const { return _device; }
        u32 getQueueFamilyIndex() const { return _queue_family_index; }
        VulkanProcTable const* getProcTable() const { return &_procs; }

    private:
        void loadGeneralProcs();
        void enumerateLayers();
        void enumerateInstanceExtensions();
        void selectLayers();
        void selectInstanceExtensions();
        void createInstance();
        void loadInstanceProcs();
        void createDebugMessenger();
        void enumeratePhysicalDevices();
        void queryPhysicalDeviceInfos();
        void queryQueueFamilyProperties(VkPhysicalDevice physical_device,
                                        base::vector<VkQueueFamilyProperties>& families);
        void selectPhysicalDevice();
        void enumerateDeviceExtensions();
        void selectDeviceExtensions();
        void createDevice();
        void loadDeviceProcs();
        void getQueue();
        void queryMemoryInfo();

        base::dynamic_library_handle* _library;
        VulkanProcTable _procs;

        base::vector<VkLayerProperties> _all_layers;
        base::vector<VkExtensionProperties> _all_inst_exts;
        base::vector<char const*> _use_layers;
        base::vector<char const*> _use_inst_exts;
        bool _has_portability_enumeration;
        bool _has_debug_utils;

        VkInstance _instance;
        VkDebugUtilsMessengerEXT _debug_messenger;

        base::vector<VkPhysicalDevice> _physical_devices;

        struct PhysicalDeviceInfo {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            base::vector<VkQueueFamilyProperties> queue_families;
        };
        base::vector<PhysicalDeviceInfo> _physical_device_infos;

        VkPhysicalDevice _physical_device;
        u32 _queue_family_index;

        base::vector<VkExtensionProperties> _all_dev_exts;
        base::vector<char const*> _use_dev_exts;

        VkDevice _device;
        VkQueue _queue;

        VkPhysicalDeviceMemoryProperties _memory_props;

        DeviceMemoryAllocatorVulkan _device_alloc;

        // todo: hook allocations
        [[maybe_unused]] VkAllocationCallbacks _vkalloc;
    };

}  // namespace spargel::gpu
