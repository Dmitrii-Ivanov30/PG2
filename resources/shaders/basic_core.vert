#version 460 core
in vec3 aPos;

uniform mat4 uP_m = mat4(1.0f);  # transform from 3D camera space to 2D display space
uniform mat4 uV_m = mat4(1.0f);  # transform from model space to 3D camera space
uniform mat4 uM_m = mat4(1.0f);  # transform from world space to model space

void main()
{
    // Outputs the positions/coordinates of all vertices
    gl_Position = uP_m * uV_m * uM_m * vec4(aPos, 1.0f);
}
