#include "render.h"

#include "sprite.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
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
    uint16 width, height;
};

SpriteMap maps[NUM_MAPS];

int16 vertices[2 * 6 * SPRITE_MAX];
float uvs[2 * 6 * SPRITE_MAX];
float tints[4 * 6 * SPRITE_MAX];

SDL_Window* window;

#ifdef EMSCRIPTEN
const char* glsl_version = "#version 300 es";
#else
const char* glsl_version = "#version 330";
#endif

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

    window = SDL_CreateWindow("Backwards Chess",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 640, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    glewInit();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);

    shaderProgramId = getShaderProgramId(vertexShader, fragmentShader);

    vertexLoc = glGetAttribLocation(shaderProgramId, "vert");
    uvLoc = glGetAttribLocation(shaderProgramId, "_uv");
    tintLoc = glGetAttribLocation(shaderProgramId, "_tint");

    for (int i = 0; i < NUM_MAPS; ++i)
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

        map->width = imageWidth;
        map->height = imageHeight;

        glGenVertexArrays(1, &map->vao);
        glGenBuffers(1, &map->vbo);
        glGenBuffers(1, &map->ubo);
        glGenBuffers(1, &map->tbo);
        glBindVertexArray(map->vao);

        glBindBuffer(GL_ARRAY_BUFFER, map->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(vertexLoc, 2, GL_SHORT, GL_FALSE, 2 * sizeof(int16), 0);

        glBindBuffer(GL_ARRAY_BUFFER, map->ubo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(GLfloat), 0);

        glBindBuffer(GL_ARRAY_BUFFER, map->tbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tints), tints, GL_DYNAMIC_DRAW);
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
    ImGui::Render();
    glViewport(0, 0, 640, 640);
    glClearColor(0.55859375f, 0.26953125f, 0.15625f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (sprites.count > 0)
    {
        SpriteMapId last_map_id = sprites.data[0].tex->map_id;
        SpriteMap* map = maps + last_map_id;

        uint32 sprite_batch_count = 0;
        for (int i = 0; i < sprites.count; ++i)
        {
            Sprite* sprite = sprites.data + i;
            if (sprite->tex->map_id != last_map_id)
            {
                if (sprite_batch_count > 0)
                {
                    glBindTexture(GL_TEXTURE_2D, map->tex_id);

                    glBindVertexArray(map->vao);
                    glBindBuffer(GL_ARRAY_BUFFER, map->vbo);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(int16) * sprite_batch_count * 2 * 6, vertices);
                    glBindBuffer(GL_ARRAY_BUFFER, map->ubo);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * sprite_batch_count * 2 * 6, uvs);
                    glBindBuffer(GL_ARRAY_BUFFER, map->tbo);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * sprite_batch_count * 4 * 6, tints);
                    glDrawArrays(GL_TRIANGLES, 0, 6 * sprite_batch_count);

                    sprite_batch_count = 0;
                }

                map = maps + sprite->tex->map_id;
            }

            int offset = 2 * 6 * sprite_batch_count;

            int16 x = sprite->smooth_pos ? sprite->x : sprite->x * 4;
            int16 y = sprite->smooth_pos ? sprite->y : sprite->y * 4;

            int16 right = x + sprite->tex->width  * 4 * 0.5f * (1.0f + sprite->w);
            int16 left  = x + sprite->tex->width  * 4 * 0.5f * (1.0f - sprite->w);
            int16 top   = y + sprite->tex->height * 4 * 0.5f * (1.0f - sprite->h);
            int16 bot   = y + sprite->tex->height * 4 * 0.5f * (1.0f + sprite->h);

            // top right
            vertices[offset + 0] = right;
            vertices[offset + 1] = top;

            // bottom right
            vertices[offset + 2] = right;
            vertices[offset + 3] = bot;

            // top left
            vertices[offset + 4] = left;
            vertices[offset + 5] = top;

            // bottom right
            vertices[offset + 6] = right;
            vertices[offset + 7] = bot;

            // bottom left
            vertices[offset + 8] = left;
            vertices[offset + 9] = bot;

            // top left
            vertices[offset + 10] = left;
            vertices[offset + 11] = top;

            uvs[offset + 0] = sprite->tex->u2 / (float)map->width;
            uvs[offset + 1] = sprite->tex->v2 / (float)map->height;

            uvs[offset + 2] = sprite->tex->u2 / (float)map->width;
            uvs[offset + 3] = sprite->tex->v1 / (float)map->height;

            uvs[offset + 4] = sprite->tex->u1 / (float)map->width;
            uvs[offset + 5] = sprite->tex->v2 / (float)map->height;

            uvs[offset + 6] = sprite->tex->u2 / (float)map->width;
            uvs[offset + 7] = sprite->tex->v1 / (float)map->height;

            uvs[offset + 8] = sprite->tex->u1 / (float)map->width;
            uvs[offset + 9] = sprite->tex->v1 / (float)map->height;

            uvs[offset + 10] = sprite->tex->u1 / (float)map->width;
            uvs[offset + 11] = sprite->tex->v2 / (float)map->height;

            for (int j = 0; j < 6; ++j)
            {
                int sub_offset = 4 * 6 * sprite_batch_count + j * 4;
                tints[sub_offset + 0] = sprite->r;
                tints[sub_offset + 1] = sprite->g;
                tints[sub_offset + 2] = sprite->b;
                tints[sub_offset + 3] = sprite->a;
            }

            last_map_id = sprite->tex->map_id;
            ++sprite_batch_count;
        }

        if (sprite_batch_count > 0)
        {
            glBindTexture(GL_TEXTURE_2D, map->tex_id);

            glBindVertexArray(map->vao);
            glBindBuffer(GL_ARRAY_BUFFER, map->vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(int16) * sprite_batch_count * 2 * 6, vertices);
            glBindBuffer(GL_ARRAY_BUFFER, map->ubo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * sprite_batch_count * 2 * 6, uvs);
            glBindBuffer(GL_ARRAY_BUFFER, map->tbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * sprite_batch_count * 4 * 6, tints);
            glDrawArrays(GL_TRIANGLES, 0, 6 * sprite_batch_count);
        }
    }

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

void render_cleanup()
{
    for (int i = 0; i < NUM_MAPS; ++i)
    {
        glDeleteVertexArrays(1, &maps[i].vao);
        glDeleteBuffers(1, &maps[i].vbo);
        glDeleteBuffers(1, &maps[i].ubo);
        glDeleteTextures(1, &maps[i].tex_id);
    }

    glDeleteProgram(shaderProgramId);
}
