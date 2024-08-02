/*
 Jogo Flappy Bird em 3D utilizando OpenGL e GLUT
 */

// Importa√ß√£o das bibliotecas necess√°rias
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
#define PIPE_COUNT 3 // N√∫mero de canos
#define PIPE_SPACING 3.0f   // Espa√ßamento entre os canos
#define PIPE_WIDTH 0.4f  // Largura do cano
#define PIPE_HEIGHT 2.5f  // Altura do cano
#define PIPE_DEPTH 0.2f  // Profundidade do cano
#define GRAVITY 0.001f  // Gravidade
#define FLAP_STRENGTH 0.04f  // For√ßa do flap

//carregar
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float birdY = 0.0f; // Posi√ß√£o inicial do p√°ssaro
float birdVelocity = 0.0f; // Velocidade inicial do p√°ssaro
float birdSize = 0.1f; // Tamanho do p√°ssaro
float birdX = -1.0f; // Posi√ß√£o horizontal do p√°ssaro

float pipePositions[PIPE_COUNT]; // Posi√ß√µes horizontais dos canos
float pipeGapY[PIPE_COUNT]; // Posi√ß√µes verticais do gap entre os canos
float pipeGapSize = 0.4f; // Tamanho do gap entre os canos

bool gameOver = false; // Flag para indicar o fim do jogo

int score = 0; // Pontua√ß√£o do jogador

// Flag para verificar se o p√°ssaro passou por um cano
bool passedPipe[PIPE_COUNT] = { false };

// Declara√ß√£o das fun√ß√µes
void init_glut(const char *window_name, int argc, char** argv);
void display(void);
void reshape(int w, int h);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void resetGame(void);
void draw_parallelepiped(float width, float height, float depth);  // Function added
bool check_collision(float px, float py, float psize, float ex, float ey, float ewidth, float eheight, float edepth); // Function added

// Fun√ß√£o principal
int main(int argc, char** argv) {
	
    // Inicializa a semente do gerador de n√∫meros aleat√≥rios
    srand(static_cast<unsigned>(time(0)));

    // Inicializa as posi√ß√µes dos canos
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

// Fun√ß√£o para inicializar o GLUT
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

    // ConfiguraÁ„o da iluminaÁ„o
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // ConfiguraÁ„o da luz ambiente
    GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    // ConfiguraÁ„o da luz difusa
    GLfloat diffuseLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

    // ConfiguraÁ„o da luz especular
    GLfloat specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // ConfiguraÁ„o da posiÁ„o da luz
    GLfloat lightPosition[] = {1.0f, 1.0f, 1.0f, 0.0f}; // Luz direcionada
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // Ativa a suavizaÁ„o de sombras
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}


// Fun√ß√£o para desenhar um paralelep√≠pedo
void draw_parallelepiped(float width, float height, float depth) {
    // ConfiguraÁ„o do material para os canos
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
    glVertex3f(-width / 2, -height / 2, depth / 2);
    glVertex3f(width / 2, -height / 2, depth / 2);
    glVertex3f(width / 2, height / 2, depth / 2);
    glVertex3f(-width / 2, height / 2, depth / 2);

    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-width / 2, -height / 2, -depth / 2);
    glVertex3f(width / 2, -height / 2, -depth / 2);
    glVertex3f(width / 2, height / 2, -depth / 2);
    glVertex3f(-width / 2, height / 2, -depth / 2);

    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-width / 2, -height / 2, -depth / 2);
    glVertex3f(-width / 2, -height / 2, depth / 2);
    glVertex3f(-width / 2, height / 2, depth / 2);
    glVertex3f(-width / 2, height / 2, -depth / 2);

    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(width / 2, -height / 2, -depth / 2);
    glVertex3f(width / 2, -height / 2, depth / 2);
    glVertex3f(width / 2, height / 2, depth / 2);
    glVertex3f(width / 2, height / 2, -depth / 2);

    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-width / 2, height / 2, -depth / 2);
    glVertex3f(width / 2, height / 2, -depth / 2);
    glVertex3f(width / 2, height / 2, depth / 2);
    glVertex3f(-width / 2, height / 2, depth / 2);

    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
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


// Fun√ß√£o para verificar colis√µes
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

    // Verifica se os bounding boxes se sobrep√µem
    return !(pxMax < exMin || pxMin > exMax || pyMax < eyMin || pyMin > eyMax);
}

// Fun√ß√£o para desenhar a cena
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -3.0f);

    if (!gameOver) {
        // Desenha o p·ssaro (cubo amarelo)
        glPushMatrix();
        glTranslatef(birdX, birdY, 0.0f);

        // ConfiguraÁ„o do material para o p·ssaro
        GLfloat materialDiffuse[] = {1.0f, 1.0f, 0.0f, 1.0f}; // Amarelo para o p·ssaro
        GLfloat materialSpecular[] = {1.0f, 1.0f, 0.0f, 1.0f}; // Amarelo para o p·ssaro
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

        glutSolidCube(birdSize);
        glPopMatrix();

        // Desenha os canos (paralelepÌpedos verdes)
        for (int i = 0; i < PIPE_COUNT; ++i) {
            // Cano superior
            glPushMatrix();
            glTranslatef(pipePositions[i], pipeGapY[i] + pipeGapSize + PIPE_HEIGHT / 2, 0.0f);
            draw_parallelepiped(PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH);
            glPopMatrix();

            // Cano inferior
            glPushMatrix();
            glTranslatef(pipePositions[i], pipeGapY[i] - pipeGapSize - PIPE_HEIGHT / 2, 0.0f);
            draw_parallelepiped(PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH);
            glPopMatrix();
        }

        // Desenha a pontuaÁ„o
        glColor3f(0.0f, 0.0f, 0.0f);
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0); // Coordenadas para a pontuaÁ„o
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glRasterPos2f(0.8f, 0.9f);  // PosiÁ„o no canto superior direito
        drawText(0.0f, 0.0f, scoreText);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    } else {
        // Mensagem combinada de Game Over e reinÌcio
        glColor3f(0.0f, 0.0f, 0.0f);
        char gameOverText[100];
        sprintf(gameOverText, "Game Over! Pressione 'R' para reiniciar!");

        // ConfiguraÁ„o de projeÁ„o para centralizar o texto
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0); // Coordenadas de projeÁ„o
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

        glRasterPos2f(centerX - textWidth / 2 / 800.0f, centerY);  // Ajuste de acordo com a resoluÁ„o

        drawText(0.0f, 0.0f, gameOverText);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    glutSwapBuffers();
}



// Fun√ß√£o para redimensionar a janela
void reshape(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 100.0); // Increased view distance
    glMatrixMode(GL_MODELVIEW);
}

// Fun√ß√£o para atualizar a cena
void timer(int value) {
    if (!gameOver) {
        birdVelocity -= GRAVITY;
        birdY += birdVelocity;

        // Atualiza a posiÁ„o horizontal do p·ssaro
        for (int i = 0; i < PIPE_COUNT; ++i) {
            pipePositions[i] -= 0.025f; // Velocidade do cano reduzida

            // Verifica se o cano saiu da tela
            if (pipePositions[i] < -3.0f) { // Atualizado para novo PIPE_SPACING
                pipePositions[i] += PIPE_COUNT * PIPE_SPACING;
                pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
                passedPipe[i] = false; // Reset flag
            }

            // Verifica se o p·ssaro passou pelo cano
            for (int j = 0; j < PIPE_COUNT; ++j) {
                // Verifica se o p·ssaro est· na mesma posiÁ„o horizontal que o cano
                if (pipePositions[j] < birdX + birdSize / 2 && pipePositions[j] > birdX - birdSize / 2) {
                    // Verifica colis„o com a parte superior do cano
                    if (check_collision(birdX, birdY, birdSize, pipePositions[j], pipeGapY[j] + pipeGapSize + PIPE_HEIGHT / 2, PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH)) {
                        gameOver = true;
                        break;
                    }
                    // Verifica colis„o com a parte inferior do cano
                    if (check_collision(birdX, birdY, birdSize, pipePositions[j], pipeGapY[j] - pipeGapSize - PIPE_HEIGHT / 2, PIPE_WIDTH, PIPE_HEIGHT, PIPE_DEPTH)) {
                        gameOver = true;
                        break;
                    }
                }
            }

            // Incrementa o score se o p·ssaro passou pelo cano
            if (!passedPipe[i] && pipePositions[i] < birdX - birdSize / 2) {
                passedPipe[i] = true;
                score++;
            }
        }

        // Verifica se o p·ssaro saiu da tela
        if (birdY - birdSize / 2 < -1.5f || birdY + birdSize / 2 > 1.5f) {
            gameOver = true;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}


// Fun√ß√£o para tratar eventos do teclado
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

// Fun√ß√£o para reiniciar o jogo
void resetGame(void) {
    birdY = 0.0f;
    birdVelocity = 0.0f;
    birdX = -1.0f;
    score = 0; // Reseta a pontua√ß√£o ao reiniciar o jogo

    for (int i = 0; i < PIPE_COUNT; ++i) {
        pipePositions[i] = i * PIPE_SPACING + 1.0f;
        pipeGapY[i] = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
        passedPipe[i] = false; // Reset flag
    }

    gameOver = false;
}