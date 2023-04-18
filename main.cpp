#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdlib.h>
#include <time.h>

typedef short           int16;
typedef int             int32;
typedef unsigned short  uint16;
typedef unsigned int    uint32;


GLuint shaderProgramId, vao, vbo, ubo, textureId;
GLint vertexLoc, uvLoc;

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

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

#ifdef EMSCRIPTEN
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); //OpenGL 3+
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0); //OpenGL 3.0
#endif

    SDL_Window* win = SDL_CreateWindow("GAME",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 640, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(win);

    glewInit();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);

    float r = 1.0f;
    float g = 0.0f;
    float b = 1.0f;

    time_t t;
    srand((unsigned) time(&t));

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

    static short vertices[12];
    static float uvs[12];

    int16 posX = 140;
    int16 posY = 48;
    int16 texWidth = 16;
    int16 texHeight = 16;

    float u1 = 0.0f;
    float u2 = 0.125f;
    float v1 = 0.0f;
    float v2 = 0.125f;

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

    bool quit = false;
    while (!quit)
    {
        Uint64 start = SDL_GetPerformanceCounter();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (u1 < .1f)
                {
                    u1 = 0.125f;
                    u2 = 0.25f;
                }
                else
                {
                    u1 = 0.0f;
                    u2 = 0.125f;
                }
            }
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        posX = mouseX / 4;
        posY = (640 - mouseY) / 4;

        // top right
        vertices[0] = posX + texWidth;
        vertices[1] = posY;

        // bottom right
        vertices[2] = posX + texWidth;
        vertices[3] = posY + texHeight;

        // top left
        vertices[4] = posX;
        vertices[5] = posY;

        // bottom right
        vertices[6] = posX + texWidth;
        vertices[7] = posY + texHeight;

        // bottom left
        vertices[8] = posX;
        vertices[9] = posY + texHeight;

        // top left
        vertices[10] = posX;
        vertices[11] = posY;

        uvs[0] = u2;
        uvs[1] = v2;

        uvs[2] = u2;
        uvs[3] = v1;

        uvs[4] = u1;
        uvs[5] = v2;

        uvs[6] = u2;
        uvs[7] = v1;

        uvs[8] = u1;
        uvs[9] = v1;

        uvs[10] = u1;
        uvs[11] = v2;

        glViewport(0, 0, 640, 640);
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, ubo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(uvs), uvs);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(win);

        Uint64 end = SDL_GetPerformanceCounter();
        double elapsedMs = ((end - start) / (double)SDL_GetPerformanceFrequency()) * 1000.0f;
        SDL_Delay(floor(fmax((1000.0 / 60.0) - elapsedMs, 0.0)));
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ubo);
    glDeleteTextures(1, &textureId);
    glDeleteProgram(shaderProgramId);

    SDL_Quit();

    return 0;
}
