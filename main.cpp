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

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define ESC 27  // Tecla ESC
#define PIPE_COUNT 3 // Número de canos
#define PIPE_SPACING 3.0f   // Espaçamento entre os canos
#define PIPE_WIDTH 0.4f  // Largura do cano
#define PIPE_HEIGHT 2.5f  // Altura do cano
#define PIPE_DEPTH 0.2f  // Profundidade do cano
#define GRAVITY 0.001f  // Gravidade
#define FLAP_STRENGTH 0.04f  // Força do flap

float birdY = 0.0f; // Posição inicial do pássaro
float birdVelocity = 0.0f; // Velocidade inicial do pássaro
float birdSize = 0.1f; // Tamanho do pássaro
float birdX = -1.0f; // Posição horizontal do pássaro

float pipePositions[PIPE_COUNT]; // Posições horizontais dos canos
float pipeGapY[PIPE_COUNT]; // Posições verticais do gap entre os canos
float pipeGapSize = 0.4f; // Tamanho do gap entre os canos

bool gameOver = false; // Flag para indicar o fim do jogo

int score = 0; // Pontuação do jogador

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

// Função principal
int main(int argc, char** argv) {
    // Inicializa a semente do gerador de números aleatórios
    srand(static_cast<unsigned>(time(0)));

    // Inicializa as posições dos canos
    for (int i = 0; i < PIPE_COUNT; ++i) {
        pipePositions[i] = i * PIPE_SPACING + 1.0f;
        pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        passedPipe[i] = false;
    }

    // Inicializa o GLUT
    init_glut("3D Flappy Bird", argc, argv);
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

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Lighter blue background
}

// Função para desenhar um paralelepípedo
void draw_parallelepiped(float width, float height, float depth) {
    glBegin(GL_QUADS);

    // Front face
    glVertex3f(-width / 2, -height / 2, depth / 2);
    glVertex3f(width / 2, -height / 2, depth / 2);
    glVertex3f(width / 2, height / 2, depth / 2);
    glVertex3f(-width / 2, height / 2, depth / 2);

    // Back face
    glVertex3f(-width / 2, -height / 2, -depth / 2);
    glVertex3f(width / 2, -height / 2, -depth / 2);
    glVertex3f(width / 2, height / 2, -depth / 2);
    glVertex3f(-width / 2, height / 2, -depth / 2);

    // Left face
    glVertex3f(-width / 2, -height / 2, -depth / 2);
    glVertex3f(-width / 2, -height / 2, depth / 2);
    glVertex3f(-width / 2, height / 2, depth / 2);
    glVertex3f(-width / 2, height / 2, -depth / 2);

    // Right face
    glVertex3f(width / 2, -height / 2, -depth / 2);
    glVertex3f(width / 2, -height / 2, depth / 2);
    glVertex3f(width / 2, height / 2, depth / 2);
    glVertex3f(width / 2, height / 2, -depth / 2);

    // Top face
    glVertex3f(-width / 2, height / 2, -depth / 2);
    glVertex3f(width / 2, height / 2, -depth / 2);
    glVertex3f(width / 2, height / 2, depth / 2);
    glVertex3f(-width / 2, height / 2, depth / 2);

    // Bottom face
    glVertex3f(-width / 2, -height / 2, -depth / 2);
    glVertex3f(width / 2, -height / 2, -depth / 2);
    glVertex3f(width / 2, -height / 2, depth / 2);
    glVertex3f(-width / 2, -height / 2, depth / 2);

    glEnd();
}

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

// Função para verificar colisões
bool check_collision(float px, float py, float psize, float ex, float ey, float ewidth, float eheight, float edepth) {
    // Bounding Box do Passaro (um cubo)
    float pxMin = px - psize / 2;
    float pxMax = px + psize / 2;
    float pyMin = py - psize / 2;
    float pyMax = py + psize / 2;

    // Bounding Box do Cano (um paralelepipedo)
    float exMin = ex - ewidth / 2;
    float exMax = ex + ewidth / 2;
    float eyMin = ey - eheight / 2;
    float eyMax = ey + eheight / 2;

    // Verifica se os bounding boxes se sobrepõem
    return !(pxMax < exMin || pxMin > exMax || pyMax < eyMin || pyMin > eyMax);
}

// Função para desenhar a cena
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -3.0f);

    if (!gameOver) {
        // Desenha o pássaro (cubo amarelo)
        glPushMatrix();
        glTranslatef(birdX, birdY, 0.0f);
        glColor3f(1.0f, 1.0f, 0.0f);
        glutSolidCube(birdSize);
        glPopMatrix();

        // Desenha os canos (paralelepípedos verdes)
        for (int i = 0; i < PIPE_COUNT; ++i) {
            // Cano superior
            glPushMatrix();
            glTranslatef(pipePositions[i], pipeGapY[i] + pipeGapSize + PIPE_HEIGHT / 2, 0.0f);
            glColor3f(0.0f, 1.0f, 0.0f);
            draw_parallelepiped(PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH);
            glPopMatrix();

            // Cano inferior
            glPushMatrix();
            glTranslatef(pipePositions[i], pipeGapY[i] - pipeGapSize - PIPE_HEIGHT / 2, 0.0f);
            glColor3f(0.0f, 1.0f, 0.0f);
            draw_parallelepiped(PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH);
            glPopMatrix();
        }

        // Desenha a pontuação
        glColor3f(0.0f, 0.0f, 0.0f);
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        drawText(-1.0f, 1.0f, scoreText);  // Ajuste a posição conforme necessário
    } else {
        // Texto de Game Over
        glColor3f(0.0f, 0.0f, 0.0f);
        const char* gameOverText = "Game Over! Press 'R' to Restart!";
        int textWidth = 0;

        // Calcula a largura do texto
        for (const char* c = gameOverText; *c; ++c) {
            textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
        }

        // Centraliza o texto horizontalmente
        float x = (800 - textWidth) / 2.0f;
        float y = 300; // Ajuste a posição conforme necessário

        drawText(x, y, gameOverText);
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

        // Atualiza a posição horizontal do pássaro
        for (int i = 0; i < PIPE_COUNT; ++i) {
            pipePositions[i] -= 0.05f; // Atualizado para novo PIPE_SPACING

            // Verifica se o cano saiu da tela
            if (pipePositions[i] < -3.0f) { // Atualizado para novo PIPE_SPACING
                pipePositions[i] += PIPE_COUNT * PIPE_SPACING;
                pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
                passedPipe[i] = false; // Reset flag
            }

            // Verifica se o pássaro passou pelo cano
            for (int j = 0; j < PIPE_COUNT; ++j) {
                // Verifica se o pássaro está na mesma posição horizontal que o cano
                if (pipePositions[j] < birdX + birdSize / 2 && pipePositions[j] > birdX - birdSize / 2) {
                    // Verifica colisão com a parte superior do cano
                    if (check_collision(birdX, birdY, birdSize, pipePositions[j], pipeGapY[j] + pipeGapSize + PIPE_HEIGHT / 2, PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH)) {
                        gameOver = true;
                        break;
                    }
                    // Verifica colisão com a parte inferior do cano
                    if (check_collision(birdX, birdY, birdSize, pipePositions[j], pipeGapY[j] - pipeGapSize - PIPE_HEIGHT / 2, PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH)) {
                        gameOver = true;
                        break;
                    }
                }
            }

            // Incrementa o score se o pássaro passou pelo cano
            if (!passedPipe[i] && pipePositions[i] < birdX - birdSize / 2) {
                passedPipe[i] = true;
                score++;
            }
        }

        // Verifica se o pássaro saiu da tela
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