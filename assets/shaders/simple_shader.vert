#version 450

// vertex attrs
layout(location = 0) in vec2 xy;

// instanced attrs
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 rot;
layout(location = 3) in vec3 scale;
layout(location = 4) in uint texId;

layout(binding = 0) uniform UBO1 {
    mat4 proj;
    mat4 view;
    vec2 user1;
    vec2 user2;
} ubo1;

layout(location = 0) out vec2 fragTexCoord;

// generate model matrix from position, rotation, and scale
mat4 generateModelMatrix(vec3 position, vec3 rotation, vec3 scale) {
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
        scale.x, 0.0, 0.0, 0.0,
        0.0, scale.y, 0.0, 0.0,
        0.0, 0.0, scale.z, 0.0,
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

vec2 TEXTURE_WH = vec2(800,900);
float pixelsToUnitsX(uint pixels) {
    return pixels / TEXTURE_WH.x;
}
float pixelsToUnitsY(uint pixels) {
    return pixels / TEXTURE_WH.y;
}

uint GLYPH_X = 260;
uint GLYPH_Y = 822;
uint GLYPH_W = 4;
uint GLYPH_H = 6;

void main() {
    mat4 model = generateModelMatrix(pos, rot, scale);
    gl_Position = ubo1.proj * ubo1.view * model * vec4(-xy.x, xy.y, 0.0, 1.0);

    // hard-coded map of texId to uvwh coords in texture atlas
    vec4 uvwh;
    if (0 == texId) { // background 0x0 800x800
        uvwh = vec4(pixelsToUnitsX(0),pixelsToUnitsY(0),pixelsToUnitsX(800),pixelsToUnitsY(800));
    }
    else if (1 == texId) { // paddle 0x815 170x45
        uvwh = vec4(pixelsToUnitsX(0),pixelsToUnitsY(815),pixelsToUnitsX(170),pixelsToUnitsY(45));
    }
    else if (2 == texId) { // ball 190x815 45x45
        uvwh = vec4(pixelsToUnitsX(190),pixelsToUnitsY(815),pixelsToUnitsX(45),pixelsToUnitsY(45));
    }

    // pixel font glyphs 260x822 4x6
    else if (texId > 31 && texId < 128) {
        uint x = (texId - 32);
        uint y = (x / 32) - 1;
        x = x % 32;
        uvwh = vec4(
            pixelsToUnitsX(GLYPH_X + (GLYPH_W * x)),
            pixelsToUnitsY(GLYPH_Y + (GLYPH_H * y)),
            pixelsToUnitsX(GLYPH_W),
            pixelsToUnitsY(GLYPH_H));
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
}