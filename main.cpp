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
#include <vector>

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

std::vector<Pipe> pipes;

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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Configuração da luz
    GLfloat lightPos[] = { 400.0f, 300.0f, 1000.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, (double)WIDTH / (double)HEIGHT, 1.0, 2000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(400.0, 300.0, 1000.0, 400.0, 300.0, 0.0, 0.0, 1.0, 0.0);
    srand(time(0));

    // Inicializa os primeiros canos
    for (int i = 0; i < 3; i++) {
        Pipe pipe;
        pipe.x = WIDTH + i * (PIPE_WIDTH + 200);
        pipe.height = rand() % (HEIGHT - PIPE_GAP);
        pipes.push_back(pipe);
    }
}

// Função para desenhar um cilindro 3D
void drawCylinder(float x, float y, float z, float radius, float height) {
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluCylinder(quad, radius, radius, height, 32, 32);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

// Função de display
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gameRunning) {
        // Material do pássaro
        GLfloat birdAmbient[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        GLfloat birdDiffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        GLfloat birdSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat birdShininess[] = { 50.0f };

        glMaterialfv(GL_FRONT, GL_AMBIENT, birdAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, birdDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, birdSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, birdShininess);

        // Desenha o pássaro
        drawCylinder(100, birdY, 0, 10, 20);

        // Material dos canos
        GLfloat pipeAmbient[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        GLfloat pipeDiffuse[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        GLfloat pipeSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat pipeShininess[] = { 50.0f };

        glMaterialfv(GL_FRONT, GL_AMBIENT, pipeAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, pipeDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, pipeSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, pipeShininess);

        // Desenha os canos
        for (size_t i = 0; i < pipes.size(); i++) {
            drawCylinder(pipes[i].x + PIPE_WIDTH / 2, pipes[i].height / 2, 0, PIPE_WIDTH / 2, pipes[i].height);
            drawCylinder(pipes[i].x + PIPE_WIDTH / 2, (pipes[i].height + PIPE_GAP) + (HEIGHT - pipes[i].height - PIPE_GAP) / 2, 0, PIPE_WIDTH / 2, HEIGHT - pipes[i].height - PIPE_GAP);
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
        for (size_t i = 0; i < pipes.size(); i++) {
            pipes[i].x -= 3.0f;
        }

        // Remove canos fora da tela e adiciona novos canos
        if (!pipes.empty() && pipes[0].x + PIPE_WIDTH < 0) {
            pipes.erase(pipes.begin());

            Pipe newPipe;
            newPipe.x = WIDTH;
            newPipe.height = rand() % (HEIGHT - PIPE_GAP);
            pipes.push_back(newPipe);

            score++;
        }

        // Verifica colisão
        for (size_t i = 0; i < pipes.size(); i++) {
            if (100 + 10 > pipes[i].x && 100 - 10 < pipes[i].x + PIPE_WIDTH) {
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Flappy Bird 3D");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
