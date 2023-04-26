#include "render.h"

#include "sprite.h"

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdlib.h>

GLuint shaderProgramId;
GLint vertexLoc, uvLoc, tintLoc;

struct SpriteMap
{
    GLuint vao, vbo, ubo, tbo, tex_id;
    int16 vertices[12 * SPRITE_MAX * SPRITE_LAYERS];
    float uvs[12 * SPRITE_MAX * SPRITE_LAYERS];
    float tints[4 * SPRITE_MAX * SPRITE_LAYERS];
};

SpriteMap maps[2];
const char* spritemap_filenames[2] = {"pieces.png", "font.png"};

SDL_Window* window;

const char* vertexShader =
#ifdef EMSCRIPTEN
"#version 300 es\n"
"precision mediump float;\n"
"precision mediump int;\n"
"precision mediump sampler2DArray;\n"
#else
"#version 330\n"
#endif
"in vec2 vert;\n"
"in vec2 _uv;\n"
"in vec4 _tint;\n"
"out vec2 uv;\n"
"out vec4 tint;\n"
"void main()\n"
"{\n"
"    uv = _uv;\n"
"    tint = _tint;\n"
"    gl_Position = vec4(vert.x / 320.0 - 1.0, vert.y / 320.0 - 1.0, 0.0, 1.0);\n"
"}\n";

const char* fragmentShader =
#ifdef EMSCRIPTEN
"#version 300 es\n"
"precision mediump float;\n"
"precision mediump int;\n"
"precision mediump sampler2DArray;\n"
#else
"#version 330\n"
#endif
"out vec4 color;\n"
"in vec2 uv;\n"
"in vec4 tint;\n"
"uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    color = texture(tex, uv) * tint;\n"
"}\n";

GLuint compileShader(const GLchar* source, GLuint shaderType)
{
    GLuint shaderHandler;

    shaderHandler = glCreateShader(shaderType);
    glShaderSource(shaderHandler, 1, &source, 0);
    glCompileShader(shaderHandler);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shaderHandler, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderHandler, 512, 0, infoLog);
        printf("Error in compilation of shader:\n%s\n", infoLog);
        exit(1);
    };

    return shaderHandler;
}

GLuint getShaderProgramId(const char* vertexFile, const char* fragmentFile)
{
    GLuint programId, vertexHandler, fragmentHandler;

    vertexHandler = compileShader(vertexFile, GL_VERTEX_SHADER);
    fragmentHandler = compileShader(fragmentFile, GL_FRAGMENT_SHADER);

    programId = glCreateProgram();
    glAttachShader(programId, vertexHandler);
    glAttachShader(programId, fragmentHandler);
    glLinkProgram(programId);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, 512, 0, infoLog);
        printf("Error in linking of shaders:\n%s\n", infoLog);
        exit(1);
    }

    glDeleteShader(vertexHandler);
    glDeleteShader(fragmentHandler);

    return programId;
}

void render_init()
{
#ifdef EMSCRIPTEN
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // OpenGL 3+
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0); // OpenGL 3.0
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // OpenGL 3+
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3); // OpenGL 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    window = SDL_CreateWindow("GAME",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 640, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    glewInit();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);

    shaderProgramId = getShaderProgramId(vertexShader, fragmentShader);

    vertexLoc = glGetAttribLocation(shaderProgramId, "vert");
    uvLoc = glGetAttribLocation(shaderProgramId, "_uv");
    tintLoc = glGetAttribLocation(shaderProgramId, "_tint");

    for (int i = 0; i < 2; ++i)
    {
        SpriteMap* map = maps + i;

        glGenTextures(1, &map->tex_id);
        glBindTexture(GL_TEXTURE_2D, map->tex_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int imageWidth, imageHeight, n;
        unsigned char* image = stbi_load(spritemap_filenames[i], &imageWidth, &imageHeight, &n, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        stbi_image_free(image);

        glGenVertexArrays(1, &map->vao);
        glGenBuffers(1, &map->vbo);
        glGenBuffers(1, &map->ubo);
        glGenBuffers(1, &map->tbo);
        glBindVertexArray(map->vao);

        glBindBuffer(GL_ARRAY_BUFFER, map->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(map->vertices), map->vertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(vertexLoc, 2, GL_SHORT, GL_FALSE, 2 * sizeof(int16), 0);

        glBindBuffer(GL_ARRAY_BUFFER, map->ubo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(map->uvs), map->uvs, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(GLfloat), 0);

        glBindBuffer(GL_ARRAY_BUFFER, map->tbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(map->tints), map->tints, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(tintLoc, 4, GL_FLOAT, GL_TRUE, 4 * sizeof(GLfloat), 0);

        glEnableVertexAttribArray(vertexLoc);
        glEnableVertexAttribArray(uvLoc);
        glEnableVertexAttribArray(tintLoc);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shaderProgramId);
}

void render_update()
{
    glViewport(0, 0, 640, 640);
    glClearColor(0.55859375f, 0.26953125f, 0.15625f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int map_id = 0; map_id < 2; ++map_id)
    {
        SpriteMap* map = maps + map_id;
        uint32 sprite_count = 0;
        for (int layer = 0; layer < SPRITE_LAYERS; ++layer)
        {
            SpriteArray* sprite_array = &sprites[map_id][layer];
            for (int j = 0; j < sprite_array->count; ++j)
            {
                int offset = 12 * j + 12 * sprite_count;
                Sprite* sprite = sprite_array->data + j;

                if (sprite->smooth_pos)
                {
                    // top right
                    map->vertices[offset + 0] = sprite->x + sprite->tex->width * 4;
                    map->vertices[offset + 1] = sprite->y;

                    // bottom right
                    map->vertices[offset + 2] = sprite->x + sprite->tex->width * 4;
                    map->vertices[offset + 3] = sprite->y + sprite->tex->height * 4;

                    // top left
                    map->vertices[offset + 4] = sprite->x;
                    map->vertices[offset + 5] = sprite->y;

                    // bottom right
                    map->vertices[offset + 6] = sprite->x + sprite->tex->width * 4;
                    map->vertices[offset + 7] = sprite->y + sprite->tex->height * 4;

                    // bottom left
                    map->vertices[offset + 8] = sprite->x;
                    map->vertices[offset + 9] = sprite->y + sprite->tex->height * 4;

                    // top left
                    map->vertices[offset + 10] = sprite->x;
                    map->vertices[offset + 11] = sprite->y;
                }
                else
                {
                    // top right
                    map->vertices[offset + 0] = sprite->x * 4 + sprite->tex->width * 4;
                    map->vertices[offset + 1] = sprite->y * 4;

                    // bottom right
                    map->vertices[offset + 2] = sprite->x * 4 + sprite->tex->width * 4;
                    map->vertices[offset + 3] = sprite->y * 4 + sprite->tex->height * 4;

                    // top left
                    map->vertices[offset + 4] = sprite->x * 4;
                    map->vertices[offset + 5] = sprite->y * 4;

                    // bottom right
                    map->vertices[offset + 6] = sprite->x * 4 + sprite->tex->width * 4;
                    map->vertices[offset + 7] = sprite->y * 4 + sprite->tex->height * 4;

                    // bottom left
                    map->vertices[offset + 8] = sprite->x * 4;
                    map->vertices[offset + 9] = sprite->y * 4 + sprite->tex->height * 4;

                    // top left
                    map->vertices[offset + 10] = sprite->x * 4;
                    map->vertices[offset + 11] = sprite->y * 4;
                }

                map->uvs[offset + 0] = sprite->tex->u2;
                map->uvs[offset + 1] = sprite->tex->v2;

                map->uvs[offset + 2] = sprite->tex->u2;
                map->uvs[offset + 3] = sprite->tex->v1;

                map->uvs[offset + 4] = sprite->tex->u1;
                map->uvs[offset + 5] = sprite->tex->v2;

                map->uvs[offset + 6] = sprite->tex->u2;
                map->uvs[offset + 7] = sprite->tex->v1;

                map->uvs[offset + 8] = sprite->tex->u1;
                map->uvs[offset + 9] = sprite->tex->v1;

                map->uvs[offset + 10] = sprite->tex->u1;
                map->uvs[offset + 11] = sprite->tex->v2;

                for (int i = 0; i < 6; ++i)
                {
                    int sub_offset = 24 * j + 24 * sprite_count + i * 4;
                    map->tints[sub_offset + 0] = sprite->r;
                    map->tints[sub_offset + 1] = sprite->g;
                    map->tints[sub_offset + 2] = sprite->b;
                    map->tints[sub_offset + 3] = sprite->a;
                }
            }

            sprite_count += sprite_array->count;
        }

        glBindTexture(GL_TEXTURE_2D, map->tex_id);

        glBindVertexArray(map->vao);
        glBindBuffer(GL_ARRAY_BUFFER, map->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(int16) * sprite_count * 12, map->vertices);
        glBindBuffer(GL_ARRAY_BUFFER, map->ubo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * sprite_count * 12, map->uvs);
        glBindBuffer(GL_ARRAY_BUFFER, map->tbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * sprite_count * 24, map->tints);
        glDrawArrays(GL_TRIANGLES, 0, 6 * sprite_count);
    }

    SDL_GL_SwapWindow(window);
}

void render_cleanup()
{
    for (int i = 0; i < 2; ++i)
    {
        glDeleteVertexArrays(1, &maps[i].vao);
        glDeleteBuffers(1, &maps[i].vbo);
        glDeleteBuffers(1, &maps[i].ubo);
        glDeleteTextures(1, &maps[i].tex_id);
    }

    glDeleteProgram(shaderProgramId);
}
