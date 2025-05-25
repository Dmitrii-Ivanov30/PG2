#version 460 core

in vec4 aPosition;
in vec3 aNormal;
in vec2 aTexCoord;

uniform mat4 m_m, v_m, p_m;
uniform vec3 light_dir; // Directional light direction (normalized)

out VS_OUT {
    vec3 N;
    vec3 L;
    vec3 V;
    vec2 texCoord;
} vs_out;

void main(void) {
    mat4 mv_m = v_m * m_m;
    vec4 P = mv_m * aPosition;
    
    vs_out.N = mat3(mv_m) * aNormal;
    vs_out.L = light_dir; // Use direction directly
    vs_out.V = -P.xyz;
    vs_out.texCoord = aTexCoord;
    
    gl_Position = p_m * P;
}