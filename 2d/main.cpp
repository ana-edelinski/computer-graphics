// Autor: Nedeljko Tesanovic
// Opis: V3 Z4

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>   
#include <GLFW/glfw3.h>

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);

int main(void)
{


    if (!glfwInit())
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // kreiranje prozora 500x500
    GLFWwindow* window;
    unsigned int wWidth = 500;
    unsigned int wHeight = 500;
    const char wTitle[] = "[Generic title]";    //naslov prozora
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window); //ako je prozor uspesno kreiran postavlja ga kao aktivan

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }


    //Crvena zvijezda
    //ovde definisem verticles za zvezdu
    float starVertices[] = {
        //poz           r     g    b    a(providnost)
        0.0, 0.5,       0.0, 0.0, 1.0, 1.0, //up
        -0.3, -0.4,     0.0, 0.0, 1.0, 1.0, //down left
        0.4, 0.2,        0.0, 0.0, 1.0, 1.0, //up right
        -0.4, 0.2,      0.0, 0.0, 1.0, 1.0, //up left
        0.3, -0.4,      0.0, 0.0, 1.0, 1.0 //down left
    };

    //Sarena traka
    //kvadrat koji se definise kao dva trougla
    float rectangleVertices[] = {
        -0.1, -0.1,       1.0, 0.0, 0.0, 0.5,
        -0.1, 0.1,       1.0, 0.0, 0.0, 0.5,
        0.1, -0.1,       1.0, 0.0, 0.0, 0.5,
        0.1, 0.1,       1.0, 0.0, 0.0, 0.5,
    };
    
    unsigned int stride = 6 * sizeof(float);    //??

    //vertex array object cuva konfiguraciju vertiksa
    unsigned VAO[2]; //0 - star, 1 - rectangle
    glGenVertexArrays(2, VAO);
    //vertex buffer object je memorija u kojoj se vertiksi cuvaju na grafickoj kartici
    unsigned VBO[2];
    glGenBuffers(2, VBO);

    //povezujem podatke sa vao i vbo
    glBindVertexArray(VAO[0]);  //aktiviram vao za zvezdu
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(starVertices), starVertices, GL_STATIC_DRAW);  //ucitam podatke u vbo
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);  //podesavam kako opengl cita poziciju (saljem poziciju vertiksa na ulaz)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));    //i boje
    glEnableVertexAttribArray(1);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float x = 0;
    float y = 0;
    unsigned int basicShader = createShader("basic.vert", "basic.frag");    //kreiram sejder tj povezujem sejdere u jedan sejderski program
    unsigned int uPosLoc = glGetUniformLocation(basicShader, "uPos"); //Mora biti POSLE pravljenja sejdera, inace ta uniforma ne postoji, kao i sejder

    glPointSize(4);

    //rendering loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();   //proverava dogadjaje

        //reaguje na tastere
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)  //esc zatvara program
            glfwSetWindowShouldClose(window, GL_TRUE);

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)  //strelica gore pomera kvadrat
        {
            if (y <= 0.9)
                y += 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)    //strelica dole pomera kvadrat
        {
            if (y > -0.9)
                y -= 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)   //strelica desno pomera kvadrat
        {
            if (x <= 0.9)
                x += 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)    //strelica levo pomera kvadrat
        {
            if (x >= -0.9)
                x -= 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)   //R vraca na pocetak
        {
            x = 0;
            y = 0;
        }
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)   //1 crta tacke
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)   //2 crta linije
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)   //3 popunjava
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)   //4 ukljuci providnost
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)   //5 iskljuci providnost
        {
            glDisable(GL_BLEND);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(basicShader);
        glUniform2f(uPosLoc, 0, 0); //podesava vrednost uniform promenljive
        glBindVertexArray(VAO[0]);
        glLineWidth(4.0);
        glDrawArrays(GL_LINE_LOOP, 0, sizeof(starVertices) / stride);   //crta zvezdu
        glLineWidth(1.0);

        glBindVertexArray(VAO[1]);
        glUniform2f(uPosLoc, x, y);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(rectangleVertices) / stride); //crta kvadrat

        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);

    }

    glDeleteBuffers(2, VBO);
    glDeleteVertexArrays(2, VAO);
    glDeleteProgram(basicShader);

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
