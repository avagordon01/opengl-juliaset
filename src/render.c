#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "file.h"
#include "vector.h"

static void error_callback(int error, const char* description) {
    printf("glfw error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        if (glfwGetWindowMonitor(window)) {
            glfwSetWindowMonitor(window, NULL, 0, 0, 800, 600, 60);
        } else {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, 60);
        }
    }
}

int main() {
    //setup glfw, window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        printf("glfw error: failed to initialise\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    GLFWwindow* window = glfwCreateWindow(800, 600, "project", NULL, NULL);
    if (!window) {
        glfwTerminate();
        printf("glfw error: failed to create window\n");
        return 1;
    }
    glfwMakeContextCurrent(window);
    //glfwSwapInterval(1);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
    glfwSetKeyCallback(window, key_callback);

    //create programs
    char* error_log = calloc(4096, sizeof(char));
    const char* shader_vertex[] = {
        "#version 450\n",
        "#line 1 0\n", load("src/header.glsl"),
        "#line 1 1\n", load("src/vertex.glsl"),
    };
    GLuint program_vertex = glCreateShaderProgramv(GL_VERTEX_SHADER, sizeof(shader_vertex) / sizeof(void*), shader_vertex);
    glGetProgramInfoLog(program_vertex, 4096, NULL, error_log);
    if (error_log[0] != 0) {
        printf("vertex:\n%s\n", error_log);
        return 1;
    }
    const char* shader_fragment[] = {
        "#version 450\n",
        "#line 1 0\n", load("src/header.glsl"),
        "#line 1 1\n", load("src/fragment.glsl"),
    };
    GLuint program_fragment = glCreateShaderProgramv(GL_FRAGMENT_SHADER, sizeof(shader_fragment) / sizeof(void*), shader_fragment);
    glGetProgramInfoLog(program_fragment, 4096, NULL, error_log);
    if (error_log[0] != 0) {
        printf("fragment:\n%s\n", error_log);
        return 1;
    }
    GLuint pipeline_render;
    glGenProgramPipelines(1, &pipeline_render);
    glUseProgramStages(pipeline_render, GL_VERTEX_SHADER_BIT, program_vertex);
    glUseProgramStages(pipeline_render, GL_FRAGMENT_SHADER_BIT, program_fragment);
    glBindProgramPipeline(pipeline_render);

    //create uniform buffer
    struct inputs {
        float mouse_x, mouse_y;
        float resolution_x, resolution_y;
    } inputs;
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(inputs), &inputs, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, glGetUniformBlockIndex(program_fragment, "inputs"), ubo);

    //create vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(vao);

    glfwSetTime(0);
    for (uint64_t frames = 0;; frames++) {
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) {
            printf("%f\n", frames / glfwGetTime());
            glfwDestroyWindow(window);
            glfwTerminate();
            return 0;
        }

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        inputs.mouse_x += (mx - inputs.mouse_x) * 0.01;
        inputs.mouse_y += (my - inputs.mouse_y) * 0.01;
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        inputs.resolution_x = width;
        inputs.resolution_y = height;
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(inputs), &inputs);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        glFinish();

        glfwSwapBuffers(window);
    }
}
