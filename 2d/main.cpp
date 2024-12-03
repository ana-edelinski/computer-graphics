    // Autor: Ana Edelinski RA 165/2021
    // Zadatak 5: Radio

    #define _CRT_SECURE_NO_WARNINGS
    #define CRES 100 // Circle Resolution = Rezolucija kruga
    #define STB_IMAGE_IMPLEMENTATION

    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <GL/glew.h>   //Omogucava upotrebu OpenGL naredbi
    #include <GLFW/glfw3.h> //Olaksava pravljenje i otvaranje prozora (konteksta) sa OpenGL sadrzajem
    #include "stb_image.h"

    bool radioOn = false;
    float timeElapsed = 0.0f; // Proteklo vreme za animaciju lampice
    float volumeBarIndicatorOffset = 0.0f;
    float sliderPosition = 0.0f; // Pozicija klizača u opsegu [-0.5, 0.5]
    bool sliderDragging = false;    // Da li korisnik trenutno prevlači klizač
    float speakerMembraneScaleLeft = 1.0f;
    float speakerMembraneScaleRight = 1.0f;
    float progressBarWidth = 0.4f; // Širina progress bara (iste širine kao slider)
    float progressBarHeight = 0.02f; // Visina progress bara
    float progressBarY = -0.25f; // Y koordinata (ispod slidera)
    float progressBarFill = 0.0f; // Popunjenost (od 0.0f do 1.0f)
    float lastProgressBarFill = 0.0f; // Memorisana popunjenost progress bara
    float displayY = -0.35f; // Y koordinata displeja
    float displayWidth = 0.4f; // Širina displeja (ista kao progress bar)
    float displayHeight = 0.1f; // Visina displeja
    bool FMon = true;

    unsigned int compileShader(GLenum type, const char* source);     //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
    unsigned int createShader(const char* vsSource, const char* fsSource);   //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource i Fragment sejdera na putanji fsSource
    static unsigned loadImageToTexture(const char* filePath);
    void generateCircle(float* circleVertices, float centerX, float centerY, float radius);
    void updateProgressBar(float sliderPosition, unsigned int VBO);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

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

                // Definišemo centar i radijus dugmeta u normalizovanim koordinatama - provera za ON/OFF button
                const float buttonCenterX = 0.8f;  // X koordinata centra dugmeta
                const float buttonCenterY = 0.125f; // Y koordinata centra dugmeta
                const float buttonRadius = 0.05f; // Poluprečnik dugmeta

                // Proveravamo da li je klik unutar radijusa dugmeta
                float dx = normX - buttonCenterX;
                float dy = normY - buttonCenterY;

                if ((dx * dx + dy * dy) <= (buttonRadius * buttonRadius)) {
                    // Menjamo stanje radioOn (uključen/isključen)
                    if (radioOn) {
                        lastProgressBarFill = progressBarFill;
                        progressBarFill = 0.0f;
                    }
                    else {
                        progressBarFill = lastProgressBarFill;
                    }
                    radioOn = !radioOn;
                    std::cout << "Kliknuto na dugme! Stanje radioOn: " << (radioOn ? "Uključeno" : "Isključeno") << std::endl;
                    return;
                }
            
                // Provera za slider (ograničavanje na područje slidera)
                const float sliderRadius = 0.05f;
                float sliderCenterX = sliderPosition;
                float sliderCenterY = -0.2f;

                dx = normX - sliderCenterX;
                dy = normY - sliderCenterY;

                if ((dx * dx + dy * dy) <= (sliderRadius * sliderRadius)) {
                    sliderDragging = true;  // Aktivira prevlačenje klizača
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

            // Ograniči poziciju slidera
            sliderPosition = fmaxf(-0.2f, fminf(0.2f, normX));

            // Normalizacija sliderPosition za progress bar popunjenost
            progressBarFill = (sliderPosition + 0.2f) / 0.4f; // Popunjenost od 0.0 do 1.0
            lastProgressBarFill = progressBarFill;
            std::cout << "Slider moved to position: " << sliderPosition << ", Progress bar fill: " << progressBarFill << std::endl;
        }
    }


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
        unsigned int wWidth = 500;
        unsigned int wHeight = 500;
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

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++

        unsigned int radioBodyShader = createShader("radiobody.vert", "radiobody.frag");
        unsigned int speakerShader = createShader("speaker.vert", "speaker.frag");
        unsigned int membraneShader = createShader("membrane.vert", "membrane.frag");
        unsigned int lineShader = createShader("line.vert", "line.frag");
        unsigned int textureShader = createShader("texture.vert", "texture.frag");
        unsigned int radioOnOffButtonShader = createShader("basic.vert", "basic.frag");
        unsigned int radioOnOffButtonIndicatorShader = createShader("basic.vert", "basic.frag");
        unsigned int radioOnOffButtonIndicatorLineShader = createShader("basic.vert", "basic.frag");
        unsigned int radioOnOffLampShader = createShader("basic.vert", "basic.frag");
        unsigned int AMFMRailShader = createShader("basic.vert", "basic.frag");
        unsigned int AMFMRailOutlineShader = createShader("basic.vert", "basic.frag");
        unsigned int AMFMSwitchShader = createShader("basic.vert", "basic.frag");
        unsigned int AMFMSwitchOutlineShader = createShader("basic.vert", "basic.frag");
        unsigned int AMShader = createShader("texture.vert", "texture.frag");
        unsigned int FMShader = createShader("texture.vert", "texture.frag");


        unsigned int VAO[28];
        unsigned int VBO[28];
        glGenVertexArrays(28, VAO);
        glGenBuffers(28, VBO);
        unsigned int stride;


        float radioBodyVertices[] = {
            -0.9, -0.7,  
             0.9, -0.7,  
            -0.9,  0.3,  
             0.9,  0.3  
        };

        float antennaVertices[] = {
            -0.8, 0.3,   
            -0.7, 0.3,   
            -0.8, 0.9,   
            -0.7, 0.9    
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
            // X    Y       S    T
             0.25, -0.65,  0.0, 0.0,
             0.75, -0.65,  1.0, 0.0,
             0.25, -0.15,  0.0, 1.0,
             0.75, -0.15,  1.0, 1.0
        };

        float leftSpeakerMeshVertices[] = {
            // X    Y       S    T
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
            // X      Y
            0.8, 0.125,
            0.8, 0.165
        };

        float sliderBarVertices[] = {
            -0.2f, -0.2f,
             0.2f, -0.2f
        };

        float progressBarVertices[] = {
        -progressBarWidth / 2, progressBarY - progressBarHeight / 2,  // Levo dole
         progressBarWidth / 2, progressBarY - progressBarHeight / 2,  // Desno dole
        -progressBarWidth / 2, progressBarY + progressBarHeight / 2,  // Levo gore
         progressBarWidth / 2, progressBarY + progressBarHeight / 2   // Desno gore
        };

        float progressBarFillVertices[] = {
        -progressBarWidth / 2, progressBarY - progressBarHeight / 2,
        -progressBarWidth / 2, progressBarY + progressBarHeight / 2,
        -progressBarWidth / 2, progressBarY - progressBarHeight / 2,
        -progressBarWidth / 2, progressBarY + progressBarHeight / 2
        };

        float displayVertices[] = {
            -displayWidth / 2, displayY - displayHeight / 2, // Levo dole
             displayWidth / 2, displayY - displayHeight / 2, // Desno dole
            -displayWidth / 2, displayY + displayHeight / 2, // Levo gore
             displayWidth / 2, displayY + displayHeight / 2  // Desno gore
        };

        float AMFMRailVertices[] =
        {
            // X      Y
            -0.2, -0.5,
            -0.05, -0.5,
            -0.2,  -0.25,
            -0.05,  -0.25
        };

        float AMFMSwitchVertices[] =
        {
            // X      Y
            -0.2, -0.5,
            -0.05, -0.5,
            -0.2,  -0.4,
            -0.05,  -0.4
        };

        float AMVertices[] =    //izmena??
        {   //X    Y      S    T 
            -0.2, -0.2, 0.0, 1.0,
            -0.05, -0.2, 1.0, 1.0,
            -0.2,  -0.1, 0.0, 0.0,
            -0.05,  -0.1, 1.0, 0.0
        };


        float FMVertices[] =    //izmena??
        {   //X    Y      S    T 
            -0.2, -0.65, 0.0, 1.0,  // Gornja leva
            -0.05, -0.65, 1.0, 1.0, // Gornja desna
            -0.2,  -0.55, 0.0, 0.0, // Donja leva
            -0.05,  -0.55, 1.0, 0.0  // Donja desna
        };

        float AMFMRailOutlineVertices[] =
        {
            // X      Y
            -0.2, -0.5,
            -0.05, -0.5,
            -0.05,  -0.25,
            -0.2,  -0.25,
            -0.2, -0.5
        };

        float AMFMSwitchOutlineVertices[] =
        {
            // X      Y
            -0.2, -0.5,
            -0.05, -0.5,
            -0.05,  -0.4,
            -0.2,  -0.4,
            -0.2, -0.5
        };




        //Povezivanje podataka sa VAO i VBO

        // Telo radija
        glBindVertexArray(VAO[0]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(radioBodyVertices), radioBodyVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Antena
        glBindVertexArray(VAO[1]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(antennaVertices), antennaVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Levi zvučnik
        glBindVertexArray(VAO[2]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(leftSpeakerVertices), leftSpeakerVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Desni zvučnik
        glBindVertexArray(VAO[3]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rightSpeakerVertices), rightSpeakerVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // *** Učitavanje teksture ***
        unsigned int meshTexture = loadImageToTexture("resources/mesh.png");
        if (meshTexture == 0) {
            std::cout << "Greska pri ucitavanju teksture!" << std::endl;
            return 4;
        }

        // Leva membrana
        glBindVertexArray(VAO[4]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(leftMembraneVertices), leftMembraneVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);   //deaktivira VAO

        // Desna membrana
        glBindVertexArray(VAO[5]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rightMembraneVertices), rightMembraneVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Leva membrana mala
        glBindVertexArray(VAO[6]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(smallLeftMembraneVertices), smallLeftMembraneVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Desna membrana mala
        glBindVertexArray(VAO[7]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(smallRightMembraneVertices), smallRightMembraneVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Leva mrezica
        glBindVertexArray(VAO[8]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(leftSpeakerMeshVertices), leftSpeakerMeshVertices, GL_STATIC_DRAW);
        // Pozicija (layout(location = 0))
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);   //omoguci atribut na lokaciji 0
        // Tekstura (layout(location = 1))
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);   // Omogući atribut na lokaciji 1
        glBindVertexArray(0);   //deaktivira vao

        // Desna mrezica
        glBindVertexArray(VAO[9]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rightSpeakerMeshVertices), rightSpeakerMeshVertices, GL_STATIC_DRAW);    
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        // Horizontalna linija
        glBindVertexArray(VAO[10]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(separatorLineVertices), separatorLineVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // RADIO ON/OFF BUTTON
        float radioOnOffButton[CRES * 2 + 4];
        float r = 0.05;
        radioOnOffButton[0] = 0.8;
        radioOnOffButton[1] = 0.125;
        for (int i = 0; i <= CRES; i++)
        {

            radioOnOffButton[2 + 2 * i] = radioOnOffButton[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
            radioOnOffButton[2 + 2 * i + 1] = radioOnOffButton[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
        }
        glBindVertexArray(VAO[11]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[11]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffButton), radioOnOffButton, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // RADIO ON/OFF BUTTON INDICATOR
        float radioOnOffButtonIndicator[CRES * 2 + 4];
        r = 0.04;
        radioOnOffButtonIndicator[0] = 0.8;
        radioOnOffButtonIndicator[1] = 0.125;
        for (int i = 0; i <= CRES; i++)
        {
            radioOnOffButtonIndicator[2 + 2 * i] = radioOnOffButtonIndicator[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
            radioOnOffButtonIndicator[2 + 2 * i + 1] = radioOnOffButtonIndicator[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
        }
        glBindVertexArray(VAO[12]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[12]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffButtonIndicator), radioOnOffButtonIndicator, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // RADIO ON/OFF BUTTON CONTENT INDICATOR INNER
        float radioOnOffButtonIndicatorInner[CRES * 2 + 4];
        r = 0.035;
        radioOnOffButtonIndicatorInner[0] = 0.8;
        radioOnOffButtonIndicatorInner[1] = 0.125;
        for (int i = 0; i <= CRES; i++)
        {
            radioOnOffButtonIndicatorInner[2 + 2 * i] = radioOnOffButtonIndicatorInner[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
            radioOnOffButtonIndicatorInner[2 + 2 * i + 1] = radioOnOffButtonIndicatorInner[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
        }
        glBindVertexArray(VAO[13]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[13]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffButtonIndicatorInner), radioOnOffButtonIndicatorInner, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // RADIO ON/OFF BUTTON INDICATOR LINE
        stride = 2 * sizeof(float);
        glBindVertexArray(VAO[14]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[14]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRadioButtonIndicatorLine), verticesRadioButtonIndicatorLine, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        // RADIO ON/OFF LAMP
        float radioOnOffLamp[CRES * 2 + 4];
        r = 0.02;
        radioOnOffLamp[0] = 0.725;
        radioOnOffLamp[1] = 0.125;
        for (int i = 0; i <= CRES; i++)
        {
            radioOnOffLamp[2 + 2 * i] = radioOnOffLamp[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
            radioOnOffLamp[2 + 2 * i + 1] = radioOnOffLamp[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
        }
        glBindVertexArray(VAO[15]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[15]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffLamp), radioOnOffLamp, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // RADIO ON/OFF LAMP LIGHT
        float radioOnOffLampLight[CRES * 2 + 4];
        r = 0.01;
        radioOnOffLampLight[0] = 0.725;
        radioOnOffLampLight[1] = 0.125;
        for (int i = 0; i <= CRES; i++)
        {
            radioOnOffLampLight[2 + 2 * i] = radioOnOffLampLight[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
            radioOnOffLampLight[2 + 2 * i + 1] = radioOnOffLampLight[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
        }
        glBindVertexArray(VAO[16]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[16]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(radioOnOffLampLight), radioOnOffLampLight, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Horizontalna traka za slider
        glBindVertexArray(VAO[17]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[17]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sliderBarVertices), sliderBarVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        // Krug slidera
        float sliderVertices[(CRES + 2) * 2];
        generateCircle(sliderVertices, sliderPosition, -0.2f, 0.03f);

        glBindVertexArray(VAO[18]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[18]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sliderVertices), sliderVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        // Progress bar
        glBindVertexArray(VAO[19]); 
        glBindBuffer(GL_ARRAY_BUFFER, VBO[19]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarVertices), progressBarVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Progress bar indikator
        glBindVertexArray(VAO[20]); 
        glBindBuffer(GL_ARRAY_BUFFER, VBO[20]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarFillVertices), progressBarFillVertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);


        // Displej
        glBindVertexArray(VAO[21]); 
        glBindBuffer(GL_ARRAY_BUFFER, VBO[21]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(displayVertices), displayVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // AM/FM Rail
        stride = 2 * sizeof(float);
        glBindVertexArray(VAO[22]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[22]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(AMFMRailVertices), AMFMRailVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        // AM/FM Switch
        stride = 2 * sizeof(float);
        glBindVertexArray(VAO[23]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[23]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(AMFMSwitchVertices), AMFMSwitchVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        // AM
        stride = (2 + 2) * sizeof(float);
        glBindVertexArray(VAO[24]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[24]);
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
        unsigned uTexLoc1 = glGetUniformLocation(AMShader, "uTex");
        glUniform1i(uTexLoc1, 0);

        // FM
        stride = (2 + 2) * sizeof(float);
        glBindVertexArray(VAO[25]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[25]);
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
        unsigned uTexLoc2 = glGetUniformLocation(FMShader, "uTex");
        glUniform1i(uTexLoc2, 0);

        // AM/FM Rail Outline
        stride = 2 * sizeof(float);
        glBindVertexArray(VAO[26]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[26]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(AMFMRailOutlineVertices), AMFMRailOutlineVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // AM/FM Switch Outline
        stride = 2 * sizeof(float);
        glBindVertexArray(VAO[27]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[27]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(AMFMSwitchOutlineVertices), AMFMSwitchOutlineVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);


        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        double previousTime = glfwGetTime();

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++

        // Glavna petlja
        while (!glfwWindowShouldClose(window))  //Beskonacna petlja iz koje izlazimo tek kada prozor treba da se zatvori
        {
            glLineWidth(1.0f);

            glfwPollEvents();  

            // Animacija vibracija membrane (sinusoidni pokret) - samo ako je radio uključen i slider nije skroz levo
            float membraneAmplitude = 0.0f; // Podrazumevano nema vibracija

            if (radioOn && sliderPosition > -0.2f) { // Provera da slider nije skroz levo (-0.2f je minimalna vrednost slider-a)
                volumeBarIndicatorOffset = (sliderPosition + 0.2f) / 0.4f; // Normalizacija slidera u opsegu [0, 1]
                membraneAmplitude = volumeBarIndicatorOffset * 0.05f; // Maksimalna amplituda je 0.05
                double time = glfwGetTime();
            
                // Sinhronizovane oscilacije zvučnika
                float oscillation = 1.0f + membraneAmplitude * sin(time * 20.0f);
                speakerMembraneScaleLeft = oscillation;
                speakerMembraneScaleRight = oscillation;
            }
            else {
                // Resetovanje skale kada je radio isključen ili slider skroz levo
                speakerMembraneScaleLeft = 1.0f;
                speakerMembraneScaleRight = 1.0f;
            }




            // Animacija vibracija membrane (sinusoidni pokret)
            double time = glfwGetTime();
            float oscillation = 1.0f + membraneAmplitude * sin(time * 20.0f);
            speakerMembraneScaleLeft = oscillation;
            speakerMembraneScaleRight = oscillation;


            // Brisanje ekrana
            glClearColor(1.0, 1.0, 1.0, 1.0);   //bela pozadina
            glClear(GL_COLOR_BUFFER_BIT);

            // [KOD ZA CRTANJE]

            glUseProgram(radioBodyShader);
            unsigned int uBodyColorLoc = glGetUniformLocation(radioBodyShader, "color");

            int indexRGB = 0;
            if (radioOn) {
                indexRGB = 1;
            }

            float onOffRGB[3] = { 0.0f, 0.0f, 0.0f };
            float lampRGB[3] = { 0.0f, 0.0f, 0.0f };
            onOffRGB[indexRGB] = 0.4f;
            lampRGB[indexRGB] = 1.0f;

            // Telo radija
            glUniform3f(uBodyColorLoc, 70 / 255.0f, 70 / 255.0f, 70 / 255.0f);
            glBindVertexArray(VAO[0]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // Antena
            glUniform3f(uBodyColorLoc, 176 / 255.0f, 176 / 255.0f, 176 / 255.0f);
            glBindVertexArray(VAO[1]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glUseProgram(speakerShader);
            unsigned int uSpeakerColorLoc = glGetUniformLocation(speakerShader, "color");

            // Levi zvučnik
            glUniform3f(uSpeakerColorLoc, 245 / 255.0f, 245 / 255.0f, 220 / 255.0f);
            glBindVertexArray(VAO[2]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // Desni zvučnik
            glUniform3f(uSpeakerColorLoc, 245 / 255.0f, 245 / 255.0f, 220 / 255.0f);
            glBindVertexArray(VAO[3]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


            glUseProgram(membraneShader);

            unsigned int uMembraneColorLoc = glGetUniformLocation(membraneShader, "color"); //

            // Leva membrana - velika
            glUseProgram(membraneShader);
            glUniform2f(glGetUniformLocation(membraneShader, "center"), -0.5, -0.4);
            glUniform1f(glGetUniformLocation(membraneShader, "scale"), speakerMembraneScaleLeft);
            glUniform3f(glGetUniformLocation(membraneShader, "color"), 0.3f, 0.3f, 0.3f); // Svetlija boja za velike membrane
            glBindVertexArray(VAO[4]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

            // Desna membrana - velika
            glUniform2f(glGetUniformLocation(membraneShader, "center"), 0.5, -0.4);
            glUniform1f(glGetUniformLocation(membraneShader, "scale"), speakerMembraneScaleRight);
            glBindVertexArray(VAO[5]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

            // Leva membrana - mala
            glUniform2f(glGetUniformLocation(membraneShader, "center"), -0.5, -0.4);
            glUniform1f(glGetUniformLocation(membraneShader, "scale"), speakerMembraneScaleLeft);
            glUniform3f(glGetUniformLocation(membraneShader, "color"), 0.2f, 0.2f, 0.2f); // Tamnija boja za male membrane
            glBindVertexArray(VAO[6]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

            // Desna membrana - mala
            glUniform2f(glGetUniformLocation(membraneShader, "center"), 0.5, -0.4);
            glUniform1f(glGetUniformLocation(membraneShader, "scale"), speakerMembraneScaleRight);
            glBindVertexArray(VAO[7]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

            glUseProgram(lineShader);

            // Horizontalna linija
            unsigned int uLineColorLoc = glGetUniformLocation(lineShader, "color");
            glUniform3f(uLineColorLoc, 0.0f, 0.0f, 0.0f);
            glBindVertexArray(VAO[10]);
            glDrawArrays(GL_LINES, 0, 2);


            // Renderovanje mrežice zvučnika
            glUseProgram(textureShader);

            // Setuj uniform za teksturu
            unsigned int textureUniform = glGetUniformLocation(textureShader, "uTex");
            glUniform1i(textureUniform, 0);  // Teksturna jedinica 0

            // Mrezica levog zvucnika
            glBindVertexArray(VAO[8]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, meshTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);

            // Mrezica desnog zvucnika
            glBindVertexArray(VAO[9]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
            glUseProgram(0);


            // Izračunavanje proteklog vremena za animaciju
            double currentTime = glfwGetTime();
            double deltaTime = currentTime - previousTime;
            previousTime = currentTime;

            // Resetovanje vremena ako je radio isključen
            if (!radioOn) {
                timeElapsed = 0.0f; // Resetuje animaciju ako je radio isključen
            }

            // Izračunavanje interpolirane boje za lampicu kada je radio uključen
            float lampRed = 0.0f, lampGreen = 0.0f, lampBlue = 0.0f; // Neutralna boja kada je radio isključen
            if (radioOn) {
                timeElapsed += deltaTime; // Ažuriranje vremena za animaciju
                float intensity = (sin(timeElapsed * 2.0f) + 1.0f) / 2.0f; // Intenzitet varira od 0 do 1
                lampRed = 1.0f; 
                lampGreen = 1.0f - 0.5f * intensity; 
                lampBlue = 1.0f - intensity; 
            }

            // Renderovanje ON/OFF buttona
            glUseProgram(radioOnOffButtonShader);

            // ON/OFF Button
            glUniform3f(glGetUniformLocation(radioOnOffButtonShader, "color"), 0.2f, 0.2f, 0.2f);
            glBindVertexArray(VAO[11]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffButton) / (2 * sizeof(float)));

            // Renderovanje ON/OFF button indikatora
            glUseProgram(radioOnOffButtonIndicatorShader);

            // Postavljanje boje kruga: crno kada je isključen, belo kada je uključen
            float indicatorR = radioOn ? 1.0f : 0.0f; // Crvena komponenta
            float indicatorG = radioOn ? 1.0f : 0.0f; // Zelena komponenta
            float indicatorB = radioOn ? 1.0f : 0.0f; // Plava komponenta

            // Postavljanje uniforma za boju indikatora
            glUniform3f(glGetUniformLocation(radioOnOffButtonIndicatorShader, "color"), indicatorR, indicatorG, indicatorB);

            // Krug ON/OFF button indikatora
            glBindVertexArray(VAO[12]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffButtonIndicator) / (2 * sizeof(float)));


            // Unutrasnji deo ON/OFF button indikatora
            glUniform3f(glGetUniformLocation(radioOnOffButtonIndicatorShader, "color"), 0.2f, 0.2f, 0.2f);
            glBindVertexArray(VAO[13]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffButtonIndicatorInner) / (2 * sizeof(float)));



            // Renderovanje sredisnje linije na ON/OFF button indikatoru
            glUseProgram(radioOnOffButtonIndicatorLineShader);


            // Postavljanje boje linije: crna kada je radio isključen, bela kada je uključen
            float lineR = radioOn ? 1.0f : 0.0f; 
            float lineG = radioOn ? 1.0f : 0.0f; 
            float lineB = radioOn ? 1.0f : 0.0f; 

            // Postavljanje uniform boje linije
            glUniform3f(glGetUniformLocation(radioOnOffButtonIndicatorLineShader, "color"), lineR, lineG, lineB);

            // Srednja linija na ON/OFF button indikatoru
            glBindVertexArray(VAO[14]);
            glLineWidth(2.0f);
            glDrawArrays(GL_LINES, 0, 2);


            // Renderovanje lampice
            glUseProgram(radioOnOffLampShader);

            // Lampica osnovna
            glUniform3f(glGetUniformLocation(radioOnOffLampShader, "color"), 0.2f, 0.2f, 0.2f);
            glBindVertexArray(VAO[15]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffLamp) / (2 * sizeof(float)));

            // Boja lampice
            glUniform3f(glGetUniformLocation(radioOnOffLampShader, "color"), lampRed, lampGreen, lampBlue);
            glBindVertexArray(VAO[16]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(radioOnOffLamp) / (2 * sizeof(float)));


            // Slider i traka
            volumeBarIndicatorOffset = (sliderPosition + 0.5f); // Skaliranje u opsegu [0, 1]
            glUseProgram(lineShader);
            glUniform3f(glGetUniformLocation(lineShader, "color"), 0.7f, 0.7f, 0.7f);
            glBindVertexArray(VAO[17]);
            glDrawArrays(GL_LINES, 0, 2);

            // Krug slidera
            float sliderVertices[(CRES + 2) * 2];
            generateCircle(sliderVertices, sliderPosition, -0.2f, 0.03f);

            glBindVertexArray(VAO[18]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[18]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(sliderVertices), sliderVertices, GL_DYNAMIC_DRAW);

            //glBufferData(GL_ARRAY_BUFFER, sizeof(sliderVertices), sliderVertices, GL_STATIC_DRAW);
            glUseProgram(radioOnOffButtonShader);
            glUniform3f(glGetUniformLocation(radioOnOffButtonShader, "color"), 0.4f, 0.4f, 0.4f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

            // Progress bar
            updateProgressBar(sliderPosition, VBO[20]);

            glUseProgram(lineShader);
            glUniform3f(glGetUniformLocation(lineShader, "color"), 0.7f, 0.7f, 0.7f);
            glBindVertexArray(VAO[19]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // Progress bar indikator
            if (!radioOn) {
                glUniform3f(glGetUniformLocation(lineShader, "color"), 0.7f, 0.7f, 0.7f); // Siva boja kada je isključen
            }
            else {
                glUniform3f(glGetUniformLocation(lineShader, "color"), 0.0f, 1.0f, 0.0f); // Zelena kada je uključen
            }
            glBindVertexArray(VAO[20]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // Renderovanje displeja
            glUseProgram(radioBodyShader); // Shader za prikaz displeja
            glUniform3f(glGetUniformLocation(radioBodyShader, "color"), 0.0f, 0.0f, 0.0f); // Crna boja
            glBindVertexArray(VAO[21]); // Displej VAO
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // AM/FM RAIL
            glUseProgram(AMFMRailShader);
            glUniform3f(glGetUniformLocation(AMFMRailShader, "color"), 0.45f, 0.45f, 0.45f);
            glBindVertexArray(VAO[22]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // AM/FM SWITCH
            glUseProgram(AMFMSwitchShader);
            glUniform2f(glGetUniformLocation(AMFMSwitchShader, "offset"), 0.0f, FMon ? 0.0f : -0.15f);
            glUniform3f(glGetUniformLocation(AMFMSwitchShader, "color"), 0.5f, 0.5f, 0.5f);
            glBindVertexArray(VAO[23]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // AM
            glUseProgram(AMShader);
            glBindVertexArray(VAO[24]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, AMTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);

            // FM
            glUseProgram(AMShader);
            glBindVertexArray(VAO[25]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, FMTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);

            // AM/FM Rail Outlines
            glUseProgram(AMFMRailOutlineShader);
            glUniform3f(glGetUniformLocation(AMFMRailOutlineShader, "color"), 0.0f, 0.0f, 0.0f);
            glBindVertexArray(VAO[26]);
            glDrawArrays(GL_LINES, 0, 2);
            glDrawArrays(GL_LINES, 1, 2);
            glDrawArrays(GL_LINES, 2, 2);
            glDrawArrays(GL_LINES, 3, 2);

            // AM/FM Switch Outlines
            glUseProgram(AMFMSwitchOutlineShader);
            glUniform2f(glGetUniformLocation(AMFMSwitchShader, "offset"), 0.0f, FMon ? 0.0f : 0.15f);
            glUniform3f(glGetUniformLocation(AMFMSwitchOutlineShader, "color"), 0.0f, 0.0f, 0.0f);
            glBindVertexArray(VAO[27]);
            glDrawArrays(GL_LINES, 0, 2);
            glDrawArrays(GL_LINES, 1, 2);
            glDrawArrays(GL_LINES, 2, 2);
            glDrawArrays(GL_LINES, 3, 2);

            //// Renderovanje AM/FM Rail outline-a
            //glUseProgram(lineShader);
            //glUniform3f(glGetUniformLocation(lineShader, "color"), 0.0f, 0.0f, 0.0f); // Crna boja za outline
            //glBindVertexArray(VAO[26]);
            //glLineWidth(1.5f); // Podešavanje debljine linije
            //glDrawArrays(GL_LINE_LOOP, 0, 4);

            //// Renderovanje AM/FM Switch outline-a
            //glBindVertexArray(VAO[27]);
            //glDrawArrays(GL_LINE_LOOP, 0, 4);


            glfwSwapBuffers(window);
        }

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++

        // Brisanje resursa
        glDeleteTextures(1, &meshTexture);

        glDeleteBuffers(28, VBO);
        glDeleteVertexArrays(28, VAO);

        // Brisanje shader programa
        glDeleteProgram(radioBodyShader);
        glDeleteProgram(speakerShader);
        glDeleteProgram(membraneShader);
        glDeleteProgram(lineShader);
        glDeleteProgram(textureShader);
        glDeleteProgram(radioOnOffButtonShader);
        glDeleteProgram(radioOnOffButtonIndicatorShader);
        glDeleteProgram(radioOnOffButtonIndicatorLineShader);
        glDeleteProgram(radioOnOffLampShader);

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

    // Funkcija za generisanje kruga
    void generateCircle(float* circleVertices, float centerX, float centerY, float radius) {
        circleVertices[0] = centerX;  // Centar kruga
        circleVertices[1] = centerY;
        for (int i = 0; i <= CRES; i++) {
            float angle = (2.0f * 3.141592f / CRES) * i;
            circleVertices[2 + i * 2] = centerX + radius * cos(angle);
            circleVertices[3 + i * 2] = centerY + radius * sin(angle);
        }
    }

    static unsigned loadImageToTexture(const char* filePath) {
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

        // Računanje popunjenosti
        float progressBarFill = (sliderPosition + 0.2f) / 0.4f; // Normalizacija vrednosti u opsegu [0.0, 1.0]
        float filledWidth = progressBarFill * progressBarWidth;

        // Definisanje vertekse za popunjeni deo progress bara
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
        }
    }