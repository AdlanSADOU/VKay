﻿#if !defined(VK_RENDERER)
#define VK_RENDERER

// #define NOPASS

#ifdef _WIN32
#define __func__ __FUNCTION__
#endif

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#elif __GNUC__
#define EXPORT __attribute__((visibility("default")))
// #define EXPORT
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <VkayTypes.h>

struct VkayRenderer;
struct VkayContext;

#define FRAME_BUFFER_COUNT 2

// this number reflects the texture array size set in the "shaders/textureArray.frag" shader
// both must be equal
#define MAX_TEXTURE_COUNT 80

#define SECONDS(value) (1000000000 * value)

void VkayContextInit(const char *title, uint32_t window_width, uint32_t window_height);
void VkayContextCleanup();

EXPORT void VkayRendererInit(VkayRenderer *vkr);
EXPORT bool VkayRendererBeginCommandBuffer(VkayRenderer *vkr);
EXPORT void VkayRendererEndCommandBuffer(VkayRenderer *vkr);
EXPORT void VkayRendererBeginRenderPass(VkayRenderer *vkr);
EXPORT void VkayRendererEndRenderPass(VkayRenderer *vkr);
EXPORT void VkayRendererClearColor(VkayRenderer *vkr, Color color);
EXPORT void VkayRendererPresent(VkayRenderer *vkr);
EXPORT void VkayRendererCleanup(VkayRenderer *vkr);

#if !defined(VKAY_DEBUG_ALLOCATIONS)
EXPORT VkResult VkayCreateBuffer(VkayBuffer *dst_buffer, VmaAllocator allocator, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
EXPORT void     VkayDestroyBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation);
EXPORT VkResult VkayMapMemcpyMemory(void *src, size_t size, VmaAllocator allocator, VmaAllocation allocation);
EXPORT bool     VkayAllocateBufferMemory(VkDevice device, VkPhysicalDevice gpu, VkBuffer buffer, VkDeviceMemory *memory);
EXPORT bool     VkayAllocateImageMemory(VmaAllocator allocator, VkImage image, VmaAllocation *allocation, VmaMemoryUsage usage);
#else
EXPORT VkResult VkayCreateBuffer(VkayBuffer *dst_buffer, VmaAllocator allocator, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, short line, const char *filename);
EXPORT void     VkayDestroyBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, short line, const char *filename);
EXPORT bool     VkayAllocateBufferMemory(VkDevice device, VkPhysicalDevice gpu, VkBuffer buffer, VkDeviceMemory *memory, short line, const char *filename);
EXPORT VkResult VkayMapMemcpyMemory(void *src, size_t size, VmaAllocator allocator, VmaAllocation allocation, short line, const char *filename);
#define VkayCreateBuffer(dst_buffer, allocator, alloc_size, usage, memory_usage) VkayCreateBuffer(dst_buffer, allocator, alloc_size, usage, memory_usage, __LINE__, __FILE__)
#define VkayDestroyBuffer(allocator, buffer, allocation)                         VkayDestroyBuffer(allocator, buffer, allocation, __LINE__, __FILE__)
#define VkayMapMemcpyMemory(src, size, allocator, allocation)                    VkayMapMemcpyMemory(src, size, allocator, allocation, __LINE__, __FILE__)
#define VkayAllocateBufferMemory(device, gpu, buffer, memory)                    VkayAllocateBufferMemory(device, gpu, buffer, memory, __LINE__, __FILE__)
#define VkayAllocateImageMemory(allocator, image, allocation, usage)             VkayAllocateImageMemory(allocator, image, allocation, usage, __LINE__, __FILE__)
#endif


// todo(ad): unsued
EXPORT bool     CreateUniformBuffer(VkDevice device, VkDeviceSize size, VkBuffer *out_buffer);
EXPORT void     CopyBuffer(VkCommandBuffer cmd_buffer, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
EXPORT uint32_t FindProperties(const VkPhysicalDeviceMemoryProperties *pMemoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties);
EXPORT VkResult CreateDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks *allocator, VkDescriptorSetLayout *set_layout, const VkDescriptorSetLayoutBinding *bindings, uint32_t binding_count);
EXPORT VkResult AllocateDescriptorSets(VkDevice device, VkDescriptorPool descriptor_pool, uint32_t descriptor_set_count, const VkDescriptorSetLayout *set_layouts, VkDescriptorSet *descriptor_set);


struct VkayContext
{
    SDL_Window *window = NULL;
    VkExtent2D  window_extent {};


    ///////////////////////////
    // Vulkan initialization phase types
    VkInstance                        instance;
    VkDebugUtilsMessengerEXT          debug_messenger;
    VkPhysicalDevice                  physical_device;
    VkSurfaceKHR                      surface;
    vkayPhysicalDeviceSurfaceInfo_KHR ph_device_surface_info;

    /////////////////////////
    // Pools
    VkCommandPool command_pool_graphics = {};
    VkCommandPool command_pool_compute  = {};
    ////////////////////////
    // Queues
    uint32_t graphics_queue_family;
    uint32_t compute_queue_family;
    uint32_t present_queue_family;
    VkQueue  graphics_queue;
    VkQueue  compute_queue;
    VkQueue  present_queue;
    VkBool32 is_present_queue_separate;
    // todo(ad): must also check is_compute_queue_separate

    VkDevice device;

    ReleaseQueue release_queue;

    bool is_initialized = false;
};
// this is a temporary global declaration until we figure out a better way of handling the context stuf
VkayContext *VkayGetContext();

// todo(ad): we'll stick this here for now
// but we need to figure out where to put this later
struct vkaySwapchain
{
    VkSwapchainKHR           handle            = {};
    VkFormat                 image_format      = {};
    VkColorSpaceKHR          color_space       = {};
    VkImageSubresourceRange  subresource_range = {};
    std::vector<VkImageView> image_views       = {};
    std::vector<VkImage>     images            = {};

    void create(VkDevice device, uint32_t min_image_count, VmaAllocator allocator, AllocatedImage *depth_image, VkImageView *depth_image_view);
};

struct VkayRenderer
{
    VkDevice     device;
    ReleaseQueue release_queue;

    VkQueue      graphics_queue;
    VkQueue      compute_queue;
    VkQueue      present_queue;
    VmaAllocator allocator;
    /////////////////////////
    // Swapchain
    // VkSwapchainKHR           swapchain;
    vkaySwapchain swapchain;

    VkClearValue clear_value;

    VkImageView    depth_image_view;
    AllocatedImage depth_image;
    VkFormat       depth_format;

    //
    // RenderPass & Framebuffers
    //
    VkRenderPass               render_pass;
    std::vector<VkFramebuffer> framebuffers; // these are created for a specific renderpass

    FrameData frames[FRAME_BUFFER_COUNT];
    uint32_t  frame_idx_inflight = 0;

    VkPipeline       instanced_pipeline;
    VkPipelineLayout instanced_pipeline_layout;
    VkPipeline       default_pipeline;
    VkPipelineLayout default_pipeline_layout;
    ////////////////////
    // Graphics
    VkDescriptorPool      descriptor_pool;
    VkDescriptorSetLayout set_layout_camera; // todo(ad): camera set layout

    // Do not modify this
    uint32_t              texture_array_count = 0;
    VkDescriptorSetLayout set_layout_array_of_textures;
    VkDescriptorSet       set_array_of_textures;

    std::vector<VkDescriptorImageInfo> descriptor_image_infos;
    VkSampler                          sampler;

    ////////////////////
    // Compute
    VkPipeline                   compute_pipeline;
    VkPipelineLayout             compute_pipeline_layout;
    VkDescriptorPool             descriptor_pool_compute;
    VkDescriptorSetLayout        set_layout_instanced_data;
    std::vector<VkDescriptorSet> compute_descriptor_sets;
};


static std::unordered_map<VkResult, std::string> vulkan_errors = {
    { (VkResult)0, "VK_SUCCESS" },
    { (VkResult)1, "VK_NOT_READY" },
    { (VkResult)2, "VK_TIMEOUT" },
    { (VkResult)3, "VK_EVENT_SET" },
    { (VkResult)4, "VK_EVENT_RESET" },
    { (VkResult)5, "VK_INCOMPLETE" },
    { (VkResult)-1, "VK_ERROR_OUT_OF_HOST_MEMORY" },
    { (VkResult)-2, "VK_ERROR_OUT_OF_DEVICE_MEMORY" },
    { (VkResult)-3, "VK_ERROR_INITIALIZATION_FAILED" },
    { (VkResult)-4, "VK_ERROR_DEVICE_LOST" },
    { (VkResult)-5, "VK_ERROR_MEMORY_MAP_FAILED" },
    { (VkResult)-6, "VK_ERROR_LAYER_NOT_PRESENT" },
    { (VkResult)-7, "VK_ERROR_EXTENSION_NOT_PRESENT" },
    { (VkResult)-8, "VK_ERROR_FEATURE_NOT_PRESENT" },
    { (VkResult)-9, "VK_ERROR_INCOMPATIBLE_DRIVER" },
    { (VkResult)-10, "VK_ERROR_TOO_MANY_OBJECTS" },
    { (VkResult)-11, "VK_ERROR_FORMAT_NOT_SUPPORTED" },
    { (VkResult)-12, "VK_ERROR_FRAGMENTED_POOL" },
    { (VkResult)-13, "VK_ERROR_UNKNOWN" },
    { (VkResult)-1000069000, "VK_ERROR_OUT_OF_POOL_MEMORY" },
    { (VkResult)-1000072003, "VK_ERROR_INVALID_EXTERNAL_HANDLE" },
    { (VkResult)-1000161000, "VK_ERROR_FRAGMENTATION" },
    { (VkResult)-1000257000, "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" },
    { (VkResult)-1000000000, "VK_ERROR_SURFACE_LOST_KHR" },
    { (VkResult)-1000000001, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" },
    { (VkResult)1000001003, "VK_SUBOPTIMAL_KHR" },
    { (VkResult)-1000001004, "VK_ERROR_OUT_OF_DATE_KHR" },
    { (VkResult)-1000003001, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" },
    { (VkResult)-1000011001, "VK_ERROR_VALIDATION_FAILED_EXT" },
    { (VkResult)-1000012000, "VK_ERROR_INVALID_SHADER_NV" },
    { (VkResult)-1000158000, "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT" },
    { (VkResult)-1000174001, "VK_ERROR_NOT_PERMITTED_EXT" },
    { (VkResult)-1000255000, "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" },
    { (VkResult)1000268000, "VK_THREAD_IDLE_KHR" },
    { (VkResult)1000268001, "VK_THREAD_DONE_KHR" },
    { (VkResult)1000268002, "VK_OPERATION_DEFERRED_KHR" },
    { (VkResult)1000268003, "VK_OPERATION_NOT_DEFERRED_KHR" },
    { (VkResult)1000297000, "VK_PIPELINE_COMPILE_REQUIRED_EXT" },
    { VK_ERROR_OUT_OF_POOL_MEMORY, "VK_ERROR_OUT_OF_POOL_MEMORY_KHR" },
    { VK_ERROR_INVALID_EXTERNAL_HANDLE, "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR" },
    { VK_ERROR_FRAGMENTATION, "VK_ERROR_FRAGMENTATION_EXT" },
    { VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS, "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT" },
    { VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS, "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR" },
    { VK_PIPELINE_COMPILE_REQUIRED_EXT, "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT" },
    { (VkResult)0x7FFFFFFF, "VK_RESULT_MAX_ENUM" }
};

#define VKCHECK(x)                                                                                      \
    do {                                                                                                \
        VkResult err = x;                                                                               \
        if (err) {                                                                                      \
            SDL_Log("%s:%d Detected Vulkan error: %s", __FILE__, __LINE__, vulkan_errors[err].c_str()); \
            abort();                                                                                    \
        }                                                                                               \
    } while (0)

#endif // VK_RENDERER
