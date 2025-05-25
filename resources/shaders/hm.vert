#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 uP_m;
uniform mat4 uV_m;
uniform mat4 uM_m;

void main() {
    gl_Position = uP_m * uV_m * uM_m * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    Normal = mat3(transpose(inverse(uM_m))) * aNormal; // Correct normal for scaling
    FragPos = vec3(uM_m * vec4(aPos, 1.0));
}