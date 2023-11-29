#version 460 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;


uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 lightPos;

out vec3 vColor;
out vec3 vNormal;
out vec3 vPosition;
out vec3 vLightPos;

void main() {
	vColor = color;
	vNormal = mat3(transpose(inverse(viewMatrix * modelMatrix))) * normal;
	vPosition = vec3(viewMatrix * modelMatrix * vec4(position, 1.0));
	vLightPos = vec3(viewMatrix * vec4(lightPos, 1.0));
	
	
	vec4 clipSpacePosition =  projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1);

	gl_Position = clipSpacePosition;
}