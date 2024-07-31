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

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Constantes
#define WIDTH 800
#define HEIGHT 600
#define PIPE_GAP 150
#define PIPE_WIDTH 80
#define GRAVITY 0.05f
#define JUMP_FORCE -1.5f

// Variáveis globais
float birdY = HEIGHT / 2.0f;
float birdVelocity = 0.0f;
bool gameRunning = true;
int score = 0;

GLuint birdTexture;
GLuint pipeTexture;

struct Pipe {
    float x;
    float height;
};

Pipe pipes[3];

// Função para desenhar texto
void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

// Função para carregar a textura
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

// Função de reinício do jogo
void restartGame() {
    birdY = HEIGHT / 2.0f;
    birdVelocity = 0.0f;
    gameRunning = true;
    score = 0;

    for (int i = 0; i < 3; i++) {
        pipes[i].x = WIDTH + i * (PIPE_WIDTH + 200);
        pipes[i].height = rand() % (HEIGHT - PIPE_GAP);
    }
}

// Função de inicialização
void init() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Fundo azul claro
    glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
    srand(time(0));

    birdTexture = loadTexture("flappy_bird.png"); // Certifique-se de que o caminho está correto
    pipeTexture = loadTexture("canos.png"); // Certifique-se de que o caminho está correto
    restartGame(); // Inicializa o jogo
}

// Função de display
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (gameRunning) {
        // Desenha o pássaro com a textura
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, birdTexture);

        glColor3f(1.0f, 1.0f, 1.0f); // Branco, para mostrar a textura como está

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(100, birdY - 10);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(120, birdY - 10);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(120, birdY + 10);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(100, birdY + 10);
        glEnd();

        // Desenha os canos com a textura
        glBindTexture(GL_TEXTURE_2D, pipeTexture);

        glBegin(GL_QUADS);
        for (int i = 0; i < 3; i++) {
            // Cano superior
            glTexCoord2f(0.0f, 1.0f); glVertex2f(pipes[i].x, 0);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(pipes[i].x + PIPE_WIDTH, 0);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(pipes[i].x + PIPE_WIDTH, pipes[i].height);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(pipes[i].x, pipes[i].height);

            // Cano inferior
            glTexCoord2f(0.0f, 0.0f); glVertex2f(pipes[i].x, pipes[i].height + PIPE_GAP);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(pipes[i].x + PIPE_WIDTH, pipes[i].height + PIPE_GAP);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(pipes[i].x + PIPE_WIDTH, HEIGHT);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(pipes[i].x, HEIGHT);
        }
        glEnd();

        glDisable(GL_TEXTURE_2D);

        // Desenha a pontuação
        glColor3f(0.0f, 0.0f, 0.0f);
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        drawText(WIDTH - 100, 30, scoreText);
    } else {
        // Centraliza o texto de Game Over
        glColor3f(0.0f, 0.0f, 0.0f);
        const char* gameOverText = "Game Over! Aperte SPACE para Reiniciar!";
        int textWidth = 0;

        // Calcula a largura do texto
        for (const char* c = gameOverText; *c; ++c) {
            textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
        }

        // Centraliza o texto horizontalmente
        float x = (WIDTH - textWidth) / 2.0f;
        float y = HEIGHT / 2.0f;

        drawText(x, y, gameOverText);
    }

    glutSwapBuffers();
}

// Função de atualização
void update(int value) {
    if (gameRunning) {
        birdY += birdVelocity;
        birdVelocity += GRAVITY;

        // Atualiza os canos
        for (int i = 0; i < 3; i++) {
            pipes[i].x -= 3.0f;
            if (pipes[i].x + PIPE_WIDTH < 0) {
                pipes[i].x = WIDTH;
                pipes[i].height = rand() % (HEIGHT - PIPE_GAP);
                score++;
            }
        }

        // Verifica colisão
        for (int i = 0; i < 3; i++) {
            if (100 + 20 > pipes[i].x && 100 < pipes[i].x + PIPE_WIDTH) {
                if (birdY - 10 < pipes[i].height || birdY + 10 > pipes[i].height + PIPE_GAP) {
                    gameRunning = false;
                }
            }
        }

        // Verifica se o pássaro caiu
        if (birdY + 10 > HEIGHT || birdY - 10 < 0) {
            gameRunning = false;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Função de entrada do teclado
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0);
    } else if (key == ' ') {
        if (!gameRunning) {
            restartGame();
        } else {
            birdVelocity = JUMP_FORCE;
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Flappy Bird");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
