#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec4 out_color;
void main(void) 
{
	//code
	fragColor = out_color;

}

