#version 460 core

out vec4 fColor;
in vec3 vColor;

void main() {
	fColor = vec4(vColor.r, vColor.g, vColor.b, 1);
}