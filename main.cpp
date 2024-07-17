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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <sstream> // Adicionado


#define WIDTH 800
#define HEIGHT 600
#define PIPE_GAP 150
#define PIPE_RADIUS 40 
#define GRAVITY 0.1f
#define JUMP_FORCE -3.0f

float birdY = HEIGHT / 2.0f;
float birdVelocity = 0.0f;
bool gameRunning = true;
int score = 0;

struct Pipe {
    float x;
    float height;
};

std::vector<Pipe> pipes;

struct Model {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned int> indices;
};

Model birdModel;

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

bool loadOBJ(const char* path, Model& model) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "ERR: " << err << std::endl;
    }

    if (!ret) {
        return false;
    }

    for (size_t s = 0; s < shapes.size(); s++) {
        const tinyobj::shape_t& shape = shapes[s];
        for (size_t i = 0; i < shape.mesh.indices.size(); i++) {
            tinyobj::index_t idx = shape.mesh.indices[i];

            model.vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
            model.vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
            model.vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

            if (!attrib.normals.empty()) {
                model.normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
                model.normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
                model.normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
            }

            if (!attrib.texcoords.empty()) {
                model.texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                model.texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
            }

            model.indices.push_back(model.indices.size());
        }
    }

    return true;
}

void init() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Fundo azul claro
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

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

    for (int i = 0; i < 3; i++) {
        Pipe pipe;
        pipe.x = WIDTH + i * (PIPE_RADIUS * 2 + 200);
        pipe.height = rand() % (HEIGHT - PIPE_GAP);
        pipes.push_back(pipe);
    }

    // Carregar o modelo do p�ssaro
    if (!loadOBJ("C:\\Users\\Cayo Cesar\\OneDrive - ufpi.edu.br\\Documentos\\GitHub\\OpenGL-Game\\flappy.obj", birdModel)) {
        std::cerr << "Failed to load bird model" << std::endl;
        exit(1);
    }
}

void drawModel(const Model& model) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &model.vertices[0]);
    
    if (!model.normals.empty()) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, &model.normals[0]);
    }
    
    if (!model.texcoords.empty()) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, &model.texcoords[0]);
    }

    glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, &model.indices[0]);

    glDisableClientState(GL_VERTEX_ARRAY);
    if (!model.normals.empty()) glDisableClientState(GL_NORMAL_ARRAY);
    if (!model.texcoords.empty()) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void drawCilinder(float radius, float height) {
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, radius, radius, height, 20, 20);
    gluDeleteQuadric(quad);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gameRunning) {
        GLfloat birdAmbient[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        GLfloat birdDiffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        GLfloat birdSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat birdShininess[] = { 50.0f };

        glMaterialfv(GL_FRONT, GL_AMBIENT, birdAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, birdDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, birdSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, birdShininess);

        glPushMatrix();
        glTranslatef(100, birdY, 0);
        glScalef(10.0f, 10.0f, 10.0f); // Ajustar o tamanho do modelo
        drawModel(birdModel);
        glPopMatrix();

        GLfloat pipeAmbient[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        GLfloat pipeDiffuse[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        GLfloat pipeSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat pipeShininess[] = { 50.0f };

        glMaterialfv(GL_FRONT, GL_AMBIENT, pipeAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, pipeDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, pipeSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, pipeShininess);

        // Desenhar os tubos (pode manter a fun��o drawCylinder ou substitu�-la por algo semelhante)
        for (size_t i = 0; i < pipes.size(); i++) {
            glPushMatrix();
            glTranslatef(pipes[i].x + PIPE_RADIUS, pipes[i].height / 2, 0);
            glScalef(PIPE_RADIUS, pipes[i].height, PIPE_RADIUS);
            glPopMatrix();
            glPushMatrix();
            float pipeHeight = HEIGHT - pipes[i].height - PIPE_GAP; // Adicionada a declara��o de height
            glTranslatef(pipes[i].x + PIPE_RADIUS, (pipes[i].height + PIPE_GAP) + pipeHeight / 2, 0);
            glScalef(PIPE_RADIUS, pipeHeight, PIPE_RADIUS);
            glPopMatrix();
        }

        std::stringstream ss;
        ss << "Score: " << score;
        drawText(10, HEIGHT - 20, ss.str().c_str());
    } else {
        drawText(WIDTH / 2 - 50, HEIGHT / 2, "Game Over");
        
        std::stringstream ss;
        ss << "Final Score: " << score;
        drawText(WIDTH / 2 - 50, HEIGHT / 2 - 30, ss.str().c_str());
    }

    glutSwapBuffers();
}

void update(int value) {
    if (gameRunning) {
        birdVelocity += GRAVITY;
        birdY += birdVelocity;

        if (birdY < 0) {
            birdY = 0;
            birdVelocity = 0;
        }

        for (size_t i = 0; i < pipes.size(); i++) {
            pipes[i].x -= 2;

            if (pipes[i].x + PIPE_RADIUS < 0) {
                pipes[i].x = WIDTH;
                pipes[i].height = rand() % (HEIGHT - PIPE_GAP);
                score++;
            }

            if (pipes[i].x < 100 + 10 && pipes[i].x + PIPE_RADIUS > 100 - 10) {
                if (birdY < pipes[i].height || birdY > pipes[i].height + PIPE_GAP) {
                    gameRunning = false;
                }
            }
        }

        if (birdY > HEIGHT) {
            gameRunning = false;
        }

        glutPostRedisplay();
    }

    glutTimerFunc(16, update, 0);
}

void handleKeypress(unsigned char key, int x, int y) {
    if (key == 27) { // ESC
        exit(0);
    }

    if (key == ' ') {
        birdVelocity = JUMP_FORCE;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("3D Game");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
