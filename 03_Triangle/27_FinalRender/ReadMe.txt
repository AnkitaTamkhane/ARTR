
//command to create sprvi file of vertex shader
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o Shader.vert.spv Shader.vert

//command to create sprvi file of fragment shader
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o Shader.frag.spv Shader.frag


// to check magic no of shader.vert
 Format-Hex -path .\Shader.vert.spv | Select-Object -First 1 
