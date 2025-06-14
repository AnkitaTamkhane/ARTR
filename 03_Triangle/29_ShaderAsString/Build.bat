cls

del vk.exe

del vk.obj

del vk.res 

cl.exe /MD /std:c++17 /c /EHsc /I C:\VulkanSDK\Vulkan\Include /I C:\VulkanSDK\Vulkan\Include\glslang vk.cpp

rc.exe vk.rc 

link.exe vk.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib gdi32.lib user32.lib /SUBSYSTEM:WINDOWS

