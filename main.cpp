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

// Função de inicialização
void init() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Fundo azul claro
    glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
    srand(time(0));

    for (int i = 0; i < 3; i++) {
        pipes[i].x = WIDTH + i * (PIPE_WIDTH + 200);
        pipes[i].height = rand() % (HEIGHT - PIPE_GAP);
    }
}

// Função de display
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (gameRunning) {
        // Desenha o pássaro
        glColor3f(1.0f, 1.0f, 0.0f);
        glRectf(100, birdY - 10, 120, birdY + 10);

        // Desenha os canos
        glColor3f(0.0f, 1.0f, 0.0f);
        for (int i = 0; i < 3; i++) {
            glRectf(pipes[i].x, 0, pipes[i].x + PIPE_WIDTH, pipes[i].height);
            glRectf(pipes[i].x, pipes[i].height + PIPE_GAP, pipes[i].x + PIPE_WIDTH, HEIGHT);
        }

        // Desenha a pontuação
        glColor3f(0.0f, 0.0f, 0.0f);
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        drawText(WIDTH - 100, 30, scoreText);
    } else {
        drawText(WIDTH / 2 - 50, HEIGHT / 2, "Game Over!");
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
        birdVelocity = JUMP_FORCE;
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
