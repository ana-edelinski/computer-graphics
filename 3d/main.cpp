// Autor: Nedeljko Tesanovic
// Opis: Testiranje dubine, Uklanjanje lica, Transformacije, Prostori i Projekcije

#define _CRT_SECURE_NO_WARNINGS
 
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

//GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);

float radius = 2.0f;       // udaljenost kamere od centra
float camAngle = 0.0f;     // ugao u XZ ravni
float camHeight = 0.0f;    // visina kamere (Y osa)


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
    unsigned int wWidth = 500;
    unsigned int wHeight = 500;
    const char wTitle[] = "[Generic Title]";
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

    float vertices[] = {
        // Pozicija           // Boja (RGBA)
        // Front face
        -0.5f, -0.25f,  0.5f,   1, 0, 0, 1,
         0.5f, -0.25f,  0.5f,   1, 0, 0, 1,
         0.5f,  0.25f,  0.5f,   1, 0, 0, 1,
         0.5f,  0.25f,  0.5f,   1, 0, 0, 1,
        -0.5f,  0.25f,  0.5f,   1, 0, 0, 1,
        -0.5f, -0.25f,  0.5f,   1, 0, 0, 1,

        // Back face
        -0.5f, -0.25f, -0.5f,   0, 1, 0, 1,
         0.5f, -0.25f, -0.5f,   0, 1, 0, 1,
         0.5f,  0.25f, -0.5f,   0, 1, 0, 1,
         0.5f,  0.25f, -0.5f,   0, 1, 0, 1,
        -0.5f,  0.25f, -0.5f,   0, 1, 0, 1,
        -0.5f, -0.25f, -0.5f,   0, 1, 0, 1,

        // Left face
        -0.5f,  0.25f,  0.5f,   0, 0, 1, 1,
        -0.5f,  0.25f, -0.5f,   0, 0, 1, 1,
        -0.5f, -0.25f, -0.5f,   0, 0, 1, 1,
        -0.5f, -0.25f, -0.5f,   0, 0, 1, 1,
        -0.5f, -0.25f,  0.5f,   0, 0, 1, 1,
        -0.5f,  0.25f,  0.5f,   0, 0, 1, 1,

        // Right face
         0.5f,  0.25f,  0.5f,   1, 1, 0, 1,
         0.5f,  0.25f, -0.5f,   1, 1, 0, 1,
         0.5f, -0.25f, -0.5f,   1, 1, 0, 1,
         0.5f, -0.25f, -0.5f,   1, 1, 0, 1,
         0.5f, -0.25f,  0.5f,   1, 1, 0, 1,
         0.5f,  0.25f,  0.5f,   1, 1, 0, 1,

         // Bottom face (bele boje)
         -0.5f, -0.25f, -0.5f,   1, 1, 1, 1,
          0.5f, -0.25f, -0.5f,   1, 1, 1, 1,
          0.5f, -0.25f,  0.5f,   1, 1, 1, 1,
          0.5f, -0.25f,  0.5f,   1, 1, 1, 1,
         -0.5f, -0.25f,  0.5f,   1, 1, 1, 1,
         -0.5f, -0.25f, -0.5f,   1, 1, 1, 1
    };

    float waterPlane[] = {
        -0.3f,  0.251f, -0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
         0.3f,  0.251f, -0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
         0.3f,  0.251f,  0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
         0.3f,  0.251f,  0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
        -0.3f,  0.251f,  0.3f,   0.4f, 0.8f, 1.0f, 1.0f,
        -0.3f,  0.251f, -0.3f,   0.4f, 0.8f, 1.0f, 1.0f
    };




    float topFrameVertices[] = {
        // Gornja traka
        -0.5f, 0.25f, -0.5f,   1, 0, 1, 1,
         0.5f, 0.25f, -0.5f,   1, 0, 1, 1,
         0.5f, 0.25f, -0.3f,   1, 0, 1, 1,

         0.5f, 0.25f, -0.3f,   1, 0, 1, 1,
        -0.5f, 0.25f, -0.3f,   1, 0, 1, 1,
        -0.5f, 0.25f, -0.5f,   1, 0, 1, 1,

        // Donja traka
        -0.5f, 0.25f, 0.3f,    1, 0, 1, 1,
         0.5f, 0.25f, 0.3f,    1, 0, 1, 1,
         0.5f, 0.25f, 0.5f,    1, 0, 1, 1,

         0.5f, 0.25f, 0.5f,    1, 0, 1, 1,
        -0.5f, 0.25f, 0.5f,    1, 0, 1, 1,
        -0.5f, 0.25f, 0.3f,    1, 0, 1, 1,

        // Leva traka
        -0.5f, 0.25f, -0.3f,   1, 0, 1, 1,
        -0.3f, 0.25f, -0.3f,   1, 0, 1, 1,
        -0.3f, 0.25f, 0.3f,    1, 0, 1, 1,

        -0.3f, 0.25f, 0.3f,    1, 0, 1, 1,
        -0.5f, 0.25f, 0.3f,    1, 0, 1, 1,
        -0.5f, 0.25f, -0.3f,   1, 0, 1, 1,

        // Desna traka
         0.3f,  0.25f, -0.3f,  1, 0, 1, 1,
         0.5f,  0.25f, -0.3f,  1, 0, 1, 1,
         0.5f,  0.25f,  0.3f,  1, 0, 1, 1,

         0.5f,  0.25f,  0.3f,  1, 0, 1, 1,
         0.3f,  0.25f,  0.3f,  1, 0, 1, 1,
         0.3f,  0.25f, -0.3f,  1, 0, 1, 1
    };

    float pillarVertices[] = {
        // Front
        -0.05f, 0.251f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.251f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.251f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,

        // Back
        -0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,

        // Left
        -0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.251f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,

        // Right
         0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.251f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,

         // Top
         -0.05f, 0.551f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
          0.05f, 0.551f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
          0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
          0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         -0.05f, 0.551f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         -0.05f, 0.551f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,

         // Bottom
         -0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
          0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
          0.05f, 0.251f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
          0.05f, 0.251f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         -0.05f, 0.251f,  0.05f,  0.6f, 0.2f, 0.8f, 1.0f,
         -0.05f, 0.251f, -0.05f,  0.6f, 0.2f, 0.8f, 1.0f
    };


    float pyramidVertices[] = {
        // Vrh
        0.0f, 0.65f, 0.0f,     0.6f, 0.2f, 0.8f, 1.0f,

        // Baza
        -0.05f, 0.551f, -0.05f, 0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f, -0.05f, 0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f,  0.05f, 0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f,  0.05f, 0.6f, 0.2f, 0.8f, 1.0f,

        // Strane (4 trougla)
        0.0f, 0.65f, 0.0f,     0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f, -0.05f, 0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f, -0.05f, 0.6f, 0.2f, 0.8f, 1.0f,

        0.0f, 0.65f, 0.0f,     0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f, -0.05f, 0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f,  0.05f, 0.6f, 0.2f, 0.8f, 1.0f,

        0.0f, 0.65f, 0.0f,     0.6f, 0.2f, 0.8f, 1.0f,
         0.05f, 0.551f,  0.05f, 0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f,  0.05f, 0.6f, 0.2f, 0.8f, 1.0f,

        0.0f, 0.65f, 0.0f,     0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f,  0.05f, 0.6f, 0.2f, 0.8f, 1.0f,
        -0.05f, 0.551f, -0.05f, 0.6f, 0.2f, 0.8f, 1.0f
    };




    unsigned int stride = (3 + 4) * sizeof(float);  //velicina jednog verteksa, 3 pozicije + 4 boje
    
    unsigned int VAO;   //vertex array object
    glGenVertexArrays(1, &VAO); //generise jedan vao - crtamo jedan objekat
    glBindVertexArray(VAO); //aktivira vao

    unsigned int VBO;   //vertex buffer object, cuva vrednosti verteksa (pozicije, boje...)
    glGenBuffers(1, &VBO);  //dovoljan je 1 jer imamo 1 niz verteksa
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //aktivira vbo
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  //salje podatke u memoriju gpu-a

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

    unsigned int frameVAO, frameVBO;
    glGenVertexArrays(1, &frameVAO);
    glGenBuffers(1, &frameVBO);

    glBindVertexArray(frameVAO);
    glBindBuffer(GL_ARRAY_BUFFER, frameVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(topFrameVertices), topFrameVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int pillarVAO, pillarVBO;
    glGenVertexArrays(1, &pillarVAO);
    glGenBuffers(1, &pillarVBO);

    glBindVertexArray(pillarVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pillarVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pillarVertices), pillarVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int pyramidVAO, pyramidVBO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);

    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    // pozicija (atribut 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);  
    glEnableVertexAttribArray(0);

    // boja (atribut 1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);   //odvezuje vbo
    glBindVertexArray(0);   //odvezuje vao
    

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
    glm::mat4 projectionO = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f); //Matrica ortogonalne projekcije (Lijeva, desna, donja, gornja, prednja i zadnja ravan)
    unsigned int projectionLoc = glGetUniformLocation(unifiedShader, "uP");


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    glUseProgram(unifiedShader); //Slanje default vrijednosti uniformi
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); //view
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionO));    //ortogonalna default
    glBindVertexArray(VAO); //aktivira vao

    glClearColor(0.5, 0.5, 0.5, 1.0);   //boja pozadine
    glCullFace(GL_BACK);//Biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        //Mijenjanje projekcija
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionO));
        }
        //Transformisanje trouglova
        //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        //{
        //    //model = glm::translate(model, glm::vec3(-0.01, 0.0, 0.0)); //Pomjeranje (Matrica transformacije, pomjeraj po XYZ)
        //    model = glm::rotate(model, glm::radians(-0.5f), glm::vec3(0.0f, 1.0f, 0.0f)); //Rotiranje (Matrica transformacije, ugao rotacije u radijanima, osa rotacije)
        //    //model = glm::scale(model, glm::vec3(0.99, 1.0, 1.0)); //Skaliranje (Matrica transformacije, skaliranje po XYZ)
        //    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //}
        //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        //{
        //    //model = glm::translate(model, glm::vec3(0.01, 0.0, 0.0));
        //    model = glm::rotate(model, glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        //    //model = glm::scale(model, glm::vec3(1.1, 1.0, 1.0));
        //    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //}

        // Obrada tastature WSAD
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camAngle -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camAngle += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camHeight += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camHeight -= 0.02f;

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            radius -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            radius += 0.02f;


        // Ažuriranje pozicije kamere
        float camX = sin(camAngle) * radius;
        float camZ = cos(camAngle) * radius;

        view = glm::lookAt(glm::vec3(camX, camHeight, camZ),
            glm::vec3(0.0f, 0.0f, 0.0f),     // gleda u centar
            glm::vec3(0.0f, 1.0f, 0.0f));    // Y je gore

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // prvo voda 
        glBindVertexArray(waterVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // zatim kvadar 
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // gornji okvir
        glBindVertexArray(frameVAO);
        glDrawArrays(GL_TRIANGLES, 0, 24);

        //stub
        glBindVertexArray(pillarVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // piramida na vrhu stuba
        glBindVertexArray(pyramidVAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++


    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);

    glDeleteVertexArrays(1, &frameVAO);
    glDeleteBuffers(1, &frameVBO);

    glDeleteVertexArrays(1, &pillarVAO);
    glDeleteBuffers(1, &pillarVBO);

    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteBuffers(1, &pyramidVBO);


    glDeleteProgram(unifiedShader);

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
