// Autor: Ana Edelinski RA 165/2021
// Zadatak 5: Radio

#define _CRT_SECURE_NO_WARNINGS
#define CRES 100 // Circle Resolution = Rezolucija kruga

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>   //Omogucava upotrebu OpenGL naredbi
#include <GLFW/glfw3.h> //Olaksava pravljenje i otvaranje prozora (konteksta) sa OpenGL sadrzajem

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int compileShader(GLenum type, const char* source);     //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
unsigned int createShader(const char* vsSource, const char* fsSource);   //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource i Fragment sejdera na putanji fsSource
static unsigned loadImageToTexture(const char* filePath);
void generateCircle(float* circleVertices, float centerX, float centerY, float radius);



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

    // Inicijalizacija GLEW biblioteke
    if (glewInit() != GLEW_OK)   //Slicno kao glfwInit. GLEW_OK je predefinisani kod za uspjesnu inicijalizaciju sadrzan unutar biblioteke
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++
    unsigned int basicShader = createShader("basic.vert", "basic.frag");    //kreiram sejder tj povezujem sejdere u jedan sejderski program
    unsigned int textureShader = createShader("texture.vert", "texture.frag");

    //****

    unsigned VAO[6];    //vertex array object cuva konfiguraciju vertiksa
    glGenVertexArrays(6, VAO);
    unsigned VBO[6];    //vertex buffer object je memorija u kojoj se vertiksi cuvaju na grafickoj kartici
    glGenBuffers(6, VBO);

    

    //mali crni krug u zvucniku
    unsigned int smallMembraneVAO[2], smallMembraneVBO[2];
    glGenVertexArrays(2, smallMembraneVAO);
    glGenBuffers(2, smallMembraneVBO);

    //za levu mrezicu
    unsigned int leftMeshVAO, leftMeshVBO;
    glGenVertexArrays(1, &leftMeshVAO);
    glGenBuffers(1, &leftMeshVBO);

    // za desnu mrežicu
    unsigned int rightMeshVAO, rightMeshVBO;
    glGenVertexArrays(1, &rightMeshVAO);
    glGenBuffers(1, &rightMeshVBO);

    //za liniju
    unsigned int lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    //*****

    // Definisanje oblika radija
    float radioBodyVertices[] = {
        -0.9, -0.7,  // Donji levi ugao
         0.9, -0.7,  // Donji desni ugao
        -0.9,  0.3,  // Gornji levi ugao
         0.9,  0.3   // Gornji desni ugao
    };

    float antennaVertices[] = {
        -0.8, 0.3,   // Baza antene levo
        -0.7, 0.3,   // Baza antene desno
        -0.8, 0.9,   // Vrh antene levo
        -0.7, 0.9    // Vrh antene desno
    };


    float rightSpeakerVertices[] = {
        0.25, -0.65,  // Donji levi ugao
        0.75, -0.65,  // Donji desni ugao
        0.25, -0.15,  // Gornji levi ugao
        0.75, -0.15   // Gornji desni ugao
    };

    float leftSpeakerVertices[] = {
        -0.75, -0.65,  // Donji levi ugao
        -0.25, -0.65,  // Donji desni ugao
        -0.75, -0.15,  // Gornji levi ugao
        -0.25, -0.15   // Gornji desni ugao
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

    // Horizontalna linija iznad zvučnika
    float separatorLineVertices[] = {
        -0.9f, -0.1f, // Leva tačka
         0.9f, -0.1f  // Desna tačka
    };

    float leftMembraneVertices[(CRES + 2) * 2];
    generateCircle(leftMembraneVertices, -0.5, -0.4, 0.25f); // Centar (-0.5, -0.4), poluprečnik 0.25

    float rightMembraneVertices[(CRES + 2) * 2];
    generateCircle(rightMembraneVertices, 0.5, -0.4, 0.25f); // Centar (0.5, -0.4), poluprečnik 0.25

    float smallLeftMembraneVertices[(CRES + 2) * 2];
    generateCircle(smallLeftMembraneVertices, -0.5, -0.4, 0.15f); // Centar (-0.5, -0.4), poluprečnik 0.15

    float smallRightMembraneVertices[(CRES + 2) * 2];
    generateCircle(smallRightMembraneVertices, 0.5, -0.4, 0.15f); // Centar (0.5, -0.4), poluprečnik 0.15

    //povezujem podatke sa vao i vbo**********************

    // Telo radija
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(radioBodyVertices), radioBodyVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Antena
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(antennaVertices), antennaVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Levi zvučnik
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftSpeakerVertices), leftSpeakerVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Desni zvučnik
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightSpeakerVertices), rightSpeakerVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

    // Desna membrana
    glBindVertexArray(VAO[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightMembraneVertices), rightMembraneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    // Levi manji krug
    glBindVertexArray(smallMembraneVAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, smallMembraneVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(smallLeftMembraneVertices), smallLeftMembraneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Desni manji krug
    glBindVertexArray(smallMembraneVAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, smallMembraneVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(smallRightMembraneVertices), smallRightMembraneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //propusteno - leva mrezica
    glBindVertexArray(leftMeshVAO);
    glBindBuffer(GL_ARRAY_BUFFER, leftMeshVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftSpeakerMeshVertices), leftSpeakerMeshVertices, GL_STATIC_DRAW);
    // Pozicija (X, Y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Teksturne koordinate (S, T)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //???
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    //desna mrezica
    glBindVertexArray(rightMeshVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rightMeshVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightSpeakerMeshVertices), rightSpeakerMeshVertices, GL_STATIC_DRAW);    
    // Pozicija (X, Y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Teksturne koordinate (S, T)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //??
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //linija horizontalna
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(separatorLineVertices), separatorLineVertices, GL_STATIC_DRAW);
    // Povezivanje atributa za liniju
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //??
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //ovo se odnosi na basic shader za sada...
    //unsigned int uPosLoc = glGetUniformLocation(basicShader, "uPos"); //Mora biti POSLE pravljenja sejdera, inace ta uniforma ne postoji, kao i sejder
    unsigned int uColorLoc = glGetUniformLocation(basicShader, "color");    //za boje
    glPointSize(4);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    // Glavna petlja
    while (!glfwWindowShouldClose(window))  //Beskonacna petlja iz koje izlazimo tek kada prozor treba da se zatvori
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);


        glUseProgram(basicShader);

        // Telo radija
        glUniform3f(uColorLoc, 70 / 255.0f, 70 / 255.0f, 70 / 255.0f);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Antena
        glUniform3f(uColorLoc, 176 / 255.0f, 176 / 255.0f, 176 / 255.0f); 
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Levi zvučnik (mreža)
        glUniform3f(uColorLoc, 245 / 255.0f, 245 / 255.0f, 220 / 255.0f); 
        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Desni zvučnik (mreža)
        glUniform3f(uColorLoc, 245 / 255.0f, 245 / 255.0f, 220 / 255.0f); 
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Leva membrana
        glUniform3f(uColorLoc, 70 / 255.0f, 70 / 255.0f, 70 / 255.0f); 
        glBindVertexArray(VAO[4]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        // Desna membrana
        glUniform3f(uColorLoc, 70 / 255.0f, 70 / 255.0f, 70 / 255.0f);
        glBindVertexArray(VAO[5]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        // Levi manji krug
        glUniform3f(uColorLoc, 30 / 255.0f, 30 / 255.0f, 30 / 255.0f); 
        glBindVertexArray(smallMembraneVAO[0]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        // Desni manji krug
        glUniform3f(uColorLoc, 30 / 255.0f, 30 / 255.0f, 30 / 255.0f); 
        glBindVertexArray(smallMembraneVAO[1]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        glBindVertexArray(0);
        glUseProgram(0);

        // Renderovanje mrežice levog zvučnika
        glUseProgram(textureShader);

        // Setuj uniform za teksturu
        unsigned int textureUniform = glGetUniformLocation(textureShader, "uTex");
        glUniform1i(textureUniform, 0);  // Teksturna jedinica 0

        // Aktiviraj VAO za mrežicu levog zvučnika
        glBindVertexArray(leftMeshVAO);

        // Aktiviraj teksturu
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, meshTexture);

        // Crtanje mrežice kao pravougaonika za levi zvučnik
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Resetuj stanje za levi zvučnik
        glBindVertexArray(0);

        // Renderovanje mrežice desnog zvučnika
        glBindVertexArray(rightMeshVAO);

        // Crtanje mrežice kao pravougaonika za desni zvučnik
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Resetuj stanje za desni zvučnik
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        // Horizontalna linija iznad zvučnika
        glUseProgram(basicShader);
        glUniform3f(uColorLoc, 0.0f, 0.0f, 0.0f);
        glBindVertexArray(lineVAO);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    //Brisanje bafera i sejdera
    glDeleteBuffers(6, VBO);
    glDeleteVertexArrays(6, VAO);
    glDeleteProgram(basicShader);
    //mrezica
    glDeleteBuffers(1, &leftMeshVBO);
    glDeleteVertexArrays(1, &leftMeshVAO);
    glDeleteBuffers(1, &rightMeshVBO);
    glDeleteVertexArrays(1, &rightMeshVAO);
    //mali krugovi u okviru zvucnika
    glDeleteBuffers(2, smallMembraneVBO);
    glDeleteVertexArrays(2, smallMembraneVAO);
    //linija horizontalna
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &lineVAO);

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
