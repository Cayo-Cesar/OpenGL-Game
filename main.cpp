/*
 Jogo Flappy Bird em 3D utilizando OpenGL e GLUT
 */

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

// Definicao condicional para GL_CLAMP_TO_EDGE
#ifndef GL_CLAMP_TO_EDGE
    #define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include <windows.h>  // Necessario para usar Mmsystem.h no Windows
#include <mmsystem.h> // Biblioteca para reproducao de audio

#define ESC 27  // Tecla ESC
#define PIPE_COUNT 3 // Número de canos
#define PIPE_SPACING 3.0f   // Espacamento entre os canos
#define PIPE_WIDTH 0.4f  // Largura do cano
#define PIPE_HEIGHT 2.5f  // Altura do cano
#define PIPE_DEPTH 0.2f  // Profundidade do cano
#define GRAVITY 0.001f  // Gravidade
#define FLAP_STRENGTH 0.04f  // Força do flap

//Carregar textura
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Caminhos das pastas
const char* soundsPath = "sounds//";

float birdY = 0.0f; // Posicao inicial do passaro
float birdVelocity = 0.0f; // Velocidade inicial do pass�ro
float birdSize = 0.1f; // Tamanho do pass�ro
float birdX = -1.0f; // Posicao horizontal do pass�ro

float pipePositions[PIPE_COUNT]; // Posicoes horizontais dos canos
float pipeGapY[PIPE_COUNT]; // Posicoes verticais do gap entre os canos
float pipeGapSize = 0.4f; // Tamanho do gap entre os canos

bool gameOver = false; // Flag para indicar o fim do jogo

int score = 0; // Pontuacao do jogador

GLuint pipeTexture; // Textura dos canos
GLuint backgroundTexture; // Textura do fundo

// Estrutura para representar as nuvens
struct Cloud {
    float x, y, z;
    float speed;
};

// Array de nuvens
const int NUM_CLOUDS = 8;
Cloud clouds[NUM_CLOUDS];

// Flag para verificar se o passaro passou por um cano
bool passedPipe[PIPE_COUNT] = { false };

// Declaracao das funcoes
void init_glut(const char *window_name, int argc, char** argv);
void display(void);
void reshape(int w, int h);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void resetGame(void);
void draw_parallelepiped(float width, float height, float depth);  
bool check_collision(float px, float py, float psize, float ex, float ey, float ewidth, float eheight, float edepth); 
void initClouds();
GLuint loadTexture(const char* filename);

// Funcao para carregar texturas
GLuint loadTexture(const char* filename) {
	//Nome da textura
    GLuint textureID;
    
    //para armazenar a largura, altura e os canais de cor
    int width, height, nrChannels;
    
    //ponteiro para os dados da imagem
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
	
	//caso n�o seja carregada com sucesso
    if (!data) {
        fprintf(stderr, "Failed to load texture\n");
        return 0;
    }
	
	//para identificar a textura
    glGenTextures(1, &textureID);
    
    //vincular uma textura a um alvo especif�co
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configura a textura
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // para filtrar e suavizar a imagem
    
	
	//liberar mem�ria alocada
    stbi_image_free(data);
    
    //retorna o identificador da textura
    return textureID;
}

int main(int argc, char** argv) {
    // Inicializa a semente do gerador de números aleatorios
    srand(static_cast<unsigned>(time(0)));

    // Inicializa as posições dos canos
    for (int i = 0; i < PIPE_COUNT; ++i) {
        pipePositions[i] = i * PIPE_SPACING + 1.0f;
        pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        passedPipe[i] = false;
    }

    // Inicializa o GLUT
    init_glut("3D Flappy Bird", argc, argv);

    std::string musicPath = std::string(soundsPath) + "music.wav"; // Caminho completo para o arquivo de música
    
    mciSendString(TEXT("close bgm"), NULL, 0, NULL);

    // Reproduz a música de fundo
    mciSendString(TEXT(("open \"" + musicPath + "\" type mpegvideo alias bgm").c_str()), NULL, 0, NULL);
    mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);

    pipeTexture = loadTexture("textures\\canos.png");
    backgroundTexture = loadTexture("textures\\bg.png");

    glutMainLoop();
    return EXIT_SUCCESS;
}

// Funcao para inicializar o GLUT
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
    
    //aplicar texturas nos canos e no background
	pipeTexture = loadTexture("canos.png");
	backgroundTexture = loadTexture("bg.png");
	
	
    glEnable(GL_DEPTH_TEST);
    initClouds();

    // Configuracao da iluminacao
    glEnable(GL_LIGHTING);
    //fonte de luz
    glEnable(GL_LIGHT0);

    // Configuracao da luz ambiente
    GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    // Configuracao da luz difusa
    GLfloat diffuseLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

    // Configuracao da luz especular
    GLfloat specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // Configuracao da posicao da luz
    GLfloat lightPosition[] = {1.0f, 1.0f, 1.0f, 0.0f}; // Luz direcionada
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void draw_background_square() {
	//para nao perder os estados atuais de textura e iluminacao
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
    glDisable(GL_DEPTH_TEST); // Desativa o teste de profundidade para garantir que o quadrado seja desenhado atras de tudo
    
    //para usar a textura "background" aqui
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glEnable(GL_TEXTURE_2D);

    // Configura o material para um brilho mais alto
    GLfloat materialAmbient[] = {2.5f, 2.5f, 2.5f, 2.0f}; 
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    

    // Desenha um quadrado com a textura
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -0.8f, -1.0f); // Vertice inferior esquerdo 
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -0.8f, -1.0f); // Vertice inferior direito 
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, -1.0f); // Vertice superior direito
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -1.0f); // Vrtice superior esquerdo
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

    // Configuracao do material para os canos
    GLfloat materialAmbient[] = {0.1f, 0.5f, 0.0f, 1.0f}; // Verde suave para a luz ambiente
    GLfloat materialDiffuse[] = {0.0f, 0.8f, 0.0f, 1.0f}; // Verde para a luz difusa
    GLfloat materialSpecular[] = {0.0f, 0.3f, 0.0f, 1.0f}; // Verde suave para o brilho especular
    GLfloat materialShininess[] = {3.0f}; // Brilho especular mais alto para suavizar sombras

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

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);

    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

bool check_collision(float px, float py, float psize, float ex, float ey, float ewidth, float eheight, float edepth) {
    // Fator de ajuste para aumentar a caixa de colisao do passaro
    float collision_margin = -0.02f; 
    float pipe_collision_margin = 0.02f; 

    // Bounding Box do Passaro 
    float pxMin = px - (psize + collision_margin) / 2;
    float pxMax = px + (psize + collision_margin) / 2;
    float pyMin = py - (psize + collision_margin) / 2;
    float pyMax = py + (psize + collision_margin) / 2;

    // Bounding Box do Cano 
    float exMin = ex - (ewidth + pipe_collision_margin) / 2;
    float exMax = ex + (ewidth + pipe_collision_margin) / 2;
    float eyMin = ey - (eheight + pipe_collision_margin) / 2;
    float eyMax = ey + (eheight + pipe_collision_margin) / 2;

    // Verifica se os bounding boxes se sobrepoem
    return !(pxMax < exMin || pxMin > exMax || pyMax < eyMin || pyMin > eyMax);
}

void drawBird() {
    // Material do passaro
    GLfloat birdBodyAmbient[] = {1.0f, 1.0f, 0.0f, 1.0f}; 
    GLfloat birdBodyDiffuse[] = {1.0f, 1.0f, 0.0f, 1.0f}; 
    GLfloat birdBodySpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; 
    GLfloat birdBodyShininess[] = {50.0f}; 

    // Aplicar as propriedades do material do corpo do p�ssaro
    glMaterialfv(GL_FRONT, GL_AMBIENT, birdBodyAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, birdBodyDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, birdBodySpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, birdBodyShininess);

    // Desenha o corpo do passaro
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f); 
    glRotatef(0.0f, 0.0f, 1.0f, 0.0f); //Roda a esfera para alinhar com a orientacao do passaro
    glutSolidSphere(0.1f, 20, 20); 
    glPopMatrix();

    // Material das asas
    GLfloat birdWingAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f}; 
    GLfloat birdWingDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f}; 
    GLfloat birdWingSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat birdWingShininess[] = {50.0f}; 

    // Aplicar as propriedades do material das asas
    glMaterialfv(GL_FRONT, GL_AMBIENT, birdWingAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, birdWingDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, birdWingSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, birdWingShininess);

    // Desenha a asa esquerda
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.11f); 
    glRotatef(0.0f, 0.0f, 1.0f, 0.0f); 
    glScalef(0.02f, 0.1f, 0.1f); 
    glutSolidCube(1.0f); 
    glPopMatrix();

    // Desenha a asa direita
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.11f); 
    glRotatef(0.0f, 0.0f, 1.0f, 0.0f); 
    glScalef(0.02f, 0.1f, 0.1f); 
    glutSolidCube(1.0f); 
    glPopMatrix();

    // Material do bico e da cauda
    GLfloat birdBeakTailAmbient[] = {1.0f, 0.5f, 0.0f, 1.0f}; 
    GLfloat birdBeakTailDiffuse[] = {1.0f, 0.5f, 0.0f, 1.0f}; 
    GLfloat birdBeakTailSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; 
    GLfloat birdBeakTailShininess[] = {50.0f}; 

    // Aplicar as propriedades do material do bico e da cauda
    glMaterialfv(GL_FRONT, GL_AMBIENT, birdBeakTailAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, birdBeakTailDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, birdBeakTailSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, birdBeakTailShininess);

    // Desenha o bico
    glPushMatrix();
    glTranslatef(0.1f, 0.0f, 0.0f); 
    glScalef(0.05f, 0.02f, 0.1f); 
    glutSolidCube(1.0f); 
    glPopMatrix();

    // Desenha a cauda
    glPushMatrix();
    glTranslatef(-0.15f, 0.0f, 0.0f); 
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); 
    glutSolidCone(0.05f, 0.1f, 20, 20); 
    glPopMatrix();

    // Material dos olhos
    GLfloat birdEyeAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f}; 
    GLfloat birdEyeDiffuse[] = {0.0f, 0.0f, 0.0f, 1.0f}; 
    GLfloat birdEyeSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; 
    GLfloat birdEyeShininess[] = {50.0f}; 

    // Aplicar as propriedades do material dos olhos
    glMaterialfv(GL_FRONT, GL_AMBIENT, birdEyeAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, birdEyeDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, birdEyeSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, birdEyeShininess);

    // Desenha o olho esquerdo
    glPushMatrix();
    glTranslatef(0.08f, 0.05f, 0.02f); 
    glutSolidSphere(0.015f, 20, 20); 
    glPopMatrix();

    // Desenha o olho direito
    glPushMatrix();
    glTranslatef(0.08f, 0.05f, -0.02f); 
    glutSolidSphere(0.015f, 20, 20); 
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
    glEnable(GL_LIGHTING);  // Habilita a iluminacao

    // Material para as nuvens
    GLfloat ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat shininess = 50.0f;  

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    // Desenha as nuvens com sombreamento
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        glPushMatrix();
        glTranslatef(clouds[i].x, clouds[i].y, clouds[i].z);
        drawCloud(0.0f, 0.0f, 0.0f);
        glPopMatrix();
    }
}

// Atualiza a posicao das nuvens
void updateClouds() {
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        clouds[i].x -= clouds[i].speed;
        if (clouds[i].x < -5.0f) {
            clouds[i].x = 5.0f;
            clouds[i].y = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        }
    }
}

// Nuvens iniciais
void initClouds() {
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        clouds[i].x = ((rand() % 100) / 100.0f) * 12.0f - 6.0f; 
        clouds[i].y = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        clouds[i].z = -2.0f - ((rand() % 100) / 100.0f);
        clouds[i].speed = 0.001f + ((rand() % 100) / 10000.0f);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	
	//desenhar o background
    draw_background_square();
    glTranslatef(0.0f, 0.0f, -5.0f); // Posiciona a camera um pouco mais longe

    drawClouds();  // Desenha as nuvens primeiro, para que fiquem no fundo
    updateClouds();

    glTranslatef(0.0f, 0.0f, 2.0f); // Avanca um pouco para os elementos do jogo

    if (!gameOver) {
        glPushMatrix();
        glTranslatef(birdX, birdY, 0.0f);
        drawBird(); // Desenha o pássaro 
        glPopMatrix();

        // Desenha os canos 
        for (int i = 0; i < PIPE_COUNT; ++i) {
            //Superior
            glPushMatrix();
            glTranslatef(pipePositions[i], pipeGapY[i] + pipeGapSize + PIPE_HEIGHT / 2, 0.0f);
            draw_parallelepiped(PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH, false);
            glPopMatrix();
        
            //Inferior
            glPushMatrix();
            glTranslatef(pipePositions[i], pipeGapY[i] - pipeGapSize - PIPE_HEIGHT / 2, 0.0f);
            draw_parallelepiped(PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH, true);
            glPopMatrix();
        }

        // Desenha a pontuacao
        glColor3f(0.0f, 0.0f, 0.0f);
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0); // Coordenadas para a pontuacao
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glRasterPos2f(0.8f, 0.9f);  // Posicao no canto superior direito
        drawText(0.0f, 0.0f, scoreText);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    } else {
        // Mensagem combinada de Game Over e reinicio
        glColor3f(0.0f, 0.0f, 0.0f);
        char gameOverText[100];
        
        sprintf(gameOverText, "Game Over! Pressione 'R' para reiniciar!");

        // Configurcao de projecao para centralizar o texto
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0); 
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

        glRasterPos2f(centerX - textWidth / 2 / 800.0f, centerY);  // Ajuste de acordo com a resolucao

        drawText(0.0f, 0.0f, gameOverText);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    glutSwapBuffers();
}

// Funcao para redimensionar a janela
void reshape(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 100.0); 
    glMatrixMode(GL_MODELVIEW);
}

// Funcao para atualizar a cena
void timer(int value) {
    if (!gameOver) {
        birdVelocity -= GRAVITY;
        birdY += birdVelocity;

        // Atualiza a posicao horizontal do passaro
        for (int i = 0; i < PIPE_COUNT; ++i) {
            pipePositions[i] -= 0.025f; 

            // Verifica se o cano saiu da tela
            if (pipePositions[i] < -3.0f) { // Atualizado para novo PIPE_SPACING
                pipePositions[i] += PIPE_COUNT * PIPE_SPACING;
                pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
                passedPipe[i] = false; 
            }

            // Verifica se o passaro passou pelo cano
            for (int j = 0; j < PIPE_COUNT; ++j) {
                // Verifica se o passaro esta na mesma posicao horizontal que o cano
                if (pipePositions[j] < birdX + birdSize / 2 && pipePositions[j] > birdX - birdSize / 2) {
                    // Verifica colisao com a parte superior do cano
                    if (check_collision(birdX, birdY, birdSize, pipePositions[j], pipeGapY[j] + pipeGapSize + PIPE_HEIGHT / 2, PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH)) {
                    	mciSendString(TEXT("stop bgm"), NULL, 0, NULL);
	  	  	            mciSendString(TEXT("close bgm"), NULL, 0, NULL);
                    	PlaySound(TEXT("sounds\\gameover.wav"), NULL, SND_ASYNC);
                        gameOver = true;
                        break;
                    }
                    // Verifica colisao com a parte inferior do cano
                    if (check_collision(birdX, birdY, birdSize, pipePositions[j], pipeGapY[j] - pipeGapSize - PIPE_HEIGHT / 2, PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH)) {
                    	mciSendString(TEXT("stop bgm"), NULL, 0, NULL);
 	 	 	 	 	 	 mciSendString(TEXT("close bgm"), NULL, 0, NULL);
                    	 PlaySound(TEXT("sounds\\gameover.wav"), NULL, SND_ASYNC);
                        gameOver = true;
                        break;
                    }
                }
            }

            // Incrementa o score se o passaro passou pelo cano
            if (!passedPipe[i] && pipePositions[i] < birdX - birdSize / 2) {
                passedPipe[i] = true;
                score++;
                PlaySound(TEXT("sounds\\score.wav"), NULL, SND_ASYNC);
                
            }
        }

        // Verifica se o passaro saiu da tela
        if (birdY - birdSize / 2 < -1.5f || birdY + birdSize / 2 > 1.5f) {
        	mciSendString(TEXT("stop bgm"), NULL, 0, NULL);
            mciSendString(TEXT("close bgm"), NULL, 0, NULL);
     	    PlaySound(TEXT("sounds\\gameover.wav"), NULL, SND_ASYNC);
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
                std::string musicPath = std::string(soundsPath) + "music.wav";  // Caminho completo para o arquivo de música
                mciSendString(TEXT("close bgm"), NULL, 0, NULL);

                // Abrir música e reproduzir em loop
                mciSendString(TEXT(("open \"" + musicPath + "\" type mpegvideo alias bgm").c_str()), NULL, 0, NULL);
                mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
            }
            break;
    }
}

// Funcao para reiniciar o jogo
void resetGame(void) {
	
    birdY = 0.0f;
    birdVelocity = 0.0f;
    birdX = -1.0f;
    score = 0; // Reseta a pontuacao ao reiniciar o jogo

    for (int i = 0; i < PIPE_COUNT; ++i) {
        pipePositions[i] = i * PIPE_SPACING + 1.0f;
        pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        passedPipe[i] = false; 
    }
    gameOver = false;
}