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
    vec2 user1;
    vec2 user2;
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

    // hard-coded map of texId to uvwh coords in texture atlas
    vec4 uvwh;
    if (0 == texId) { // background
        uvwh = vec4(-0.99699125814731,0.081695377707851,0.99911651130726,0.82168017871098);
    }
    else if (1 == texId) { // paddle
        uvwh = vec4(0.00041967857539781, 0.93061139238235, 0.28865594350274, 0.066830996592963);
    }
    else if (2 == texId) { // ball
        uvwh = vec4(0.31366626362349, 0.93399233307363, 0.077279650284362, 0.066458183637589);
    }

    if (xy.x == 0.5 && xy.y == -0.5) {
        fragTexCoord = vec2(uvwh.x, uvwh.y); // top-left
    }
    else if (xy.x == -0.5 && xy.y == -0.5) {
        fragTexCoord = vec2(uvwh.x+uvwh.z, uvwh.y); // top-right
    }
    else if (xy.x == -0.5 && xy.y == 0.5) {
        fragTexCoord = vec2(uvwh.x+uvwh.z, uvwh.y+uvwh.w); // bottom-right
    }
    else if (xy.x == 0.5 && xy.y == 0.5) {
        fragTexCoord = vec2(uvwh.x, uvwh.y+uvwh.w); // bottom-left
    }

    // TODO: support stretching vert along instanced WxH dims
    // if (xy.x == 0.5 && xy.y == -0.5) {
    //     fragTexCoord = vec2(ubo1.user1.x, ubo1.user1.y); // top-left
    // }
    // else if (xy.x == -0.5 && xy.y == -0.5) {
    //     fragTexCoord = vec2(ubo1.user1.x+ubo1.user2.x, ubo1.user1.y); // top-right
    // }
    // else if (xy.x == -0.5 && xy.y == 0.5) {
    //     fragTexCoord = vec2(ubo1.user1.x+ubo1.user2.x, ubo1.user1.y+ubo1.user2.y); // bottom-right
    // }
    // else if (xy.x == 0.5 && xy.y == 0.5) {
    //     fragTexCoord = vec2(ubo1.user1.x, ubo1.user1.y+ubo1.user2.y); // bottom-left
    // }
}