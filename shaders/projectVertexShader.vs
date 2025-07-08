#version 330 core

layout(location = 0) in vec3 vertexPosition;  // Vertex position
layout(location = 1) in vec3 vertexNormal;    // Vertex normal
layout(location = 2) in vec2 vertexTexCoords; // Texture coordinates

out vec3 fragPos;    // To pass fragment position to fragment shader
out vec3 normal;     // To pass normal vector to fragment shader
out vec2 texCoords;  // To pass texture coordinates to fragment shader

uniform mat4 model;       // Model matrix
uniform mat4 view;        // View matrix
uniform mat4 projection;  // Projection matrix

void main()
{
    fragPos = vec3(model * vec4(vertexPosition, 1.0)); 
    normal = mat3(transpose(inverse(model))) * vertexNormal; 
    texCoords = vertexTexCoords; 

    gl_Position = projection * view * vec4(fragPos, 1.0); // Final position
}
