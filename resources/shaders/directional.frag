#version 460 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirLight light;
uniform sampler2D tex0;

in VS_OUT {
    vec3 N;
    vec3 L;
    vec3 V;
    vec2 texCoord;
} fs_in;

void main() {
    vec3 N = normalize(fs_in.N);
    vec3 L = normalize(fs_in.L);
    vec3 V = normalize(fs_in.V);
    vec3 R = reflect(-L, N);

    // Ambient
    vec3 ambient = light.ambient * material.ambient;
    
    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // Specular
    float spec = pow(max(dot(R, V), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);
    
    vec4 texColor = texture(tex0, fs_in.texCoord);
    FragColor = vec4((ambient + diffuse) * texColor.rgb + specular, 1.0);
}