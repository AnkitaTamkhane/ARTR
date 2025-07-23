cls

del vk.exe

del vk.obj

del vk.res 

cl.exe /c /EHsc /I C:\VulkanSDK\Vulkan\Include vk.cpp

rc.exe vk.rc 

link.exe vk.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib gdi32.lib user32.lib /SUBSYSTEM:WINDOWS

