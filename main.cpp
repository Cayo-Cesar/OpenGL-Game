/*
 Jogo Flappy Bird em 3D utilizando OpenGL e GLUT
 */

// Importação das bibliotecas necessárias
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    
#endif

// Defini��o condicional para GL_CLAMP_TO_EDGE
#ifndef GL_CLAMP_TO_EDGE
    #define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include <windows.h>  // Necess�rio para usar Mmsystem.h no Windows
#include <mmsystem.h> // Biblioteca para reprodu��o de �udio


#define ESC 27  // Tecla ESC
#define PIPE_COUNT 3 // Número de canos
#define PIPE_SPACING 3.0f   // Espaçamento entre os canos
#define PIPE_WIDTH 0.4f  // Largura do cano
#define PIPE_HEIGHT 2.5f  // Altura do cano
#define PIPE_DEPTH 0.2f  // Profundidade do cano
#define GRAVITY 0.001f  // Gravidade
#define FLAP_STRENGTH 0.04f  // Força do flap

//carregar textura
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float birdY = 0.0f; // Posição inicial do pássaro
float birdVelocity = 0.0f; // Velocidade inicial do pássaro
float birdSize = 0.1f; // Tamanho do pássaro
float birdX = -1.0f; // Posição horizontal do pássaro

float pipePositions[PIPE_COUNT]; // Posições horizontais dos canos
float pipeGapY[PIPE_COUNT]; // Posições verticais do gap entre os canos
float pipeGapSize = 0.4f; // Tamanho do gap entre os canos

bool gameOver = false; // Flag para indicar o fim do jogo

int score = 0; // Pontuação do jogador

GLuint pipeTexture;
GLuint birdTexture;
GLuint backgroundTexture;

struct Cloud {
    float x, y, z;
    float speed;
};

const int NUM_CLOUDS = 8;
Cloud clouds[NUM_CLOUDS];

// Flag para verificar se o pássaro passou por um cano
bool passedPipe[PIPE_COUNT] = { false };

// Declaração das funções
void init_glut(const char *window_name, int argc, char** argv);
void display(void);
void reshape(int w, int h);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void resetGame(void);
void draw_parallelepiped(float width, float height, float depth);  // Function added
bool check_collision(float px, float py, float psize, float ex, float ey, float ewidth, float eheight, float edepth); // Function added
void initClouds();

GLuint loadTexture(const char* filename) {
    GLuint textureID;
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (!data) {
        fprintf(stderr, "Failed to load texture\n");
        return 0;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configura a textura
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Usando GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Usando GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}



// Função principal
int main(int argc, char** argv) {
    // Inicializa a semente do gerador de n�meros aleat�rios
    srand(static_cast<unsigned>(time(0)));

    // Inicializa as posi��es dos canos
    for (int i = 0; i < PIPE_COUNT; ++i) {
        pipePositions[i] = i * PIPE_SPACING + 1.0f;
        pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        passedPipe[i] = false;
    }

    // Inicializa o GLUT
    init_glut("3D Flappy Bird", argc, argv);

    // Reproduz a m�sica de fundo
    PlaySound(TEXT("music.wav"), NULL, SND_LOOP | SND_ASYNC);

    // Carrega a textura dos canos
    pipeTexture = loadTexture("canos.png");
    backgroundTexture = loadTexture("bg.png");
    // birdTexture = loadTexture("flappy_bird.png");

    glutMainLoop();

    return EXIT_SUCCESS;
}

// Função para inicializar o GLUT
void init_glut(const char *window_name, int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(window_name);
 
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, timer, 0);
    
	pipeTexture = loadTexture("canos.png");
	backgroundTexture = loadTexture("bg.png");
	//birdTexture = loadTexture("flappy_bird.png");

     // Carregar a textura do fundo
   
	
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Lighter blue background

    initClouds();

    // Configura��o da ilumina��o
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Configura��o da luz ambiente
    GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    // Configura��o da luz difusa
    GLfloat diffuseLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

    // Configura��o da luz especular
    GLfloat specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // Configura��o da posi��o da luz
    GLfloat lightPosition[] = {1.0f, 1.0f, 1.0f, 0.0f}; // Luz direcionada
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // Ativa a suaviza��o de sombras
    //glShadeModel(GL_SMOOTH);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
}

void draw_background_square() {
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
    glDisable(GL_DEPTH_TEST); // Desativa o teste de profundidade para garantir que o quadrado seja desenhado atrás de tudo
    
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glEnable(GL_TEXTURE_2D);

    // Configura o material para um brilho mais alto
    GLfloat materialAmbient[] = {1.5f, 1.5f, 1.5f, 1.0f}; 
    GLfloat materialDiffuse[] = {1.5f, 1.5f, 1.5f, 1.0f}; 
    GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Intensidade máxima do brilho especular
    GLfloat materialShininess[] = {3.0f}; // Aumenta o brilho especular para o máximo

    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

    // Desenha um quadrado com a textura
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -0.8f, -1.0f); // Vértice inferior esquerdo ajustado
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -0.8f, -1.0f); // Vértice inferior direito ajustado
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, -1.0f); // Vértice superior direito
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -1.0f); // Vértice superior esquerdo
    glEnd();

    glEnable(GL_DEPTH_TEST); // Reativa o teste de profundidade após desenhar o fundo
    
    glPopAttrib();
}

void draw_parallelepiped(float width, float height, float depth, bool invertTexture = false) {
    // Salva o estado atual
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);

    // Ativa a textura
    glBindTexture(GL_TEXTURE_2D, pipeTexture);
    glEnable(GL_TEXTURE_2D);

    // Configura��o do material para os canos
    GLfloat materialAmbient[] = {0.1f, 0.5f, 0.0f, 1.0f}; // Verde suave para a luz ambiente
    GLfloat materialDiffuse[] = {0.0f, 0.8f, 0.0f, 1.0f}; // Verde para a luz difusa
    GLfloat materialSpecular[] = {0.0f, 0.3f, 0.0f, 1.0f}; // Verde suave para o brilho especular
    GLfloat materialShininess[] = {20.0f}; // Brilho especular mais alto para suavizar sombras

    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

    glBegin(GL_QUADS);

    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(-width / 2, -height / 2, depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(width / 2, -height / 2, depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(width / 2, height / 2, depth / 2);
    glTexCoord2f(0.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(-width / 2, height / 2, depth / 2);

    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(-width / 2, -height / 2, -depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(width / 2, -height / 2, -depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(width / 2, height / 2, -depth / 2);
    glTexCoord2f(0.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(-width / 2, height / 2, -depth / 2);

    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(-width / 2, -height / 2, -depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(-width / 2, -height / 2, depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(-width / 2, height / 2, depth / 2);
    glTexCoord2f(0.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(-width / 2, height / 2, -depth / 2);

    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(width / 2, -height / 2, -depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(width / 2, -height / 2, depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(width / 2, height / 2, depth / 2);
    glTexCoord2f(0.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(width / 2, height / 2, -depth / 2);

    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(-width / 2, height / 2, -depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(width / 2, height / 2, -depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(width / 2, height / 2, depth / 2);
    glTexCoord2f(0.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(-width / 2, height / 2, depth / 2);

    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(-width / 2, -height / 2, -depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 1.0f : 0.0f);
    glVertex3f(width / 2, -height / 2, -depth / 2);
    glTexCoord2f(1.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(width / 2, -height / 2, depth / 2);
    glTexCoord2f(0.0f, invertTexture ? 0.0f : 1.0f);
    glVertex3f(-width / 2, -height / 2, depth / 2);

    glEnd();

    glPopAttrib();
}



//antigo passaro

// void drawTexturedCube(float size, GLuint textureID) {
//     glBindTexture(GL_TEXTURE_2D, textureID);
//     glEnable(GL_TEXTURE_2D);

//     glBegin(GL_QUADS);

//     // Face frontal
//     glTexCoord2f(0.0f, 1.0f); 
//     glVertex3f(-size / 2, -size / 2, size / 2);
//     glTexCoord2f(1.0f, 1.0f); 
//     glVertex3f(size / 2, -size / 2, size / 2);
//     glTexCoord2f(1.0f, 0.0f);
//     glVertex3f(size / 2, size / 2, size / 2);
//     glTexCoord2f(0.0f, 0.0f);
//     glVertex3f(-size / 2, size / 2, size / 2);

//     // Face traseira
//     glTexCoord2f(0.0f, 1.0f); 
//     glVertex3f(-size / 2, -size / 2, -size / 2);
//     glTexCoord2f(1.0f, 1.0f); 
//     glVertex3f(size / 2, -size / 2, -size / 2);
//     glTexCoord2f(1.0f, 0.0f);
//     glVertex3f(size / 2, size / 2, -size / 2);
//     glTexCoord2f(0.0f, 0.0f);
//     glVertex3f(-size / 2, size / 2, -size / 2);

//     // Face esquerda
//     glTexCoord2f(0.0f, 1.0f); 
//     glVertex3f(-size / 2, -size / 2, -size / 2);
//     glTexCoord2f(1.0f, 1.0f); 
//     glVertex3f(-size / 2, -size / 2, size / 2);
//     glTexCoord2f(1.0f, 0.0f);
//     glVertex3f(-size / 2, size / 2, size / 2);
//     glTexCoord2f(0.0f, 0.0f);
//     glVertex3f(-size / 2, size / 2, -size / 2);

//     // Face direita
//     glTexCoord2f(0.0f, 1.0f);
//     glVertex3f(size / 2, -size / 2, -size / 2);
//     glTexCoord2f(1.0f, 1.0f); 
//     glVertex3f(size / 2, -size / 2, size / 2);
//     glTexCoord2f(1.0f, 0.0f);
//     glVertex3f(size / 2, size / 2, size / 2);
//     glTexCoord2f(0.0f, 0.0f);
//     glVertex3f(size / 2, size / 2, -size / 2);

//     // Face superior
//     glTexCoord2f(0.0f, 1.0f); 
//     glVertex3f(-size / 2, size / 2, -size / 2);
//     glTexCoord2f(1.0f, 1.0f); 
//     glVertex3f(size / 2, size / 2, -size / 2);
//     glTexCoord2f(1.0f, 0.0f);
//     glVertex3f(size / 2, size / 2, size / 2);
//     glTexCoord2f(0.0f, 0.0f);
//     glVertex3f(-size / 2, size / 2, size / 2);

//     // Face inferior
//     glTexCoord2f(0.0f, 1.0f); 
//     glVertex3f(-size / 2, -size / 2, -size / 2);
//     glTexCoord2f(1.0f, 1.0f); 
//     glVertex3f(size / 2, -size / 2, -size / 2);
//     glTexCoord2f(1.0f, 0.0f);
//     glVertex3f(size / 2, -size / 2, size / 2);
//     glTexCoord2f(0.0f, 0.0f);
//     glVertex3f(-size / 2, -size / 2, size / 2);

//     glEnd();

//     glDisable(GL_TEXTURE_2D);
// }

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);

    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

bool check_collision(float px, float py, float psize, float ex, float ey, float ewidth, float eheight, float edepth) {
    // Fator de ajuste para aumentar a caixa de colisão do pássaro
    float collision_margin = 0.02f; // Ajuste esse valor conforme necessário

    // Bounding Box do Passaro (um cubo) com margem extra
    float pxMin = px - (psize + collision_margin) / 2;
    float pxMax = px + (psize + collision_margin) / 2;
    float pyMin = py - (psize + collision_margin) / 2;
    float pyMax = py + (psize + collision_margin) / 2;

    // Bounding Box do Cano (um paralelepípedo)
    float exMin = ex - ewidth / 2;
    float exMax = ex + ewidth / 2;
    float eyMin = ey - eheight / 2;
    float eyMax = ey + eheight / 2;

    // Verifica se os bounding boxes se sobrepõem
    return !(pxMax < exMin || pxMin > exMax || pyMax < eyMin || pyMin > eyMax);
}

void drawBird() {
    // Configuração do material do corpo do pássaro
    GLfloat birdBodyAmbient[] = {1.0f, 1.0f, 0.0f, 1.0f}; // Cor ambiente (amarelo)
    GLfloat birdBodyDiffuse[] = {1.0f, 1.0f, 0.0f, 1.0f}; // Cor difusa (amarelo)
    GLfloat birdBodySpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Cor especular (branco)
    GLfloat birdBodyShininess[] = {50.0f}; // Brilho

    // Aplicar as propriedades do material do corpo do pássaro
    glMaterialfv(GL_FRONT, GL_AMBIENT, birdBodyAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, birdBodyDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, birdBodySpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, birdBodyShininess);

    // Desenha o corpo do pássaro
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f); // Posição do corpo
    glRotatef(0.0f, 0.0f, 1.0f, 0.0f); // Gira a esfera 0 graus no eixo Y para posicionar o pássaro de frente para a direita
    glutSolidSphere(0.1f, 20, 20); // Desenha a esfera com raio 0.1
    glPopMatrix();

    // Configuração do material das asas
    GLfloat birdWingAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Cor ambiente (branco)
    GLfloat birdWingDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Cor difusa (branco)
    GLfloat birdWingSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Cor especular (branco)
    GLfloat birdWingShininess[] = {50.0f}; // Brilho

    // Aplicar as propriedades do material das asas
    glMaterialfv(GL_FRONT, GL_AMBIENT, birdWingAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, birdWingDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, birdWingSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, birdWingShininess);

    // Desenha a asa esquerda
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.11f); // Posição da asa esquerda (grudada ao corpo)
    glRotatef(0.0f, 0.0f, 1.0f, 0.0f); // Gira a asa para alinhar com a nova orientação da esfera
    glScalef(0.02f, 0.1f, 0.1f); // Escala para criar a asa (fininha e longa)
    glutSolidCube(1.0f); // Usa um cubo sólido para representar a asa
    glPopMatrix();

    // Desenha a asa direita
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.11f); // Posição da asa direita (grudada ao corpo)
    glRotatef(0.0f, 0.0f, 1.0f, 0.0f); // Gira a asa para alinhar com a nova orientação da esfera
    glScalef(0.02f, 0.1f, 0.1f); // Escala para criar a asa (fininha e longa)
    glutSolidCube(1.0f); // Usa um cubo sólido para representar a asa
    glPopMatrix();

    // Configuração do material do bico e da cauda
    GLfloat birdBeakTailAmbient[] = {1.0f, 0.5f, 0.0f, 1.0f}; // Cor ambiente (laranja)
    GLfloat birdBeakTailDiffuse[] = {1.0f, 0.5f, 0.0f, 1.0f}; // Cor difusa (laranja)
    GLfloat birdBeakTailSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Cor especular (branco)
    GLfloat birdBeakTailShininess[] = {50.0f}; // Brilho

    // Aplicar as propriedades do material do bico e da cauda
    glMaterialfv(GL_FRONT, GL_AMBIENT, birdBeakTailAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, birdBeakTailDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, birdBeakTailSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, birdBeakTailShininess);

    // Desenha o bico
    glPushMatrix();
    glTranslatef(0.1f, 0.0f, 0.0f); // Posição do bico na frente da esfera
    glScalef(0.05f, 0.02f, 0.1f); // Escala para criar o bico (pequeno e fino)
    glutSolidCube(1.0f); // Usa um cubo sólido para representar o bico
    glPopMatrix();

    // Desenha a cauda
    glPushMatrix();
    glTranslatef(-0.15f, 0.0f, 0.0f); // Posição da cauda na parte traseira da esfera
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Gira o cone para alinhar com a orientação do corpo
    glutSolidCone(0.05f, 0.1f, 20, 20); // Desenha um cone com base 0.05 e altura 0.1
    glPopMatrix();

    // Configuração do material dos olhos
    GLfloat birdEyeAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f}; // Cor ambiente (preto)
    GLfloat birdEyeDiffuse[] = {0.0f, 0.0f, 0.0f, 1.0f}; // Cor difusa (preto)
    GLfloat birdEyeSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Cor especular (branco)
    GLfloat birdEyeShininess[] = {50.0f}; // Brilho

    // Aplicar as propriedades do material dos olhos
    glMaterialfv(GL_FRONT, GL_AMBIENT, birdEyeAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, birdEyeDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, birdEyeSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, birdEyeShininess);

    // Desenha o olho esquerdo
    glPushMatrix();
    glTranslatef(0.08f, 0.05f, 0.02f); // Posição do olho esquerdo
    glutSolidSphere(0.015f, 20, 20); // Aumenta o raio da esfera para representar o olho
    glPopMatrix();

    // Desenha o olho direito
    glPushMatrix();
    glTranslatef(0.08f, 0.05f, -0.02f); // Posição do olho direito
    glutSolidSphere(0.015f, 20, 20); // Aumenta o raio da esfera para representar o olho
    glPopMatrix();
}

void drawCloud(float x, float y, float z) {
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f); // Cor branca com transparência

    glPushMatrix();
    glTranslatef(x, y, z);

    // Desenha várias esferas para criar uma nuvem
    glutSolidSphere(0.2f, 20, 20);

    glTranslatef(-0.2f, 0.1f, 0.1f);
    glutSolidSphere(0.15f, 20, 20);

    glTranslatef(0.4f, -0.2f, -0.1f);
    glutSolidSphere(0.15f, 20, 20);

    glTranslatef(-0.1f, 0.15f, 0.2f);
    glutSolidSphere(0.1f, 20, 20);

    glTranslatef(0.1f, 0.05f, -0.3f);
    glutSolidSphere(0.12f, 20, 20);

    glPopMatrix();
}

void drawClouds() {
    // Desabilita a iluminação para as nuvens e contornos
    glDisable(GL_LIGHTING);

    // Define a cor para o contorno (cinza escuro)
    glColor3f(0.4f, 0.4f, 0.4f);

    // Desenha o contorno
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        glPushMatrix();
        glTranslatef(clouds[i].x, clouds[i].y, clouds[i].z);
        glScalef(1.1f, 1.1f, 1.1f); // Aumenta ligeiramente para criar o efeito de contorno
        drawCloud(0.0f, 0.0f, 0.0f);
        glPopMatrix();
    }

    // Define a cor branca para as nuvens reais
    glColor3f(1.0f, 1.0f, 1.0f);

    // Desenha as nuvens reais
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        drawCloud(clouds[i].x, clouds[i].y, clouds[i].z);
    }

    // Reabilita a iluminação para os outros objetos
    glEnable(GL_LIGHTING);
}

void updateClouds() {
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        clouds[i].x -= clouds[i].speed;
        if (clouds[i].x < -5.0f) {
            clouds[i].x = 5.0f;
            clouds[i].y = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        }
    }
}

void initClouds() {
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        clouds[i].x = ((rand() % 100) / 100.0f) * 6.0f - 3.0f;
        clouds[i].y = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        clouds[i].z = -2.0f - ((rand() % 100) / 100.0f);
        clouds[i].speed = 0.001f + ((rand() % 100) / 10000.0f);
    }
}

// void drawTree(float trunkHeight, float trunkRadius, float foliageHeight, float foliageRadius) {
//     // Desenha o tronco da árvore
//     glPushMatrix();
//     glColor3f(0.55f, 0.27f, 0.07f); // Cor marrom para o tronco
//     GLUquadric* trunk = gluNewQuadric();
//     gluCylinder(trunk, trunkRadius, trunkRadius, trunkHeight, 20, 20);
//     glPopMatrix();

//     // Desenha a copa da árvore
//     glPushMatrix();
//     glColor3f(0.0f, 0.5f, 0.0f); // Cor verde para a copa
//     glTranslatef(0.0f, 0.0f, trunkHeight); // Move a copa para cima do tronco
//     glutSolidCone(foliageRadius, foliageHeight, 20, 20);
//     glPopMatrix();
// }

// void drawTrees() {
//     glDisable(GL_LIGHTING); // Desabilita a iluminação para as árvores

//     const int numTrees = 10; // Número de árvores
//     for (int i = 0; i < numTrees; ++i) {
//         float x = (rand() % 200 - 100) / 10.0f; // Posição aleatória no eixo X
//         float z = (rand() % 100 - 150) / 10.0f; // Posição aleatória no eixo Z

//         // Corrige a altura para que as árvores estejam no chão
//         float trunkHeight = 0.3f;
//         float trunkRadius = 0.05f;
//         float foliageHeight = 0.4f;
//         float foliageRadius = 0.2f;

//         glPushMatrix();
//         glTranslatef(x, trunkHeight / 2, z); // Move a árvore para a posição correta no fundo
//         drawTree(trunkHeight, trunkRadius, foliageHeight, foliageRadius); // Desenha a árvore
//         glPopMatrix();
//     }

//     glEnable(GL_LIGHTING); // Reabilita a iluminação para os outros objetos
// }

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Desenha o fundo
    draw_background_square();

    // Configuração da câmera
    glTranslatef(0.0f, 0.0f, -5.0f); // Posiciona a câmera um pouco mais longe

    // Desenha as árvores no fundo
    //drawTrees();

    // Atualiza e desenha as nuvens
    drawClouds();  // Desenha as nuvens primeiro, para que fiquem no fundo
    updateClouds();

    // Avança um pouco para os elementos do jogo
    glTranslatef(0.0f, 0.0f, 2.0f);

    if (!gameOver) {
        // Desenha o pássaro
        glPushMatrix();
        glTranslatef(birdX, birdY, 0.0f);
        drawBird(); // Desenha o pássaro usando primitivas do OpenGL
        glPopMatrix();

        // Desenha os canos 
        for (int i = 0; i < PIPE_COUNT; ++i) {
            // Cano superior
            glPushMatrix();
            glTranslatef(pipePositions[i], pipeGapY[i] + pipeGapSize + PIPE_HEIGHT / 2, 0.0f);
            draw_parallelepiped(PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH, false);
            glPopMatrix();
        
            // Cano inferior
            glPushMatrix();
            glTranslatef(pipePositions[i], pipeGapY[i] - pipeGapSize - PIPE_HEIGHT / 2, 0.0f);
            draw_parallelepiped(PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH, true);
            glPopMatrix();
        }

        // Desenha a pontuação
        glColor3f(0.0f, 0.0f, 0.0f);
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0); // Coordenadas para a pontuação
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glRasterPos2f(0.8f, 0.9f);  // Posição no canto superior direito
        drawText(0.0f, 0.0f, scoreText);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    } else {
        // Mensagem combinada de Game Over e reinício
        glColor3f(0.0f, 0.0f, 0.0f);
        char gameOverText[100];
        sprintf(gameOverText, "Game Over! Pressione 'R' para reiniciar!");

        // Configuração de projeção para centralizar o texto
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0); // Coordenadas de projeção
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Centraliza o texto
        float textWidth = 0.0f;
        const char* tempText = gameOverText;
        while (*tempText) {
            textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *tempText);
            tempText++;
        }

        float centerX = 0.5f; // Centraliza horizontalmente
        float centerY = 0.5f; // Centraliza verticalmente

        glRasterPos2f(centerX - textWidth / 2 / 800.0f, centerY);  // Ajuste de acordo com a resolução

        drawText(0.0f, 0.0f, gameOverText);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    glutSwapBuffers();
}


// Função para redimensionar a janela
void reshape(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 100.0); // Increased view distance
    glMatrixMode(GL_MODELVIEW);
}

// Função para atualizar a cena
void timer(int value) {
    if (!gameOver) {
        birdVelocity -= GRAVITY;
        birdY += birdVelocity;

        // Atualiza a posi��o horizontal do p�ssaro
        for (int i = 0; i < PIPE_COUNT; ++i) {
            pipePositions[i] -= 0.025f; // Velocidade do cano reduzida

            // Verifica se o cano saiu da tela
            if (pipePositions[i] < -3.0f) { // Atualizado para novo PIPE_SPACING
                pipePositions[i] += PIPE_COUNT * PIPE_SPACING;
                pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
                passedPipe[i] = false; // Reset flag
            }

            // Verifica se o p�ssaro passou pelo cano
            for (int j = 0; j < PIPE_COUNT; ++j) {
                // Verifica se o p�ssaro est� na mesma posi��o horizontal que o cano
                if (pipePositions[j] < birdX + birdSize / 2 && pipePositions[j] > birdX - birdSize / 2) {
                    // Verifica colis�o com a parte superior do cano
                    if (check_collision(birdX, birdY, birdSize, pipePositions[j], pipeGapY[j] + pipeGapSize + PIPE_HEIGHT / 2, PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH)) {
                        gameOver = true;
                        break;
                    }
                    // Verifica colis�o com a parte inferior do cano
                    if (check_collision(birdX, birdY, birdSize, pipePositions[j], pipeGapY[j] - pipeGapSize - PIPE_HEIGHT / 2, PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH)) {
                        gameOver = true;
                        break;
                    }
                }
            }

            // Incrementa o score se o p�ssaro passou pelo cano
            if (!passedPipe[i] && pipePositions[i] < birdX - birdSize / 2) {
                passedPipe[i] = true;
                score++;
            }
        }

        // Verifica se o p�ssaro saiu da tela
        if (birdY - birdSize / 2 < -1.5f || birdY + birdSize / 2 > 1.5f) {
            gameOver = true;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}


// Função para tratar eventos do teclado
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case ' ':
            if (!gameOver) {
                birdVelocity = FLAP_STRENGTH;
            }
            break;
        case ESC:
            exit(EXIT_SUCCESS);
            break;
        case 'r': // Reiniciar o jogo
            if (gameOver) {
                resetGame();
            }
            break;
    }
}

// Função para reiniciar o jogo
void resetGame(void) {
    birdY = 0.0f;
    birdVelocity = 0.0f;
    birdX = -1.0f;
    score = 0; // Reseta a pontuação ao reiniciar o jogo

    for (int i = 0; i < PIPE_COUNT; ++i) {
        pipePositions[i] = i * PIPE_SPACING + 1.0f;
        pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        passedPipe[i] = false; // Reset flag
    }

    gameOver = false;
}