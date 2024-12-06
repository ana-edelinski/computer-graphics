// Autor: Ana Edelinski RA 165/2021
// Zadatak 5: Radio

#define _CRT_SECURE_NO_WARNINGS
#define CRES 100 // Circle Resolution = Rezolucija kruga
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   //Omogucava upotrebu OpenGL naredbi
#include <GLFW/glfw3.h> //Olaksava pravljenje i otvaranje prozora (konteksta) sa OpenGL sadrzajem
#include "stb_image.h"
#include <chrono>
#include <thread>

bool radioOn = false;
float timeElapsed = 0.0f; //Proteklo vreme za animaciju lampice
float volumeBarIndicatorOffset = 0.0f;
float sliderPosition = 0.0f; 
bool sliderDragging = false;    //Da li korisnik trenutno prevlaci slider
float speakerMembraneScaleLeft = 1.0f;
float speakerMembraneScaleRight = 1.0f;
float progressBarWidth = 0.4f;
float progressBarHeight = 0.02f; 
float progressBarY = -0.25f; 
float progressBarFill = 0.0f;
float lastProgressBarFill = 0.0f; 
float displayY = -0.37f; 
float displayWidth = 0.4f; 
float displayHeight = 0.15f; 
bool FMon = true;   //Da li je FM mode on
float antennaOffset = -0.55f; //Antena na pocetku uvucena
float scaleIndicatorOffset = 0.0f;
float textureOffset = 0.0f; 
float scrollSpeed = 0.0006f; 
const int TARGET_FPS = 60;
const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;

unsigned int compileShader(GLenum type, const char* source);     //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
unsigned int createShader(const char* vsSource, const char* fsSource);   //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource i Fragment sejdera na putanji fsSource
static unsigned loadImageToTexture(const char* filePath);
void generateCircle(float* circleVertices, float centerX, float centerY, float radius);
void updateProgressBar(float sliderPosition, unsigned int VBO);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos);

struct RadioStation {
    float startFrequency;  
    float endFrequency;   
    unsigned int texture;  
    bool isFM;            
};


int main(void)
{
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ INICIJALIZACIJA ++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Pokretanje GLFW biblioteke
    // Nju koristimo za stvaranje okvira prozora
    if (!glfwInit())    // !0 == 1  | glfwInit inicijalizuje GLFW i vrati 1 ako je inicijalizovana uspesno, a 0 ako nije
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    //Odredjivanje OpenGL verzije i profila (3.3, programabilni pajplajn)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // kreiranje prozora 500x500
    GLFWwindow* window; //mesto u memoriji za prozor
    unsigned int wWidth = 1024;
    unsigned int wHeight = 1024;
    const char wTitle[] = "Radio";    //naslov prozora
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window); //ako je prozor uspesno kreiran postavlja ga kao aktivan
    glfwSetKeyCallback(window, keyCallback);

    // Inicijalizacija GLEW biblioteke
    if (glewInit() != GLEW_OK)   //Slicno kao glfwInit. GLEW_OK je predefinisani kod za uspjesnu inicijalizaciju sadrzan unutar biblioteke
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::vector<RadioStation> stations = {
        {-0.04f, 0.1f, loadImageToTexture("resources/radio1.png"), true}, // FM stanica
        {0.1f, 0.3f, loadImageToTexture("resources/radio2.png"), true}, // FM stanica
        {0.3f, 0.6f, loadImageToTexture("resources/radio3.png"), false}, // AM stanica
        {0.6f, 0.9f, loadImageToTexture("resources/radio4.png"), true}, // FM stanica
        {0.9f, 1.15f, loadImageToTexture("resources/radio5.png"), true} // FM stanica
    };


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int sharedShader = createShader("shared.vert", "shared.frag");    
    unsigned int membraneShader = createShader("membrane.vert", "membrane.frag");
    unsigned int textureShader = createShader("texture.vert", "texture.frag");
    unsigned int backgroundShader = createShader("background.vert", "background.frag");
    unsigned int scrollTextureShader = createShader("scroll.vert", "scroll.frag");
    unsigned int basicShader = createShader("basic.vert", "basic.frag");    


    unsigned int VAO[37];  
    unsigned int VBO[37];
    glGenVertexArrays(37, VAO);
    glGenBuffers(37, VBO);
    unsigned int stride;

    unsigned int activeStationTexture = 0; 

    float radioBodyVertices[] = {
        -0.9, -0.7, 
         0.9, -0.7,  
        -0.9,  0.2, 
         0.9,  0.2   
    };

    float rightSpeakerVertices[] = {
        0.25, -0.65,  
        0.75, -0.65,  
        0.25, -0.15,  
        0.75, -0.15   
    };

    float leftSpeakerVertices[] = {
        -0.75, -0.65,  
        -0.25, -0.65,  
        -0.75, -0.15,  
        -0.25, -0.15   
    };

    float rightSpeakerMeshVertices[] = {
        0.25, -0.65,  0.0, 0.0,
        0.75, -0.65,  1.0, 0.0,
        0.25, -0.15,  0.0, 1.0,
        0.75, -0.15,  1.0, 1.0
    };

    float leftSpeakerMeshVertices[] = {
        -0.75, -0.65,  0.0, 0.0,
        -0.25, -0.65,  1.0, 0.0,
        -0.75, -0.15,  0.0, 1.0,
        -0.25, -0.15,  1.0, 1.0F
    };

    float separatorLineVertices[] = {
        -0.9f, -0.1f, 
         0.9f, -0.1f  
    };

    float leftMembraneVertices[(CRES + 2) * 2];
    generateCircle(leftMembraneVertices, -0.5, -0.4, 0.25f); 

    float rightMembraneVertices[(CRES + 2) * 2];
    generateCircle(rightMembraneVertices, 0.5, -0.4, 0.25f); 

    float smallLeftMembraneVertices[(CRES + 2) * 2];
    generateCircle(smallLeftMembraneVertices, -0.5, -0.4, 0.10f); 

    float smallRightMembraneVertices[(CRES + 2) * 2];
    generateCircle(smallRightMembraneVertices, 0.5, -0.4, 0.10f);

    float verticesRadioButtonIndicatorLine[] =
    {
        0.8, 0.025,
        0.8, 0.065
    };

    float sliderBarVertices[] = {
        -0.2f, -0.2f,
         0.2f, -0.2f
    };

    float progressBarVertices[] = {
        -progressBarWidth / 2, progressBarY - progressBarHeight / 2, 
        progressBarWidth / 2, progressBarY - progressBarHeight / 2,  
        -progressBarWidth / 2, progressBarY + progressBarHeight / 2,  
        progressBarWidth / 2, progressBarY + progressBarHeight / 2  
    };

    float progressBarFillVertices[] = {
        -progressBarWidth / 2, progressBarY - progressBarHeight / 2,
        -progressBarWidth / 2, progressBarY + progressBarHeight / 2,
        -progressBarWidth / 2, progressBarY - progressBarHeight / 2,
        -progressBarWidth / 2, progressBarY + progressBarHeight / 2
    };

    float displayVertices[] = {
        -displayWidth / 2, displayY - displayHeight / 2,  0.0f, 0.0f, 
         displayWidth / 2, displayY - displayHeight / 2,  1.0f, 0.0f, 
        -displayWidth / 2, displayY + displayHeight / 2,  0.0f, 1.0f, 
         displayWidth / 2, displayY + displayHeight / 2,  1.0f, 1.0f 
    };


    float AMFMRailVertices[] = {
        -0.1f, -0.58f, 
         0.1f, -0.58f,
        -0.1f, -0.48f,
         0.1f, -0.48f
    };

    float AMFMSwitchVertices[] = {
        -0.05f, -0.58f,
         0.05f, -0.58f,
        -0.05f, -0.48f,
         0.05f, -0.48f
    };

    float AMVertices[] = {
        -displayWidth / 2, -0.58f, 0.0, 0.0,  
        -displayWidth / 4, -0.58f, 1.0, 0.0,
        -displayWidth / 2, -0.48f, 0.0, 1.0,
        -displayWidth / 4, -0.48f, 1.0, 1.0
    };

    float FMVertices[] = {
        displayWidth / 4, -0.58f, 0.0, 0.0, 
        displayWidth / 2, -0.58f, 1.0, 0.0,
        displayWidth / 4, -0.48f, 0.0, 1.0,
        displayWidth / 2, -0.48f, 1.0, 1.0
    };


    float AMFMRailOutlineVertices[] = {
        -displayWidth / 4, -0.58f,
         displayWidth / 4, -0.58f, 
         displayWidth / 4, -0.48f, 
        -displayWidth / 4, -0.48f, 
        -displayWidth / 4, -0.58f 
    };

    float AMFMSwitchOutlineVertices[] = {
        -0.05f, -0.58f,
         0.05f, -0.58f, 
         0.05f, -0.48f, 
        -0.05f, -0.48f, 
        -0.05f, -0.58f  
    };


    float antennaBaseVertices[] = {
        -0.85f, 0.2f,  
        -0.80f, 0.2f,
        -0.85f, 0.35f, 
        -0.80f, 0.35f  
    };

    float antennaVertices[] = {
        -0.8375f, 0.25f, 
        -0.8125f, 0.25f,
        -0.8375f, 0.9f, 
        -0.8125f, 0.9f   
    };

    float antennaTipVertices[] = {
        -0.85f, 0.9f,  
        -0.80f, 0.9f, 
        -0.85f, 0.95f, 
        -0.80f, 0.95f  
    };

    float signatureVertices[] =
    {  
        0.0, 0.85, 0.0, 0.0,
        1.0, 0.85, 1.0, 0.0,
        0.0,  1.0, 0.0, 1.0,
        1.0,  1.0, 1.0, 1.0
    };

    float scaleVertices[] = {
        -0.85, -0.025, 
         0.65, -0.025,  
        -0.85, 0.075,  
         0.65, 0.075   
    };

    float scaleValuesVertices[134] =
    {
        -0.85, 0.125,
        0.65, 0.125
    };

    float handleHolderLeft[] = {
        -0.70f, 0.2f, 
        -0.65f, 0.2f,  
        -0.70f, 0.4f, 
        -0.65f, 0.4f   
    };

    float handleHolderRight[] = {
        0.65f, 0.2f,   
        0.70f, 0.2f,  
        0.65f, 0.4f,  
        0.70f, 0.4f    
    };

    float handleBar[] = {
        -0.65f, 0.35f,  
        0.65f, 0.35f,  
        -0.65f, 0.45f,  
        0.65f, 0.45f   
    };

    float backgroundVertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,  
         1.0f, -1.0f, 1.0f, 0.0f,  
        -1.0f,  1.0f, 0.0f, 1.0f,  
         1.0f,  1.0f, 1.0f, 1.0f   
    };



    //Povezivanje podataka sa VAO i VBO

    // Telo radija
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(radioBodyVertices), radioBodyVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Levi zvucnik
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftSpeakerVertices), leftSpeakerVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Desni zvucnik
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightSpeakerVertices), rightSpeakerVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // *** Ucitavanje teksture ***
    unsigned int meshTexture = loadImageToTexture("resources/mesh.png");
    if (meshTexture == 0) {
        std::cout << "Greska pri ucitavanju teksture!" << std::endl;
        return 4;
    }

    // Leva membrana
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftMembraneVertices), leftMembraneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);   

    // Desna membrana
    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightMembraneVertices), rightMembraneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Leva membrana mala
    glBindVertexArray(VAO[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(smallLeftMembraneVertices), smallLeftMembraneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Desna membrana mala
    glBindVertexArray(VAO[6]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(smallRightMembraneVertices), smallRightMembraneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Leva mrezica
    glBindVertexArray(VAO[7]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftSpeakerMeshVertices), leftSpeakerMeshVertices, GL_STATIC_DRAW);
    // Pozicija (layout(location = 0))
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);   
    // Tekstura (layout(location = 1))
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);  
    glBindVertexArray(0); 

    // Desna mrezica
    glBindVertexArray(VAO[8]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightSpeakerMeshVertices), rightSpeakerMeshVertices, GL_STATIC_DRAW);    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Horizontalna linija
    glBindVertexArray(VAO[9]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(separatorLineVertices), separatorLineVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Radio ON/OFF Button
    float radioOnOffButton[CRES * 2 + 4];
    float r = 0.05;
    radioOnOffButton[0] = 0.8;
    radioOnOffButton[1] = 0.025;
    for (int i = 0; i <= CRES; i++)
    {

        radioOnOffButton[2 + 2 * i] = radioOnOffButton[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
        radioOnOffButton[2 + 2 * i + 1] = radioOnOffButton[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
    }
    glBindVertexArray(VAO[10]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffButton), radioOnOffButton, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Radio ON/OFF Button Indicator
    float radioOnOffButtonIndicator[CRES * 2 + 4];
    r = 0.04;
    radioOnOffButtonIndicator[0] = 0.8;
    radioOnOffButtonIndicator[1] = 0.025;
    for (int i = 0; i <= CRES; i++)
    {
        radioOnOffButtonIndicator[2 + 2 * i] = radioOnOffButtonIndicator[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
        radioOnOffButtonIndicator[2 + 2 * i + 1] = radioOnOffButtonIndicator[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
    }
    glBindVertexArray(VAO[11]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[11]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffButtonIndicator), radioOnOffButtonIndicator, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Radio ON/OFF Button Indicator Inner
    float radioOnOffButtonIndicatorInner[CRES * 2 + 4];
    r = 0.035;
    radioOnOffButtonIndicatorInner[0] = 0.8;
    radioOnOffButtonIndicatorInner[1] = 0.025;
    for (int i = 0; i <= CRES; i++)
    {
        radioOnOffButtonIndicatorInner[2 + 2 * i] = radioOnOffButtonIndicatorInner[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
        radioOnOffButtonIndicatorInner[2 + 2 * i + 1] = radioOnOffButtonIndicatorInner[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
    }
    glBindVertexArray(VAO[12]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[12]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffButtonIndicatorInner), radioOnOffButtonIndicatorInner, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Radio ON/OFF Button Line
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[13]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[13]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRadioButtonIndicatorLine), verticesRadioButtonIndicatorLine, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Radio ON/OFF Lamp
    float radioOnOffLamp[CRES * 2 + 4];
    r = 0.02;
    radioOnOffLamp[0] = 0.725;
    radioOnOffLamp[1] = 0.025;
    for (int i = 0; i <= CRES; i++)
    {
        radioOnOffLamp[2 + 2 * i] = radioOnOffLamp[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
        radioOnOffLamp[2 + 2 * i + 1] = radioOnOffLamp[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
    }
    glBindVertexArray(VAO[14]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[14]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffLamp), radioOnOffLamp, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Radio ON/OFF Lamp Light
    float radioOnOffLampLight[CRES * 2 + 4];
    r = 0.01;
    radioOnOffLampLight[0] = 0.725;
    radioOnOffLampLight[1] = 0.025;
    for (int i = 0; i <= CRES; i++)
    {
        radioOnOffLampLight[2 + 2 * i] = radioOnOffLampLight[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
        radioOnOffLampLight[2 + 2 * i + 1] = radioOnOffLampLight[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
    }
    glBindVertexArray(VAO[15]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[15]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffLampLight), radioOnOffLampLight, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Horizontalna traka za slider
    glBindVertexArray(VAO[16]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[16]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sliderBarVertices), sliderBarVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Krug slidera
    float sliderVertices[(CRES + 2) * 2];
    generateCircle(sliderVertices, sliderPosition, -0.2f, 0.03f);
    glBindVertexArray(VAO[17]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[17]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sliderVertices), sliderVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Progress bar
    glBindVertexArray(VAO[18]); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO[18]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarVertices), progressBarVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Progress bar indikator
    glBindVertexArray(VAO[19]); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO[19]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarFillVertices), progressBarFillVertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Displej
    glBindVertexArray(VAO[20]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[20]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(displayVertices), displayVertices, GL_STATIC_DRAW);
    // Atribut za poziciju (layout(location = 0))
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Atribut za teksturne koordinate (layout(location = 1))
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // AM/FM Rail
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[21]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[21]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(AMFMRailVertices), AMFMRailVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // AM/FM Switch
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[22]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[22]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(AMFMSwitchVertices), AMFMSwitchVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // AM
    stride = (2 + 2) * sizeof(float);
    glBindVertexArray(VAO[23]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[23]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(AMVertices), AMVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    unsigned AMTexture = loadImageToTexture("resources/am.png");
    glBindTexture(GL_TEXTURE_2D, AMTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc1 = glGetUniformLocation(textureShader, "uTex");
    glUniform1i(uTexLoc1, 0);

    // FM
    stride = (2 + 2) * sizeof(float);
    glBindVertexArray(VAO[24]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[24]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FMVertices), FMVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    unsigned FMTexture = loadImageToTexture("resources/fm.png");
    glBindTexture(GL_TEXTURE_2D, FMTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc2 = glGetUniformLocation(textureShader, "uTex");
    glUniform1i(uTexLoc2, 0);

    // AM/FM Rail Outline
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[25]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[25]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(AMFMRailOutlineVertices), AMFMRailOutlineVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // AM/FM Switch Outline
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[26]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[26]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(AMFMSwitchOutlineVertices), AMFMSwitchOutlineVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Antena osnova
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[27]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[27]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(antennaBaseVertices), antennaBaseVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Antena
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[28]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[28]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(antennaVertices), antennaVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Antena vrh
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[29]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[29]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(antennaTipVertices), antennaTipVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Potpis
    stride = (2 + 2) * sizeof(float);
    glBindVertexArray(VAO[30]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[30]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signatureVertices), signatureVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    unsigned signatureTexture = loadImageToTexture("resources/potpis.png");
    glBindTexture(GL_TEXTURE_2D, signatureTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc4 = glGetUniformLocation(textureShader, "uTex");
    glUniform1i(uTexLoc4, 0);

    // Skala
    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[31]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[31]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(scaleVertices), scaleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Vrednosti skale
    float position = 0.8;
    for (int i = 1; i <= 29; i++) {
        scaleValuesVertices[4 * i] = -0.8 + (i - 1) * 0.05;
        scaleValuesVertices[4 * i + 1] = 0.1375;
        scaleValuesVertices[4 * i + 2] = -0.8 + (i - 1) * 0.05;
        scaleValuesVertices[4 * i + 3] = 0.1125;
    }
    scaleValuesVertices[120] = -0.80; // X koordinata donje tačke kazaljke (središnja linija)
    scaleValuesVertices[121] = 0.075; // Y koordinata donje tačke kazaljke (središnja linija)
    scaleValuesVertices[122] = -0.80; // X koordinata gornje tačke kazaljke (središnja linija)
    scaleValuesVertices[123] = 0.1625; // Y koordinata gornje tačke kazaljke (središnja linija)
    scaleValuesVertices[124] = -0.85; // X koordinata leve donje tačke pravougaonika (telo kazaljke)
    scaleValuesVertices[125] = 0.075; // Y koordinata leve donje tačke pravougaonika
    scaleValuesVertices[126] = 0.65; // X koordinata desne donje tačke pravougaonika
    scaleValuesVertices[127] = 0.075; // Y koordinata desne donje tačke pravougaonika
    scaleValuesVertices[128] = 0.65; // X koordinata desne gornje tačke pravougaonika
    scaleValuesVertices[129] = 0.175; // Y koordinata desne gornje tačke pravougaonika
    scaleValuesVertices[130] = -0.85; // X koordinata leve gornje tačke pravougaonika
    scaleValuesVertices[131] = 0.175; // Y koordinata leve gornje tačke pravougaonika
    scaleValuesVertices[132] = -0.85; // Ponovna X koordinata leve donje tačke za zatvaranje pravougaonika
    scaleValuesVertices[133] = 0.075; // Ponovna Y koordinata leve donje tačke za zatvaranje pravougaonika


    // Pomeranje po Y osi (jer sam naknadno menjala neke velicine pa da ne bih svuda oduzimala 0.1)
    float adjustmentY = -0.1f; 
    for (int i = 1; i < sizeof(scaleValuesVertices) / sizeof(float); i += 2) {
        scaleValuesVertices[i] += adjustmentY;
    }

    stride = 2 * sizeof(float);
    glBindVertexArray(VAO[32]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[32]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(scaleValuesVertices), scaleValuesVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Leva strana nosaca rucke
    glBindVertexArray(VAO[33]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[33]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(handleHolderLeft), handleHolderLeft, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Desna strana nosaca rucke
    glBindVertexArray(VAO[34]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[34]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(handleHolderRight), handleHolderRight, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Rucka
    glBindVertexArray(VAO[35]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[35]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(handleBar), handleBar, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Background
    unsigned int backgroundTexture = loadImageToTexture("resources/background.png");
    if (backgroundTexture == 0) {
        std::cout << "Greska pri ucitavanju teksture pozadine!" << std::endl;
        return 4;
    }
    glBindVertexArray(VAO[36]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[36]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backgroundVertices), backgroundVertices, GL_STATIC_DRAW);
    // Povezivanje atributa za poziciju
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Povezivanje atributa za teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);


    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    double previousTime = glfwGetTime();

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    // Glavna petlja
    while (!glfwWindowShouldClose(window))  //Beskonacna petlja iz koje izlazimo tek kada prozor treba da se zatvori
    {
        glLineWidth(1.0f);

        glfwPollEvents();  //omogućava aplikaciji da reaguje na interakcije korisnika i druge sistemske dogadjaje

        float membraneAmplitude = 0.0f; 

        if (radioOn && activeStationTexture != 0 && sliderPosition > -0.2f) {
            volumeBarIndicatorOffset = (sliderPosition + 0.2f) / 0.4f; // Normalizacija slidera u opsegu [0, 1]
            membraneAmplitude = volumeBarIndicatorOffset * 0.05f; // Maksimalna amplituda je 0.05
            double time = glfwGetTime();
            
            float oscillation = 1.0f + membraneAmplitude * sin(time * 20.0f);
            speakerMembraneScaleLeft = oscillation;
            speakerMembraneScaleRight = oscillation;
        }
        else {
            speakerMembraneScaleLeft = 1.0f;
            speakerMembraneScaleRight = 1.0f;
        }

        // Animacija vibracija membrane (sinusoidni pokret)
        double time = glfwGetTime();
        float oscillation = 1.0f + membraneAmplitude * sin(time * 20.0f);
        speakerMembraneScaleLeft = oscillation;
        speakerMembraneScaleRight = oscillation;


        // [KOD ZA CRTANJE]

        // Pozadina
        glUseProgram(backgroundShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glUniform1i(glGetUniformLocation(backgroundShader, "backgroundTexture"), 0);
        glBindVertexArray(VAO[36]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        // Antena
        glUseProgram(basicShader);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.8f, 0.8f, 0.8f);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), 0.0f, antennaOffset); 
        glBindVertexArray(VAO[28]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Antena vrh
        glUseProgram(basicShader);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.8f, 0.8f, 0.8f);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), 0.0f, antennaOffset); 
        glBindVertexArray(VAO[29]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Antena osnova
        glUseProgram(basicShader);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.5f, 0.5f, 0.5f);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), 0.0f, 0.0f);
        glBindVertexArray(VAO[27]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


        // Telo radija
        glUseProgram(sharedShader);
        unsigned int uBodyColorLoc = glGetUniformLocation(sharedShader, "color");
        glUniform3f(uBodyColorLoc, 70 / 255.0f, 70 / 255.0f, 70 / 255.0f);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // ZVUCNICI
        glUseProgram(sharedShader);
        unsigned int uSpeakerColorLoc = glGetUniformLocation(sharedShader, "color");

        // Levi zvucnik
        glUniform3f(uSpeakerColorLoc, 245 / 255.0f, 245 / 255.0f, 220 / 255.0f);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Desni zvucik
        glUniform3f(uSpeakerColorLoc, 245 / 255.0f, 245 / 255.0f, 220 / 255.0f);
        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // MEMBRANE
        glUseProgram(membraneShader);
        unsigned int uMembraneColorLoc = glGetUniformLocation(membraneShader, "color"); 

        // Leva membrana - velika
        glUseProgram(membraneShader);
        glUniform2f(glGetUniformLocation(membraneShader, "center"), -0.5, -0.4);
        glUniform1f(glGetUniformLocation(membraneShader, "scale"), speakerMembraneScaleLeft);
        glUniform3f(glGetUniformLocation(membraneShader, "color"), 0.3f, 0.3f, 0.3f); 
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        // Desna membrana - velika
        glUniform2f(glGetUniformLocation(membraneShader, "center"), 0.5, -0.4);
        glUniform1f(glGetUniformLocation(membraneShader, "scale"), speakerMembraneScaleRight);
        glBindVertexArray(VAO[4]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        // Leva membrana - mala
        glUniform2f(glGetUniformLocation(membraneShader, "center"), -0.5, -0.4);
        glUniform1f(glGetUniformLocation(membraneShader, "scale"), speakerMembraneScaleLeft);
        glUniform3f(glGetUniformLocation(membraneShader, "color"), 0.2f, 0.2f, 0.2f);
        glBindVertexArray(VAO[5]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        // Desna membrana - mala
        glUniform2f(glGetUniformLocation(membraneShader, "center"), 0.5, -0.4);
        glUniform1f(glGetUniformLocation(membraneShader, "scale"), speakerMembraneScaleRight);
        glBindVertexArray(VAO[6]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        // Horizontalna linija
        glUseProgram(sharedShader);
        unsigned int uLineColorLoc = glGetUniformLocation(sharedShader, "color");
        glUniform3f(uLineColorLoc, 0.0f, 0.0f, 0.0f);
        glBindVertexArray(VAO[9]);
        glDrawArrays(GL_LINES, 0, 2);


        // MREZICE ZVUCNIKA
        glUseProgram(textureShader);
        unsigned int textureUniform = glGetUniformLocation(textureShader, "uTex");
        glUniform1i(textureUniform, 0); 

        // Mrezica levog zvucnika
        glBindVertexArray(VAO[7]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, meshTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        // Mrezica desnog zvucnika
        glBindVertexArray(VAO[8]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glUseProgram(0);


        // Izracunavanje proteklog vremena za animaciju
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        if (!radioOn) {
            timeElapsed = 0.0f; 
        }

        // Izracunavanje interpolirane boje za lampicu kada je radio ukljucen
        float lampRed = 0.0f, lampGreen = 0.0f, lampBlue = 0.0f; 
        if (radioOn) {
            timeElapsed += deltaTime; // Azuriranje vremena za animaciju
            float intensity = (sin(timeElapsed * 2.0f) + 1.0f) / 2.0f;
            lampRed = 1.0f; 
            lampGreen = 1.0f - 0.5f * intensity; 
            lampBlue = 1.0f - intensity; 
        }

        // ON/OFF BUTTON SA SVIM NJEGOVIM DELOVIMA + LAMPICA
        glUseProgram(basicShader);

        // ON/OFF Button
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.2f, 0.2f, 0.2f);
        glBindVertexArray(VAO[10]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffButton) / (2 * sizeof(float)));

        // Postavljanje boje indikatora u zavisnosti od toga da li je radio ukljucen (krug)
        float indicatorR = radioOn ? 1.0f : 0.0f; 
        float indicatorG = radioOn ? 1.0f : 0.0f; 
        float indicatorB = radioOn ? 1.0f : 0.0f; 

        glUniform3f(glGetUniformLocation(basicShader, "color"), indicatorR, indicatorG, indicatorB);

        // Krug ON/OFF button indikatora
        glBindVertexArray(VAO[11]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffButtonIndicator) / (2 * sizeof(float)));

        // Unutrasnji deo ON/OFF button indikatora
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.2f, 0.2f, 0.2f);
        glBindVertexArray(VAO[12]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffButtonIndicatorInner) / (2 * sizeof(float)));


        // Postavljanje boje linije u zavisnosti od toga da li je radio ukljcuen (linija u sredini)
        float lineR = radioOn ? 1.0f : 0.0f; 
        float lineG = radioOn ? 1.0f : 0.0f; 
        float lineB = radioOn ? 1.0f : 0.0f; 

        glUniform3f(glGetUniformLocation(basicShader, "color"), lineR, lineG, lineB);

        // Srednja linija na ON/OFF button indikatoru
        glBindVertexArray(VAO[13]);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 2);

        // Lampica osnovna (sivo iza crne tackice)
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.2f, 0.2f, 0.2f);
        glBindVertexArray(VAO[14]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffLamp) / (2 * sizeof(float)));

        // Boja lampice
        glUniform3f(glGetUniformLocation(basicShader, "color"), lampRed, lampGreen, lampBlue);
        glBindVertexArray(VAO[15]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffLamp) / (2 * sizeof(float)));


        // Slider i traka
        volumeBarIndicatorOffset = (sliderPosition + 0.5f); // Skaliranje u opsegu [0, 1]
        glUseProgram(sharedShader);
        glUniform3f(glGetUniformLocation(sharedShader, "color"), 0.7f, 0.7f, 0.7f);
        glBindVertexArray(VAO[16]);
        glDrawArrays(GL_LINES, 0, 2);

        // Krug slidera
        float sliderVertices[(CRES + 2) * 2];
        generateCircle(sliderVertices, sliderPosition, -0.2f, 0.03f);
        glBindVertexArray(VAO[17]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[17]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sliderVertices), sliderVertices, GL_DYNAMIC_DRAW);

        // Progress bar
        glUseProgram(basicShader);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.4f, 0.4f, 0.4f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);
        updateProgressBar(sliderPosition, VBO[19]);

        // Progress bar indikator
        glUseProgram(sharedShader);
        glUniform3f(glGetUniformLocation(sharedShader, "color"), 0.7f, 0.7f, 0.7f);
        glBindVertexArray(VAO[18]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            
        //postavljanje boje za progress bar
        if (!radioOn) {
            glUniform3f(glGetUniformLocation(sharedShader, "color"), 0.7f, 0.7f, 0.7f);
        }
        else {
            glUniform3f(glGetUniformLocation(sharedShader, "color"), 0.0f, 1.0f, 0.0f);
        }
        glBindVertexArray(VAO[19]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Displej
        glUseProgram(sharedShader);
        glUniform3f(glGetUniformLocation(sharedShader, "color"), 0.0f, 0.0f, 0.0f);
        glBindVertexArray(VAO[20]); 
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Detekcija aktivne stanice
        activeStationTexture = 0; 
        if (radioOn && antennaOffset > -0.5f) { 
            for (const auto& station : stations) {
                if (scaleIndicatorOffset >= station.startFrequency && scaleIndicatorOffset <= station.endFrequency) {
                    if ((FMon && station.isFM) || (!FMon && !station.isFM)) { 
                        activeStationTexture = station.texture; 
                        break;
                    }
                }
            }
        }


        if (radioOn && activeStationTexture != 0 && sliderPosition > -0.2f && antennaOffset > -0.5f) {

            textureOffset += scrollSpeed;
            if (textureOffset <= -1.0f) {
                textureOffset += 1.0f; 
            }

            glUseProgram(scrollTextureShader);
            glUniform1f(glGetUniformLocation(scrollTextureShader, "uOffset"), textureOffset);
            unsigned int textureUniform = glGetUniformLocation(scrollTextureShader, "uTex");
            glUniform1i(textureUniform, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, activeStationTexture);
            glBindVertexArray(VAO[20]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            volumeBarIndicatorOffset = (sliderPosition + 0.2f) / 0.4f; // Normalizacija slidera u opsegu [0, 1]
            membraneAmplitude = volumeBarIndicatorOffset * 0.05f; // Maksimalna amplituda je 0.05
            double time = glfwGetTime();

            float oscillation = 1.0f + membraneAmplitude * sin(time * 20.0f);
            speakerMembraneScaleLeft = oscillation;
            speakerMembraneScaleRight = oscillation;
        }
        else {
            speakerMembraneScaleLeft = 1.0f;
            speakerMembraneScaleRight = 1.0f;
        }

        // AM/FM RAIL
        glUseProgram(basicShader);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.45f, 0.45f, 0.45f);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), 0.0f, 0.0f);
        glBindVertexArray(VAO[21]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // AM/FM SWITCH
        glUseProgram(basicShader);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.8f, 0.8f, 0.8f);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), FMon ? 0.05f : -0.05f, 0.0f);
        glBindVertexArray(VAO[22]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // AM
        glUseProgram(textureShader);
        glBindVertexArray(VAO[23]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, AMTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);

        // FM
        glUseProgram(textureShader);
        glBindVertexArray(VAO[24]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FMTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);

        // AM/FM Rail Outlines
        glUseProgram(basicShader);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.0f, 0.0f, 0.0f);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), 0.0f, 0.0f);
        glBindVertexArray(VAO[25]);
        glDrawArrays(GL_LINES, 0, 2);
        glDrawArrays(GL_LINES, 1, 2);
        glDrawArrays(GL_LINES, 2, 2);
        glDrawArrays(GL_LINES, 3, 2);

        // AM/FM Switch Outlines
        glUseProgram(basicShader);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), FMon ? 0.05f : -0.05f, 0.0f);
        glBindVertexArray(VAO[26]);
        glDrawArrays(GL_LINES, 0, 2);
        glDrawArrays(GL_LINES, 1, 2);
        glDrawArrays(GL_LINES, 2, 2);
        glDrawArrays(GL_LINES, 3, 2);

        // Potpis
        glUseProgram(textureShader);
        glBindVertexArray(VAO[30]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signatureTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Skala
        glUseProgram(basicShader);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 1.0f, 1.0f, 1.0f);
        glBindVertexArray(VAO[31]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Vrednosti skale
        glUseProgram(basicShader);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.0f, 0.0f, 0.0f);
        glBindVertexArray(VAO[32]);
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, 2);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 2, 2);
        glDrawArrays(GL_LINES, 4, 2);
        glDrawArrays(GL_LINES, 6, 2);
        glDrawArrays(GL_LINES, 8, 2);
        glDrawArrays(GL_LINES, 10, 2);
        glDrawArrays(GL_LINES, 12, 2);
        glDrawArrays(GL_LINES, 14, 2);
        glDrawArrays(GL_LINES, 16, 2);
        glDrawArrays(GL_LINES, 18, 2);
        glDrawArrays(GL_LINES, 20, 2);
        glDrawArrays(GL_LINES, 22, 2);
        glDrawArrays(GL_LINES, 24, 2);
        glDrawArrays(GL_LINES, 26, 2);
        glDrawArrays(GL_LINES, 28, 2);
        glDrawArrays(GL_LINES, 30, 2);
        glDrawArrays(GL_LINES, 32, 2);
        glDrawArrays(GL_LINES, 34, 2);
        glDrawArrays(GL_LINES, 36, 2);
        glDrawArrays(GL_LINES, 38, 2);
        glDrawArrays(GL_LINES, 40, 2);
        glDrawArrays(GL_LINES, 42, 2);
        glDrawArrays(GL_LINES, 44, 2);
        glDrawArrays(GL_LINES, 46, 2);
        glDrawArrays(GL_LINES, 48, 2);
        glDrawArrays(GL_LINES, 50, 2);
        glDrawArrays(GL_LINES, 52, 2);
        glDrawArrays(GL_LINES, 54, 2);
        glDrawArrays(GL_LINES, 56, 2);
        glDrawArrays(GL_LINES, 58, 2);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 1.0f, 0.5f, 0.0f);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), scaleIndicatorOffset, 0.0f); 
        glLineWidth(3.6f);
        glDrawArrays(GL_LINES, 60, 2);
        glUniform3f(glGetUniformLocation(basicShader, "color"), 0.0f, 0.0f, 0.0f);
        glUniform2f(glGetUniformLocation(basicShader, "offset"), 0.0f, 0.0f);
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 62, 2);
        glDrawArrays(GL_LINES, 63, 2);
        glDrawArrays(GL_LINES, 64, 2);
        glDrawArrays(GL_LINES, 65, 2);

        // Leva strana nosaca rucke
        glUseProgram(sharedShader);
        glUniform3f(glGetUniformLocation(sharedShader, "color"), 0.6f, 0.6f, 0.6f);
        glBindVertexArray(VAO[33]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Desna strana nosaca rucke
        glBindVertexArray(VAO[34]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Rucka
        glUniform3f(glGetUniformLocation(sharedShader, "color"), 0.2f, 0.2f, 0.2f);
        glBindVertexArray(VAO[35]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


        glfwSwapBuffers(window);

        // Ograničavanje frejmova po sekundi
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - previousTime;

        if (deltaTime < TARGET_FRAME_TIME) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<int>((TARGET_FRAME_TIME - deltaTime) * 1000))
            );
        }

        previousTime = glfwGetTime();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    // Brisanje resursa
    glDeleteTextures(1, &meshTexture);
    glDeleteTextures(1, &backgroundTexture);
    glDeleteTextures(1, &scrollTextureShader);

    glDeleteBuffers(37, VBO);
    glDeleteVertexArrays(37, VAO);

    // Brisanje shader programa
    glDeleteProgram(sharedShader);
    glDeleteProgram(membraneShader);
    glDeleteProgram(textureShader);
    glDeleteProgram(backgroundShader);
    glDeleteProgram(scrollTextureShader);
    glDeleteProgram(basicShader);

    // Terminate GLFW (Sve OK - batali program)
    glfwTerminate();
    return 0;

}

unsigned int compileShader(GLenum type, const char* source)
{
    //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
    //Citanje izvornog koda iz fajla
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
        const char* sourceCode = temp.c_str(); //Izvorni kod sejdera koji citamo iz fajla na putanji "source"

    int shader = glCreateShader(type); //Napravimo prazan sejder odredjenog tipa (vertex ili fragment)
    
    int success; //Da li je kompajliranje bilo uspjesno (1 - da)
    char infoLog[512]; //Poruka o gresci (Objasnjava sta je puklo unutar sejdera)
    glShaderSource(shader, 1, &sourceCode, NULL); //Postavi izvorni kod sejdera
    glCompileShader(shader); //Kompajliraj sejder

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //Provjeri da li je sejder uspjesno kompajliran
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog); //Pribavi poruku o gresci
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
    //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

    unsigned int program; //Objedinjeni sejder
    unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
    unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

    program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

    //Zakaci verteks i fragment sejdere za objedinjeni program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
    glValidateProgram(program); //Izvrsi provjeru novopecenog programa

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    //Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

void generateCircle(float* circleVertices, float centerX, float centerY, float radius) {
    circleVertices[0] = centerX; 
    circleVertices[1] = centerY;
    for (int i = 0; i <= CRES; i++) {
        float angle = (2.0f * 3.141592f / CRES) * i;
        circleVertices[2 + i * 2] = centerX + radius * cos(angle);
        circleVertices[3 + i * 2] = centerY + radius * sin(angle);
    }
}

static unsigned loadImageToTexture(const char* filePath) {
    stbi_set_flip_vertically_on_load(true);
    int TextureWidth, TextureHeight, TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData == NULL) {
        std::cout << "Greska pri ucitavanju teksture: " << filePath << std::endl;
        return 0;
    }

    std::cout << "Tekstura ucitana: " << filePath << std::endl;
    std::cout << "Dimenzije teksture: " << TextureWidth << " x " << TextureHeight << std::endl;
    std::cout << "Broj kanala: " << TextureChannels << std::endl;

    unsigned int Texture;
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);

    GLint format = (TextureChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, TextureWidth, TextureHeight, 0, format, GL_UNSIGNED_BYTE, ImageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(ImageData);
    return Texture;
}

void updateProgressBar(float sliderPosition, unsigned int VBO) {
    float progressBarWidth = 0.4f;
    float progressBarHeight = 0.02f;
    float progressBarY = -0.25f;

    float progressBarFill = (sliderPosition + 0.2f) / 0.4f; // Normalizacija vrednosti u opsegu [0.0, 1.0]
    float filledWidth = progressBarFill * progressBarWidth;

    // Definisanje verteksa za popunjeni deo progress bara
    float progressBarFillVertices[] = {
        -progressBarWidth / 2, progressBarY - progressBarHeight / 2,
        -progressBarWidth / 2 + filledWidth, progressBarY - progressBarHeight / 2,
        -progressBarWidth / 2, progressBarY + progressBarHeight / 2,
        -progressBarWidth / 2 + filledWidth, progressBarY + progressBarHeight / 2
    };

    // Ažuriranje VBO podataka
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(progressBarFillVertices), progressBarFillVertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    // yOffset nam govori da li je tockic pomeren napred (pozitivno) ili nazad (negativno)
    scaleIndicatorOffset += 0.01f * static_cast<float>(yOffset);

    // Ogranicavamo kazaljku na ivice skale
    if (scaleIndicatorOffset < -0.04f) { // Leva ivica
        scaleIndicatorOffset = -0.04f;
    }
    else if (scaleIndicatorOffset > 1.44f) { // Desna ivica
        scaleIndicatorOffset = 1.44f;
    }
    std::cout << "Scale indicator offset: " << scaleIndicatorOffset << std::endl;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (action == GLFW_RELEASE) return;
    bool isDown = action == GLFW_PRESS;
    switch (key) {
    case GLFW_KEY_F:
        FMon = true;
        break;
    case GLFW_KEY_A:
        FMon = false;
        break;
    case GLFW_KEY_UP:
        if (antennaOffset < -0.2) {
            antennaOffset = round((antennaOffset + 0.01f) * 100.0) / 100.0;
        }
        break;
    case GLFW_KEY_DOWN:
        if (antennaOffset > -0.55f) { 
            antennaOffset = round((antennaOffset - 0.01f) * 100.0) / 100.0;
        }
        break;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Dobijanje pozicije kursora u pikselima
            double xPos, yPos;
            glfwGetCursorPos(window, &xPos, &yPos);

            // Dobijanje dimenzija prozora
            int width, height;
            glfwGetWindowSize(window, &width, &height);

            // Konverzija koordinata u OpenGL normalizovani prostor [-1, 1]
            float normX = 2.0f * (xPos / width) - 1.0f;
            float normY = 1.0f - 2.0f * (yPos / height); // OpenGL ima obrnutu osu Y

            // Definicija centra i radijus dugmeta u normalizovanim koordinatama - provera za ON/OFF button
            const float buttonCenterX = 0.8f;  // X koordinata centra dugmeta
            const float buttonCenterY = 0.025f; // Y koordinata centra dugmeta
            const float buttonRadius = 0.05f; // Poluprecnik dugmeta

            // da li je klik unutar radijusa dugmeta
            float dx = normX - buttonCenterX;
            float dy = normY - buttonCenterY;

            if ((dx * dx + dy * dy) <= (buttonRadius * buttonRadius)) {
                if (radioOn) {
                    lastProgressBarFill = progressBarFill;
                    progressBarFill = 0.0f;
                }
                else {
                    progressBarFill = lastProgressBarFill;
                }
                radioOn = !radioOn;
                std::cout << "Kliknuto na dugme! Stanje radioOn: " << (radioOn ? "Ukljuceno" : "Iskljuceno") << std::endl;
                return;
            }

            // Provera za slider (ogranicavanje na podrucje slidera)
            const float sliderRadius = 0.05f;
            float sliderCenterX = sliderPosition;
            float sliderCenterY = -0.2f;

            dx = normX - sliderCenterX;
            dy = normY - sliderCenterY;

            if ((dx * dx + dy * dy) <= (sliderRadius * sliderRadius)) {
                sliderDragging = true;  // Aktivira prevlacenje slidera
                std::cout << "Slider dragging started" << std::endl;
                return;
            }
        }
        else if (action == GLFW_RELEASE) {
            if (sliderDragging) {
                // Prestanak prevlacenja slidera
                sliderDragging = false;
                std::cout << "Slider released at position: " << sliderPosition << std::endl;
            }
        }
    }
}


void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos) {
    if (sliderDragging) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        float normX = 2.0f * (xPos / width) - 1.0f;

        // Ogranici poziciju slidera
        sliderPosition = fmaxf(-0.2f, fminf(0.2f, normX));

        // Normalizacija sliderPosition za progress bar popunjenost
        progressBarFill = (sliderPosition + 0.2f) / 0.4f; // Popunjenost od 0.0 do 1.0
        lastProgressBarFill = progressBarFill;
        std::cout << "Slider moved to position: " << sliderPosition << ", Progress bar fill: " << progressBarFill << std::endl;
    }
}