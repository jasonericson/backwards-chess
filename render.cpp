#include "render.h"

#include "sprite.h"

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdlib.h>

GLuint shaderProgramId, vao, vbo, ubo, textureId;
GLint vertexLoc, uvLoc;

SDL_Window* window;

int16 vertices[12 * SPRITE_MAX];
float uvs[12 * SPRITE_MAX];

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
"out vec2 uv;\n"
"void main()\n"
"{\n"
"    uv = _uv;\n"
"    gl_Position = vec4(vert.x / 80.0 - 1.0, vert.y / 80.0 - 1.0, 0.0, 1.0);\n"
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
"uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    color = texture(tex, uv);\n"
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); //OpenGL 3+
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0); //OpenGL 3.0
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

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int imageWidth, imageHeight, n;
    unsigned char* image = stbi_load("pieces.png", &imageWidth, &imageHeight, &n, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ubo);
    glBindVertexArray(vao);

    vertexLoc = glGetAttribLocation(shaderProgramId, "vert");
    uvLoc = glGetAttribLocation(shaderProgramId, "_uv");

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vertexLoc, 2, GL_SHORT, GL_FALSE, 2 * sizeof(int16), 0);

    glBindBuffer(GL_ARRAY_BUFFER, ubo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(vertexLoc);
    glEnableVertexAttribArray(uvLoc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shaderProgramId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glBindVertexArray(vao);
}

void render_update()
{
    for (int i = 0; i < sprites.count; ++i)
    {
        int offset = 12 * i;
        Sprite* sprite = sprites.data + i;

        // top right
        vertices[offset + 0] = sprite->x + 16;
        vertices[offset + 1] = sprite->y;

        // bottom right
        vertices[offset + 2] = sprite->x + 16;
        vertices[offset + 3] = sprite->y + 16;

        // top left
        vertices[offset + 4] = sprite->x;
        vertices[offset + 5] = sprite->y;

        // bottom right
        vertices[offset + 6] = sprite->x + 16;
        vertices[offset + 7] = sprite->y + 16;

        // bottom left
        vertices[offset + 8] = sprite->x;
        vertices[offset + 9] = sprite->y + 16;

        // top left
        vertices[offset + 10] = sprite->x;
        vertices[offset + 11] = sprite->y;

        uvs[offset + 0] = sprite->tex->u2;
        uvs[offset + 1] = sprite->tex->v2;

        uvs[offset + 2] = sprite->tex->u2;
        uvs[offset + 3] = sprite->tex->v1;

        uvs[offset + 4] = sprite->tex->u1;
        uvs[offset + 5] = sprite->tex->v2;

        uvs[offset + 6] = sprite->tex->u2;
        uvs[offset + 7] = sprite->tex->v1;

        uvs[offset + 8] = sprite->tex->u1;
        uvs[offset + 9] = sprite->tex->v1;

        uvs[offset + 10] = sprite->tex->u1;
        uvs[offset + 11] = sprite->tex->v2;
    }

    glViewport(0, 0, 640, 640);
    glClearColor(0.55859375f, 0.26953125f, 0.15625f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(int16) * sprites.count * 12, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, ubo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * sprites.count * 12, uvs);
    glDrawArrays(GL_TRIANGLES, 0, 6 * sprites.count);

    SDL_GL_SwapWindow(window);
}

void render_cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ubo);
    glDeleteTextures(1, &textureId);
    glDeleteProgram(shaderProgramId);
}
