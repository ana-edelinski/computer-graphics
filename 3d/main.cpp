// Autor: Ana Edelinski RA 165/2021
// Specifikacija 21: Fontana

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
 
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include "stb_image.h"

//GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);

float radius = 2.0f;       // udaljenost kamere od centra
float camAngle = 0.0f;     // ugao u XZ ravni
float camHeight = 0.0f;    // visina kamere (Y osa)

struct FallingCube {
    glm::vec3 position;
    glm::vec3 velocity;
    float rotation;
};

std::vector<FallingCube> cubes;
float cubeSpawnCooldown = 0.5f; 

float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

int main(void)
{
    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    //postavljanje openGL verzije i profila
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  //major verzija 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  //minor verzija 3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  //koristi core profil

    GLFWwindow* window;
    unsigned int wWidth = 800;
    unsigned int wHeight = 600;
    const char wTitle[] = "Fontana";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }
    
    glfwMakeContextCurrent(window);

    
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    unsigned int textureShader = createShader("texture.vert", "texture.frag");
    
    unsigned int skyboxShader = createShader("skybox.vert", "skybox.frag");
    int skyboxViewLoc = glGetUniformLocation(skyboxShader, "uV");
    int skyboxProjLoc = glGetUniformLocation(skyboxShader, "uP");

    float cube[] = {
        //  Pozicija            // UV
    // Front
    -0.5f, -0.25f,  0.5f,   0.0f, 0.0f,
     0.5f, -0.25f,  0.5f,   1.0f, 0.0f,
     0.5f,  0.25f,  0.5f,   1.0f, 1.0f,
     0.5f,  0.25f,  0.5f,   1.0f, 1.0f,
    -0.5f,  0.25f,  0.5f,   0.0f, 1.0f,
    -0.5f, -0.25f,  0.5f,   0.0f, 0.0f,

    // Back
    -0.5f, -0.25f, -0.5f,   1.0f, 0.0f,
     0.5f, -0.25f, -0.5f,   0.0f, 0.0f,
     0.5f,  0.25f, -0.5f,   0.0f, 1.0f,
     0.5f,  0.25f, -0.5f,   0.0f, 1.0f,
    -0.5f,  0.25f, -0.5f,   1.0f, 1.0f,
    -0.5f, -0.25f, -0.5f,   1.0f, 0.0f,

    // Left
    -0.5f,  0.25f,  0.5f,   1.0f, 1.0f,
    -0.5f,  0.25f, -0.5f,   0.0f, 1.0f,
    -0.5f, -0.25f, -0.5f,   0.0f, 0.0f,
    -0.5f, -0.25f, -0.5f,   0.0f, 0.0f,
    -0.5f, -0.25f,  0.5f,   1.0f, 0.0f,
    -0.5f,  0.25f,  0.5f,   1.0f, 1.0f,

    // Right
     0.5f,  0.25f,  0.5f,   0.0f, 1.0f,
     0.5f,  0.25f, -0.5f,   1.0f, 1.0f,
     0.5f, -0.25f, -0.5f,   1.0f, 0.0f,
     0.5f, -0.25f, -0.5f,   1.0f, 0.0f,
     0.5f, -0.25f,  0.5f,   0.0f, 0.0f,
     0.5f,  0.25f,  0.5f,   0.0f, 1.0f,

     // Bottom
     -0.5f, -0.25f, -0.5f,   0.0f, 1.0f,
      0.5f, -0.25f, -0.5f,   1.0f, 1.0f,
      0.5f, -0.25f,  0.5f,   1.0f, 0.0f,
      0.5f, -0.25f,  0.5f,   1.0f, 0.0f,
     -0.5f, -0.25f,  0.5f,   0.0f, 0.0f,
     -0.5f, -0.25f, -0.5f,   0.0f, 1.0f,
    };

    float waterPlane[] = {
        -0.3f,  0.251f, -0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
         0.3f,  0.251f, -0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
         0.3f,  0.251f,  0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
         0.3f,  0.251f,  0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
        -0.3f,  0.251f,  0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
        -0.3f,  0.251f, -0.3f,   0.4f, 0.8f, 1.0f, 1.0f
    };

    float groundPlane[] = {
        // pozicija            // boja (sivkasto-zelena RGBA)
        -5.0f, 0.0f, -5.0f,     0.3f, 0.6f, 0.3f, 1.0f,
         5.0f, 0.0f, -5.0f,     0.3f, 0.6f, 0.3f, 1.0f,
         5.0f, 0.0f,  5.0f,     0.3f, 0.6f, 0.3f, 1.0f,
         5.0f, 0.0f,  5.0f,     0.3f, 0.6f, 0.3f, 1.0f,
        -5.0f, 0.0f,  5.0f,     0.3f, 0.6f, 0.3f, 1.0f,
        -5.0f, 0.0f, -5.0f,     0.3f, 0.6f, 0.3f, 1.0f,
    };

    float topFrameVertices[] = {
        // Gornja traka
        -0.5f, 0.25f, -0.5f,   0.0f, 0.0f,
         0.5f, 0.25f, -0.5f,   1.0f, 0.0f,
         0.5f, 0.25f, -0.3f,   1.0f, 1.0f,

         0.5f, 0.25f, -0.3f,   1.0f, 1.0f,
        -0.5f, 0.25f, -0.3f,   0.0f, 1.0f,
        -0.5f, 0.25f, -0.5f,   0.0f, 0.0f,

        // Donja traka
        -0.5f, 0.25f, 0.3f,    0.0f, 0.0f,
         0.5f, 0.25f, 0.3f,    1.0f, 0.0f,
         0.5f, 0.25f, 0.5f,    1.0f, 1.0f,

         0.5f, 0.25f, 0.5f,    1.0f, 1.0f,
        -0.5f, 0.25f, 0.5f,    0.0f, 1.0f,
        -0.5f, 0.25f, 0.3f,    0.0f, 0.0f,

        // Leva traka
        -0.5f, 0.25f, -0.3f,   0.0f, 0.0f,
        -0.3f, 0.25f, -0.3f,   1.0f, 0.0f,
        -0.3f, 0.25f,  0.3f,   1.0f, 1.0f,

        -0.3f, 0.25f,  0.3f,   1.0f, 1.0f,
        -0.5f, 0.25f,  0.3f,   0.0f, 1.0f,
        -0.5f, 0.25f, -0.3f,   0.0f, 0.0f,

        // Desna traka
         0.3f, 0.25f, -0.3f,   0.0f, 0.0f,
         0.5f, 0.25f, -0.3f,   1.0f, 0.0f,
         0.5f, 0.25f,  0.3f,   1.0f, 1.0f,

         0.5f, 0.25f,  0.3f,   1.0f, 1.0f,
         0.3f, 0.25f,  0.3f,   0.0f, 1.0f,
         0.3f, 0.25f, -0.3f,   0.0f, 0.0f
    };

    float pillarVertices[] = {
        // Front
        -0.05f, 0.251f,  0.05f,  0.0f, 0.0f,
         0.05f, 0.251f,  0.05f,  1.0f, 0.0f,
         0.05f, 0.551f,  0.05f,  1.0f, 1.0f,
         0.05f, 0.551f,  0.05f,  1.0f, 1.0f,
        -0.05f, 0.551f,  0.05f,  0.0f, 1.0f,
        -0.05f, 0.251f,  0.05f,  0.0f, 0.0f,

        // Back
        -0.05f, 0.251f, -0.05f,  1.0f, 0.0f,
         0.05f, 0.251f, -0.05f,  0.0f, 0.0f,
         0.05f, 0.551f, -0.05f,  0.0f, 1.0f,
         0.05f, 0.551f, -0.05f,  0.0f, 1.0f,
        -0.05f, 0.551f, -0.05f,  1.0f, 1.0f,
        -0.05f, 0.251f, -0.05f,  1.0f, 0.0f,

        // Left
        -0.05f, 0.551f,  0.05f,  1.0f, 1.0f,
        -0.05f, 0.551f, -0.05f,  0.0f, 1.0f,
        -0.05f, 0.251f, -0.05f,  0.0f, 0.0f,
        -0.05f, 0.251f, -0.05f,  0.0f, 0.0f,
        -0.05f, 0.251f,  0.05f,  1.0f, 0.0f,
        -0.05f, 0.551f,  0.05f,  1.0f, 1.0f,

        // Right
         0.05f, 0.551f,  0.05f,  0.0f, 1.0f,
         0.05f, 0.551f, -0.05f,  1.0f, 1.0f,
         0.05f, 0.251f, -0.05f,  1.0f, 0.0f,
         0.05f, 0.251f, -0.05f,  1.0f, 0.0f,
         0.05f, 0.251f,  0.05f,  0.0f, 0.0f,
         0.05f, 0.551f,  0.05f,  0.0f, 1.0f,

         // Top
         -0.05f, 0.551f, -0.05f,  0.0f, 1.0f,
          0.05f, 0.551f, -0.05f,  1.0f, 1.0f,
          0.05f, 0.551f,  0.05f,  1.0f, 0.0f,
          0.05f, 0.551f,  0.05f,  1.0f, 0.0f,
         -0.05f, 0.551f,  0.05f,  0.0f, 0.0f,
         -0.05f, 0.551f, -0.05f,  0.0f, 1.0f,

         // Bottom
         -0.05f, 0.251f, -0.05f,  0.0f, 1.0f,
          0.05f, 0.251f, -0.05f,  1.0f, 1.0f,
          0.05f, 0.251f,  0.05f,  1.0f, 0.0f,
          0.05f, 0.251f,  0.05f,  1.0f, 0.0f,
         -0.05f, 0.251f,  0.05f,  0.0f, 0.0f,
         -0.05f, 0.251f, -0.05f,  0.0f, 1.0f
    };

    float pyramidVertices[] = {
        // Baza (kvadrat)
        -0.05f, 0.551f, -0.05f, 0.0f, 0.0f,
         0.05f, 0.551f, -0.05f, 1.0f, 0.0f,
         0.05f, 0.551f,  0.05f, 1.0f, 1.0f,
         0.05f, 0.551f,  0.05f, 1.0f, 1.0f,
        -0.05f, 0.551f,  0.05f, 0.0f, 1.0f,
        -0.05f, 0.551f, -0.05f, 0.0f, 0.0f,

        // Strane (trouglovi)
        0.0f, 0.65f, 0.0f,      0.5f, 1.0f,
        -0.05f, 0.551f, -0.05f, 0.0f, 0.0f,
         0.05f, 0.551f, -0.05f, 1.0f, 0.0f,

        0.0f, 0.65f, 0.0f,      0.5f, 1.0f,
         0.05f, 0.551f, -0.05f, 0.0f, 0.0f,
         0.05f, 0.551f,  0.05f, 1.0f, 0.0f,

        0.0f, 0.65f, 0.0f,      0.5f, 1.0f,
         0.05f, 0.551f,  0.05f, 0.0f, 0.0f,
        -0.05f, 0.551f,  0.05f, 1.0f, 0.0f,

        0.0f, 0.65f, 0.0f,      0.5f, 1.0f,
        -0.05f, 0.551f,  0.05f, 0.0f, 0.0f,
        -0.05f, 0.551f, -0.05f, 1.0f, 0.0f
    };

    float animatedCubes[] = {
        // Pozicija           // Boja (svetla plava, RGBA)
        // Front
        -0.5f, -0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,

        // Back
        -0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,

        // Left
        -0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,

        // Right
         0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,

         // Bottom
         -0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
          0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,

         // Top
         -0.5f,  0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
          0.5f,  0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,   0.35f, 0.65f, 1.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,   0.35f, 0.65f, 1.0f, 1.0f
    };

    unsigned int stride = (3 + 4) * sizeof(float);  //velicina jednog verteksa, 3 pozicije + 4 boje

    // osnova fontane (kvadar)
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);           // pozicija
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // UV
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // voda
    unsigned int waterVAO, waterVBO;
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);

    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(waterPlane), waterPlane, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // gornji deo osnove, okvir
    unsigned int frameVAO, frameVBO;
    glGenVertexArrays(1, &frameVAO);
    glGenBuffers(1, &frameVBO);

    glBindVertexArray(frameVAO);
    glBindBuffer(GL_ARRAY_BUFFER, frameVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(topFrameVertices), topFrameVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // pozicija
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // UV
    glEnableVertexAttribArray(1);

    // stub
    unsigned int pillarVAO, pillarVBO;
    glGenVertexArrays(1, &pillarVAO);
    glGenBuffers(1, &pillarVBO);

    glBindVertexArray(pillarVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pillarVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pillarVertices), pillarVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);           // pozicija
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // UV
    glEnableVertexAttribArray(1);

    // piramida na vrhu stuba
    unsigned int pyramidVAO, pyramidVBO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);

    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);           // pozicija
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // UV
    glEnableVertexAttribArray(1);

    // kockice koje iskacu
    unsigned int animCubeVAO, animCubeVBO;
    glGenVertexArrays(1, &animCubeVAO);
    glGenBuffers(1, &animCubeVBO);

    glBindVertexArray(animCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, animCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(animatedCubes), animatedCubes, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0); // inPos
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float))); // inCol
    glEnableVertexAttribArray(1);

    // tlo
    unsigned int groundVAO, groundVBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundPlane), groundPlane, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0); // pozicija
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float))); // boja
    glEnableVertexAttribArray(1);

    //glBindVertexArray(0);

    // nebo
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // zavrsni unbind
    //glBindVertexArray(0);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    glm::mat4 model = glm::mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    unsigned int modelLoc = glGetUniformLocation(unifiedShader, "uM");  //lokacija uniforme
    
    glm::mat4 view; //Matrica pogleda (kamere)
    //view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
    //view = glm::lookAt(glm::vec3(1.5f, 1.0f, 2.0f),  // kamera iz ugla
    //    glm::vec3(0.0f, 0.0f, 0.0f),  // centar scene
    //    glm::vec3(0.0f, 1.0f, 0.0f)); // Y je gore
    unsigned int viewLoc = glGetUniformLocation(unifiedShader, "uV");
    
    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    unsigned int projectionLoc = glGetUniformLocation(unifiedShader, "uP");


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    glUseProgram(unifiedShader); //Slanje default vrijednosti uniformi
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); //view
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));    //ortogonalna default

    glClearColor(0.5, 0.5, 0.5, 1.0);   //boja pozadine
    glCullFace(GL_BACK);//Biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)

    glEnable(GL_DEPTH_TEST);

    unsigned int stoneTexture;
    glGenTextures(1, &stoneTexture);
    glBindTexture(GL_TEXTURE_2D, stoneTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("resources/stone.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Neuspesno ucitavanje teksture" << std::endl;
    }
    stbi_image_free(data);


    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);  //esc zatvara prozor
        }

        //Testiranje dubine
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            glEnable(GL_DEPTH_TEST); //Ukljucivanje testiranja Z bafera
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            glDisable(GL_DEPTH_TEST);
        }

        //Odstranjivanje lica (Prethodno smo podesili koje lice uklanjamo sa glCullFace)
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            glEnable(GL_CULL_FACE);
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            glDisable(GL_CULL_FACE);
        }

        // Obrada tastature 
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camAngle -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camAngle += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camHeight += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camHeight -= 0.02f;

        if (glfwGetKey(window, GLFW_KEY_KP_ADD) || glfwGetKey(window, GLFW_KEY_E)  == GLFW_PRESS)
            radius -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            radius += 0.02f;

        // Ažuriranje pozicije kamere
        float camX = sin(camAngle) * radius;
        float camZ = cos(camAngle) * radius;

        view = glm::lookAt(glm::vec3(camX, camHeight, camZ),
            glm::vec3(0.0f, 0.0f, 0.0f),     // gleda u centar
            glm::vec3(0.0f, 1.0f, 0.0f));    // Y je gore

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // nebo
        glDepthFunc(GL_LEQUAL);  
        glDepthMask(GL_FALSE);

        glUseProgram(skyboxShader);
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // bez translacije
        glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, glm::value_ptr(skyboxView));
        glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(projectionP));

        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);   // <<< obavezno
        glDepthFunc(GL_LESS);   // <<< obavezno

        // tlo
        glUseProgram(unifiedShader);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // voda 
        glUseProgram(unifiedShader);
        glBindVertexArray(waterVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // kvadar - osnova fontane
        glUseProgram(textureShader);
        glUniformMatrix4fv(glGetUniformLocation(textureShader, "uM"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(textureShader, "uV"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(textureShader, "uP"), 1, GL_FALSE, glm::value_ptr(projectionP));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stoneTexture);
        glUniform1i(glGetUniformLocation(textureShader, "uTex"), 0);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // gornji okvir
        glBindVertexArray(frameVAO);
        glDrawArrays(GL_TRIANGLES, 0, 24);

        // stub
        glBindVertexArray(pillarVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // piramida na vrhu stuba
        glBindVertexArray(pyramidVAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        // kockice koje iskacu
        float deltaTime = 0.016f; // fiksni frame (~60 FPS)
        glUseProgram(unifiedShader);

        // Dodavanje novih kockica iz vrha piramide
        cubeSpawnCooldown -= deltaTime;
        if (cubeSpawnCooldown <= 0.0f) {
            FallingCube newCube;
            newCube.position = glm::vec3(0.0f, 0.65f, 0.0f);

            // Brzina: navise + u stranu (nasumicno)
            newCube.velocity = glm::vec3(
                (rand() % 100 - 50) / 350.0f,     // X: -0.143 do 0.143
                0.55f + (rand() % 100) / 800.0f,  // Y: 0.55 do 0.675
                (rand() % 100 - 50) / 350.0f      // Z: -0.143 do 0.143
            );

            newCube.rotation = 0.0f;
            cubes.push_back(newCube);
            cubeSpawnCooldown = 0.05f;  // nova svakih 0.05s
        }

        // Azuriranje i crtanje kockica
        for (int i = 0; i < cubes.size(); ++i) {
            // Ažuriraj poziciju po paraboli
            cubes[i].position += cubes[i].velocity * deltaTime;

            // Gravitacija
            cubes[i].velocity.y -= 1.0f * deltaTime;

            // Ogranici da ne padnu ispod vode
            if (cubes[i].position.y < 0.251f) {
                cubes[i].position.y = 0.251f;
                cubes[i].velocity = glm::vec3(0.0f); // stop kretanja
            }

            // Rotacija
            cubes[i].rotation += glm::radians(90.0f) * deltaTime;

            // Model matrica
            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, cubes[i].position);
            cubeModel = glm::rotate(cubeModel, cubes[i].rotation, glm::vec3(1.0f, 1.0f, 0.0f));
            cubeModel = glm::scale(cubeModel, glm::vec3(0.07f));  // male kocke

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel));
            glBindVertexArray(animCubeVAO);  // isti kao za bazu
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Ogranici broj kocki u memoriji
        if (cubes.size() > 50)
            cubes.erase(cubes.begin());

        glfwSwapBuffers(window);
        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);

    glDeleteVertexArrays(1, &frameVAO);
    glDeleteBuffers(1, &frameVBO);

    glDeleteVertexArrays(1, &pillarVAO);
    glDeleteBuffers(1, &pillarVBO);

    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteBuffers(1, &pyramidVBO);

    glDeleteVertexArrays(1, &animCubeVAO);
    glDeleteBuffers(1, &animCubeVBO);

    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glDeleteTextures(1, &stoneTexture);

    glDeleteProgram(unifiedShader);
    glDeleteProgram(textureShader);
    glDeleteProgram(skyboxShader);

    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
     std::string temp = ss.str();
     const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);
    
    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
