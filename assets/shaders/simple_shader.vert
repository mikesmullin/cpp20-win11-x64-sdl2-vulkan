#version 450

// vertex attrs
layout(location = 0) in vec2 xy;

// instanced attrs
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 rot;
layout(location = 3) in float scale;
layout(location = 4) in uint texId;

layout(binding = 0) uniform UBO1 {
    mat4 proj;
    mat4 view;
} ubo1;

layout(location = 0) out vec2 fragTexCoord;

// generate model matrix from position, rotation, and scale
mat4 generateModelMatrix(vec3 position, vec3 rotation, float scale) {
    // Rotation matrices for each axis
    mat4 rotX = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, cos(rotation.x), -sin(rotation.x), 0.0,
        0.0, sin(rotation.x), cos(rotation.x), 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    mat4 rotY = mat4(
        cos(rotation.y), 0.0, sin(rotation.y), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(rotation.y), 0.0, cos(rotation.y), 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    mat4 rotZ = mat4(
        cos(rotation.z), -sin(rotation.z), 0.0, 0.0,
        sin(rotation.z), cos(rotation.z), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    // Combine rotation matrices
    mat4 rotationMatrix = rotX * rotY * rotZ;

    // Scale matrix
    mat4 scaleMatrix = mat4(
        scale, 0.0, 0.0, 0.0,
        0.0, scale, 0.0, 0.0,
        0.0, 0.0, scale, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    // Translation matrix
    mat4 translationMatrix = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        position.x, position.y, position.z, 1.0
    );

    // Combine translation, rotation, and scale
    mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

    return modelMatrix;
}

void main() {
    mat4 model = generateModelMatrix(pos, rot, scale);
    gl_Position = ubo1.proj * ubo1.view * model * vec4(xy, 0.0, 1.0);

    // calculate x,y,w,h of texture from texture id
    // 4x4 grid, -1 because last item in grid is black
    uint x = 0;
    uint y = 0;
    if      (texId == 0)  { x = 0; y = 0; }
    else if (texId == 1)  { x = 1; y = 0; }
    else if (texId == 2)  { x = 2; y = 0; }
    else if (texId == 3)  { x = 3; y = 0; }

    else if (texId == 4)  { x = 0; y = 1; }
    else if (texId == 5)  { x = 1; y = 1; }
    else if (texId == 6)  { x = 2; y = 1; }
    else if (texId == 7)  { x = 3; y = 1; }

    else if (texId == 8)  { x = 0; y = 2; }
    else if (texId == 9)  { x = 1; y = 2; }
    else if (texId == 10) { x = 2; y = 2; }
    else if (texId == 11) { x = 3; y = 2; }

    else if (texId == 12) { x = 0; y = 3; }
    else if (texId == 13) { x = 1; y = 3; }
    else if (texId == 14) { x = 2; y = 3; }

    if (xy.x == 0.5 && xy.y == -0.5) {
        fragTexCoord = vec2(0.2*x, 0.25*y); // top-left
    }
    else if (xy.x == -0.5 && xy.y == -0.5) {
        fragTexCoord = vec2(0.2*(x+1), 0.25*y); // top-right
    }
    else if (xy.x == -0.5 && xy.y == 0.5) {
        fragTexCoord = vec2(0.2*(x+1), 0.25*(y+1)); // bottom-right
    }
    else if (xy.x == 0.5 && xy.y == 0.5) {
        fragTexCoord = vec2(0.2*x, 0.25*(y+1)); // bottom-left
    }
}
