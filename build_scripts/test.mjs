const log = console.log

// #region deps
const SDL_Init = () => { log('SDL_Init') }
const SDL_CreateWindow = () => { log('SDL_CreateWindow') }
const SDL_Vulkan_GetInstanceExtensions = () => { log('SDL_Vulkan_GetInstanceExtensions') }
const memcpy = () => { log('memcpy') }
const SDL_Vulkan_CreateSurface = () => { log('SDL_Vulkan_CreateSurface') }
const SDL_Vulkan_GetDrawableSize = () => { log('SDL_Vulkan_GetDrawableSize') }
const stbi_image_free = () => { log('stbi_image_free') }
const stbi_load = () => { log('stbi_load') }
const vkAcquireNextImageKHR = () => { log('vkAcquireNextImageKHR') }
const vkAllocateCommandBuffers = () => { log('vkAllocateCommandBuffers') }
const vkAllocateMemory = () => { log('vkAllocateMemory') }
const vkBeginCommandBuffer = () => { log('vkBeginCommandBuffer') }
const vkBindBufferMemory = () => { log('vkBindBufferMemory') }
const vkBindImageMemory = () => { log('vkBindImageMemory') }
const vkCmdBeginRenderPass = () => { log('vkCmdBeginRenderPass') }
const vkCmdBindDescriptorSets = () => { log('vkCmdBindDescriptorSets') }
const vkCmdBindIndexBuffer = () => { log('vkCmdBindIndexBuffer') }
const vkCmdBindPipeline = () => { log('vkCmdBindPipeline') }
const vkCmdBindVertexBuffers = () => { log('vkCmdBindVertexBuffers') }
const vkCmdCopyBuffer = () => { log('vkCmdCopyBuffer') }
const vkCmdCopyBufferToImage = () => { log('vkCmdCopyBufferToImage') }
const vkCmdDrawIndexed = () => { log('vkCmdDrawIndexed') }
const vkCmdEndRenderPass = () => { log('vkCmdEndRenderPass') }
const vkCmdPipelineBarrier = () => { log('vkCmdPipelineBarrier') }
const vkCmdSetScissor = () => { log('vkCmdSetScissor') }
const vkCmdSetViewport = () => { log('vkCmdSetViewport') }
const vkCreateBuffer = () => { log('vkCreateBuffer') }
const vkCreateCommandPool = () => { log('vkCreateCommandPool') }
const vkCreateDescriptorPool = () => { log('vkCreateDescriptorPool') }
const vkCreateDescriptorSetLayout = () => { log('vkCreateDescriptorSetLayout') }
const vkCreateDevice = () => { log('vkCreateDevice') }
const vkCreateFence = () => { log('vkCreateFence') }
const vkCreateFramebuffer = () => { log('vkCreateFramebuffer') }
const vkCreateGraphicsPipelines = () => { log('vkCreateGraphicsPipelines') }
const vkCreateImage = () => { log('vkCreateImage') }
const vkCreateImageView = () => { log('vkCreateImageView') }
const vkCreateInstance = () => { log('vkCreateInstance') }
const vkCreatePipelineLayout = () => { log('vkCreatePipelineLayout') }
const vkCreateRenderPass = () => { log('vkCreateRenderPass') }
const vkCreateSampler = () => { log('vkCreateSampler') }
const vkCreateSemaphore = () => { log('vkCreateSemaphore') }
const vkCreateShaderModule = () => { log('vkCreateShaderModule') }
const vkCreateSwapchainKHR = () => { log('vkCreateSwapchainKHR') }
const vkDestroyBuffer = () => { log('vkDestroyBuffer') }
const vkDestroyFramebuffer = () => { log('vkDestroyFramebuffer') }
const vkDestroyImageView = () => { log('vkDestroyImageView') }
const vkDestroyShaderModule = () => { log('vkDestroyShaderModule') }
const vkDestroySwapchainKHR = () => { log('vkDestroySwapchainKHR') }
const vkDeviceWaitIdle = () => { log('vkDeviceWaitIdle') }
const vkEndCommandBuffer = () => { log('vkEndCommandBuffer') }
const vkEnumerateInstanceExtensionProperties = () => { log('vkEnumerateInstanceExtensionProperties') }
const vkEnumerateInstanceLayerProperties = () => { log('vkEnumerateInstanceLayerProperties') }
const vkEnumeratePhysicalDevices = () => { log('vkEnumeratePhysicalDevices') }
const vkFreeCommandBuffers = () => { log('vkFreeCommandBuffers') }
const vkFreeMemory = () => { log('vkFreeMemory') }
const vkGetBufferMemoryRequirements = () => { log('vkGetBufferMemoryRequirements') }
const vkGetDeviceQueue = () => { log('vkGetDeviceQueue') }
const vkGetImageMemoryRequirements = () => { log('vkGetImageMemoryRequirements') }
const vkGetPhysicalDeviceMemoryProperties = () => { log('vkGetPhysicalDeviceMemoryProperties') }
const vkGetPhysicalDeviceProperties = () => { log('vkGetPhysicalDeviceProperties') }
const vkGetPhysicalDeviceQueueFamilyProperties = () => { log('vkGetPhysicalDeviceQueueFamilyProperties') }
const vkGetPhysicalDeviceSurfaceCapabilitiesKHR = () => { log('vkGetPhysicalDeviceSurfaceCapabilitiesKHR') }
const vkGetPhysicalDeviceSurfaceFormatsKHR = () => { log('vkGetPhysicalDeviceSurfaceFormatsKHR') }
const vkGetPhysicalDeviceSurfacePresentModesKHR = () => { log('vkGetPhysicalDeviceSurfacePresentModesKHR') }
const vkGetPhysicalDeviceSurfaceSupportKHR = () => { log('vkGetPhysicalDeviceSurfaceSupportKHR') }
const vkGetSwapchainImagesKHR = () => { log('vkGetSwapchainImagesKHR') }
const vkMapMemory = () => { log('vkMapMemory') }
const vkQueuePresentKHR = () => { log('vkQueuePresentKHR') }
const vkQueueSubmit = () => { log('vkQueueSubmit') }
const vkQueueWaitIdle = () => { log('vkQueueWaitIdle') }
const vkResetCommandBuffer = () => { log('vkResetCommandBuffer') }
const vkResetFences = () => { log('vkResetFences') }
const vkUnmapMemory = () => { log('vkUnmapMemory') }
const vkUpdateDescriptorSets = () => { log('vkUpdateDescriptorSets') }
const vkWaitForFences = () => { log('vkWaitForFences') }
// #endregion

// #region api
const Window__Begin = () => {
  SDL_Init()
  SDL_CreateWindow()
  SDL_Vulkan_GetInstanceExtensions()
}

const Vulkan__AssertDriverValidationLayersSupported = () => {
  const Vulkan__CheckInstanceLayers = () => {
    vkEnumerateInstanceLayerProperties()
  }
  Vulkan__CheckInstanceLayers()
}
const Vulkan__AssertDriverExtensionsSupported = () => {
  const Vulkan__CheckInstanceExtensions = () => {
    vkEnumerateInstanceExtensionProperties()
  }
  Vulkan__CheckInstanceExtensions()
}
const Vulkan__CreateInstance = () => {
  vkCreateInstance()
}
const Vulkan__UsePhysicalDevice = () => {
  vkEnumeratePhysicalDevices()
  vkGetPhysicalDeviceProperties()
}
const Window__Bind = () => {
  SDL_Vulkan_CreateSurface()
}
const Window__GetDrawableAreaExtentBounds = () => {
  SDL_Vulkan_GetDrawableSize()
}
const Vulkan__CreateSwapChain = () => {
  vkCreateSwapchainKHR()
  vkGetSwapchainImagesKHR()
}
const Vulkan__InitSwapChain = () => {
  const Vulkan__AssertSwapChainSupport = () => {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
    vkGetPhysicalDeviceSurfaceFormatsKHR()
    vkGetPhysicalDeviceSurfacePresentModesKHR()
  }
  Vulkan__AssertSwapChainSupport()
  const Vulkan__UseLogicalDevice = () => {
    const Vulkan__LocateQueueFamilies = () => {
      vkGetPhysicalDeviceQueueFamilyProperties()
      vkGetPhysicalDeviceSurfaceSupportKHR()
    }
    Vulkan__LocateQueueFamilies()
    vkCreateDevice()
    vkGetDeviceQueue()
  }
  Vulkan__UseLogicalDevice()
  Vulkan__CreateSwapChain()
}
const Vulkan__SetVertexBufferData = () => { }
const Vulkan__CreateImageViews = () => {
  vkCreateImageView()
}
const Vulkan__CreateRenderPass = () => {
  vkCreateRenderPass()
}
const Vulkan__CreateDescriptorSetLayout = () => {
  vkCreateDescriptorSetLayout()
}
const Vulkan__CreateGraphicsPipeline = () => {
  const Vulkan__CreateShaderModule = () => {
    vkCreateShaderModule()
  }
  Vulkan__CreateShaderModule()
  vkCreatePipelineLayout()
  vkCreateGraphicsPipelines()
  const Vulkan__DestroyShaderModule = () => {
    vkDestroyShaderModule()
  }
  Vulkan__DestroyShaderModule()
}
const Vulkan__CreateFrameBuffers = () => {
  vkCreateFramebuffer()
}
const Vulkan__CreateCommandPool = () => {
  vkCreateCommandPool()
}
const Vulkan__FindMemoryType = () => {
  vkGetPhysicalDeviceMemoryProperties()
}
const Vulkan__BeginSingleTimeCommands = () => {
  vkAllocateCommandBuffers()
  vkBeginCommandBuffer()
}
const Vulkan__EndSingleTimeCommands = () => {
  vkEndCommandBuffer()
  vkQueueSubmit()
  vkQueueWaitIdle()
  vkFreeCommandBuffers()
}
const Vulkan__CreateTextureImage = () => {
  stbi_load()
  const Vulkan__CreateBuffer = () => {
    vkCreateBuffer()
    vkGetBufferMemoryRequirements()
    Vulkan__FindMemoryType()
    vkAllocateMemory()
    vkBindBufferMemory()
  }
  Vulkan__CreateBuffer()
  vkMapMemory()
  memcpy()
  vkUnmapMemory()
  stbi_image_free()
  const Vulkan__CreateImage = () => {
    vkCreateImage()
    vkGetImageMemoryRequirements()
    Vulkan__FindMemoryType()
    vkAllocateMemory()
    vkBindImageMemory()
  }
  Vulkan__CreateImage()
  const Vulkan__TransitionImageLayout = () => {
    Vulkan__BeginSingleTimeCommands()
    vkCmdPipelineBarrier()
    Vulkan__EndSingleTimeCommands()
    const Vulkan__CopyBufferToImage = () => {
      Vulkan__BeginSingleTimeCommands()
      vkCmdCopyBufferToImage()
      Vulkan__EndSingleTimeCommands()
    }
    Vulkan__CopyBufferToImage()
    vkDestroyBuffer()
    vkFreeMemory()
  }
  Vulkan__TransitionImageLayout()
}
const Vulkan__CreateTextureImageView = () => {
  const Vulkan__CreateImageView = () => {
    vkCreateImageView()
  }
  Vulkan__CreateImageView()
}
const Vulkan__CreateTextureSampler = () => {
  vkGetPhysicalDeviceProperties()
  vkCreateSampler()
}
const Vulkan__CreateBuffer = () => {
  vkMapMemory()
  memcpy()
  vkUnmapMemory()
}
const Vulkan__CopyBuffer = () => {
  Vulkan__BeginSingleTimeCommands()
  vkCmdCopyBuffer()
  Vulkan__EndSingleTimeCommands()
}
const Vulkan__CreateVertexBuffer = () => {
  Vulkan__CreateBuffer()
  Vulkan__CopyBuffer()
  vkDestroyBuffer()
  vkFreeMemory()
}
const Vulkan__CreateIndexBuffer = () => {
  Vulkan__CreateBuffer()
  vkMapMemory()
  memcpy()
  vkUnmapMemory()
  Vulkan__CreateBuffer()
  Vulkan__CopyBuffer()
  vkDestroyBuffer()
  vkFreeMemory()
}
const Vulkan__CreateUniformBuffers = () => {
  Vulkan__CreateBuffer()
  vkMapMemory()
}
const Vulkan__CreateDescriptorPool = () => {
  vkCreateDescriptorPool()
}
const Vulkan__CreateDescriptorSets = () => {
  vkUpdateDescriptorSets()
}
const Vulkan__CreateCommandBuffers = () => {
  vkAllocateCommandBuffers()
}
const Vulkan__CreateSyncObjects = () => {
  vkCreateSemaphore()
  vkCreateFence()
}
const Vulkan__UpdateVertexBuffer = () => {
  Vulkan__CreateBuffer()
  vkMapMemory()
  memcpy()
  vkUnmapMemory()
  Vulkan__CopyBuffer()
  vkDestroyBuffer()
  vkFreeMemory()
}
const Vulkan__UpdateUniformBuffer = () => {
  memcpy()
}
const Window__RenderLoop = (cb) => {
  const Vulkan__RecreateSwapChain = () => {
    Vulkan__DeviceWaitIdle()
    const Vulkan__CleanupSwapChain = () => {
      vkDestroyFramebuffer()
      vkDestroyImageView()
      vkDestroySwapchainKHR()
    }
    Vulkan__CleanupSwapChain()
    Vulkan__CreateSwapChain()
    Vulkan__CreateImageViews()
    Vulkan__CreateFrameBuffers()
  }
  const Vulkan__AwaitNextFrame = () => {
    vkWaitForFences()
    vkAcquireNextImageKHR()

    Vulkan__RecreateSwapChain()
  }
  Vulkan__AwaitNextFrame()
  const Vulkan__DrawFrame = () => {
    vkResetFences()
    vkResetCommandBuffer()
    const Vulkan__RecordCommandBuffer = () => {
      vkBeginCommandBuffer()
      vkCmdBeginRenderPass()
      vkCmdBindPipeline()
      vkCmdBindVertexBuffers()
      vkCmdBindIndexBuffer()
      vkCmdSetViewport()
      vkCmdSetScissor()
      vkCmdBindDescriptorSets()
      vkCmdDrawIndexed()
      vkCmdEndRenderPass()
      vkEndCommandBuffer()
    }
    Vulkan__RecordCommandBuffer()
    vkQueueSubmit()
    vkQueuePresentKHR()
    Vulkan__RecreateSwapChain()
  }
  cb()
  Vulkan__DrawFrame()
}
const Vulkan__DeviceWaitIdle = () => {
  vkDeviceWaitIdle()
}
const Vulkan__Cleanup = () => { }
const Window__End = () => { }
// #endregion

// #region program
log('main')

//auto w = mks::Window{};
Window__Begin();
Vulkan__AssertDriverValidationLayersSupported();
Vulkan__AssertDriverExtensionsSupported();
Vulkan__CreateInstance();
Vulkan__UsePhysicalDevice();
Window__Bind();
Window__GetDrawableAreaExtentBounds();

Vulkan__InitSwapChain();
Vulkan__SetVertexBufferData();

Vulkan__CreateImageViews();          // pre
Vulkan__CreateRenderPass();          // pre
Vulkan__CreateDescriptorSetLayout(); // pre
Vulkan__CreateGraphicsPipeline();    // pre
Vulkan__CreateFrameBuffers();        // pre
Vulkan__CreateCommandPool();         // setting
Vulkan__CreateTextureImage();        // cmd
Vulkan__CreateTextureImageView();    // setting
Vulkan__CreateTextureSampler();      // setting
Vulkan__CreateVertexBuffer();        // cmd
Vulkan__CreateIndexBuffer();         // cmd
Vulkan__CreateUniformBuffers();      // cmd
Vulkan__CreateDescriptorPool();      // setting
Vulkan__CreateDescriptorSets();      // setting
Vulkan__CreateCommandBuffers();      // these theoretically would get used in render loop by me
Vulkan__CreateSyncObjects();         // fence and semaphores

Window__RenderLoop(() => {
  Vulkan__UpdateVertexBuffer();
  Vulkan__UpdateUniformBuffer();
})

Vulkan__DeviceWaitIdle();
Vulkan__Cleanup();
Window__End();

// #endregion
