#include <GL/freeglut_std.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <cmath>

float cameraX = 0;
float cameraY = 0;
float cameraZ = 5;
float cameraShift = 1;

float rotationAngle = 0;
float translateSphereZ = 0;
float translateSphereSpeed = 0.005;
float torusScale = 1;
float torusScaleSpeed = 0.001;
bool isGrowing = true;

float rotationSpeed = 0.1;
int previousMouseX = -1;
int previousMouseY = -1;

bool teapotTextureEnabled = false;
bool torusTextureEnabled = false;
bool heartTextureEnabled = false;

bool firstLightEnabled = true;
bool secondLightEnabled = true;

GLuint torusTexture;
GLuint teapotTexture;

void loadTexture(const char* filename, GLuint &texture){
    FILE *file = fopen(filename, "rb");
    if(!file){
        printf("Ne može učitati teksturu\n");
        exit(1);
    }

    unsigned char header[54];
    if (fread(header, 1, 54, file) != 54){
        printf("Nevalidan format fajla\n");
        fclose(file);
        exit(1);
    }

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int channels = 3;
    int imageSize = channels * width * height;
    unsigned char *data = (unsigned char *)malloc(imageSize);

    if (fread(data, 1, imageSize, file) != imageSize){
        printf("Greška pri čitanju slike\n");
        fclose(file);
        exit(1);
    }

    fclose(file);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    // Oslobodi memoriju
    free(data);
}

void setFirstLight() {
    GLfloat light_position[] = {1.0, 3.0, 1.0, 0.0};
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat no_light[] = {0.0, 0.0, 0.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, no_light);
    if (firstLightEnabled)
        glEnable(GL_LIGHT0);
    else
        glDisable(GL_LIGHT0);
}

void setSecondLight() {
    GLfloat light_position[] = {1.0, -3.0, 1.0, 0.0};
    GLfloat white_light[] = {0.7, 0.7, 0.7, 1.0};
    GLfloat yellow_light[] = {1.0, 1.0, 0.0, 1.0};
    glLightfv(GL_LIGHT1, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, yellow_light);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
    if (secondLightEnabled)
        glEnable(GL_LIGHT1);
    else
        glDisable(GL_LIGHT1);
}

void setFirstMaterial() {
    GLfloat mat_ambient[] = {0.2, 0.0, 0.0, 1.0};
    GLfloat mat_diffuse[] = {0.8, 0.8, 0.0, 1.0};
    GLfloat mat_specular[] = {0.5, 0.0, 0.0, 1.0};
    GLfloat mat_shininess[] = {110.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void setSecondMaterial() {
    GLfloat mat_ambient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat mat_diffuse[] = {0.8, 0.0, 0.8, 1.0};
    GLfloat mat_specular[] = {0.3, 0.3, 0.3, 1.0};
    GLfloat mat_shininess[] = {5.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void drawDrop() {
    if(heartTextureEnabled)
        setFirstMaterial();
    else{
        GLfloat materialDiffuse[] = {1.0f, 0.5f, 0.0f, 1.0f}; // Orange
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    }

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.5f);
    for (int i = 0; i <= 360; i += 1) {
        float theta = i * 3.14159 / 180;
        float x = 0.5 * sin(theta);
        float y = 0.5 * cos(theta) - 0.5;
        glVertex2f(x, y);
    }
    glEnd();
}

void drawSphere() {
    GLfloat materialDiffuse[] = {0.0f, 1.0f, 1.0f, 1.0f}; // Cyan
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glutSolidSphere(0.5, 30, 30);    
}

void drawTorus() {
    if(torusTextureEnabled){
        glBindTexture(GL_TEXTURE_2D, torusTexture);
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
        GLfloat materialDiffuse[] = {0.5f, 0.0f, 0.5f, 1.0f}; // Purple
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    }
    glutSolidTorus(0.2, 0.5, 25, 25);
    //glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void drawCone() {
    GLfloat materialDiffuse[] = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glutWireCone(0.5, 1, 10, 10);
}

void drawTeapot() {
    if(teapotTextureEnabled){
        glBindTexture(GL_TEXTURE_2D, teapotTexture);
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
        GLfloat material_diffuse[] = {1, 0.3, 0.3, 1}; // Set diffuse material color to light red
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);
    }
    glutSolidTeapot(0.7);
    glDisable(GL_TEXTURE_2D);
}

void setBackgroundColor() {
    glClearColor(0.0f, 0.5f, 0.0f, 1.0f); // Tamnozelena boja pozadine
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    setFirstLight();
    setSecondLight();
 
    gluLookAt(cameraX, cameraY, cameraZ, 0, 0, 0, 0, 0.1, 0);
    setBackgroundColor();

    glTranslatef(0, 0, 3);

    // Kapljica
    glPushMatrix();
    glRotatef(rotationAngle, 1, 1, 1);
    drawDrop();
    glPopMatrix();

    // Torus
    glPushMatrix();
    glTranslatef(1, 3, -1);
    glScalef(torusScale, torusScale, torusScale);
    drawTorus();
    glPopMatrix();

    // Sfera
    glPushMatrix();
    glTranslatef(-3, 0, -1);
    glTranslatef(translateSphereZ, 0, 0);
    drawSphere();
    glPopMatrix();

    // Konus
    glPushMatrix();
    glTranslatef(0, -3, -1);
    glRotatef(rotationAngle, 0, 1, 0);
    drawCone();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3, 0, -1);
    glRotatef(rotationAngle, 0, 0, 1);
    drawTeapot();
    glPopMatrix();

    glFlush();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    GLfloat lmodel_ambient[] = {0.3, 0.3, 0.3, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-5, 5, -5, 5, -10, 10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    loadTexture("3.bmp", teapotTexture);
    loadTexture("1.bmp", torusTexture);
}

void animation() {
    rotationAngle += rotationSpeed;
    if (rotationAngle > 360)
        rotationAngle -= 360;

    translateSphereZ += translateSphereSpeed;
    if (translateSphereZ > 1.5 || translateSphereZ < -1)
        translateSphereSpeed = -translateSphereSpeed;

    if (isGrowing) {
        torusScale += torusScaleSpeed;
        if (torusScale >= 2)
            isGrowing = false;
    } else {
        torusScale -= torusScaleSpeed;
        if (torusScale <= 1)
            isGrowing = true;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            cameraY += cameraShift;
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN:
            cameraY -= cameraShift;
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT:
            cameraX -= cameraShift;
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT:
            cameraX += cameraShift;
            glutPostRedisplay();
            break;
        default:
            break;
    }
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        previousMouseX = x;
        previousMouseY = y;

        switch(button) {
            case GLUT_LEFT_BUTTON:
                rotationSpeed = 0.1;
                break;
            case GLUT_MIDDLE_BUTTON:
                translateSphereSpeed = 0.005;
                break;
            case GLUT_RIGHT_BUTTON:
                torusScaleSpeed = 0.001;
                break;
        }
    } else if (state == GLUT_UP) {
        switch(button) {
            case GLUT_LEFT_BUTTON:
                rotationSpeed = 0;
                break;
            case GLUT_MIDDLE_BUTTON:
                translateSphereSpeed = 0;
                break;
            case GLUT_RIGHT_BUTTON:
                torusScaleSpeed = 0;
                break;
        }
    }
}

void motion(int x, int y) {
    if (previousMouseX == -1 || previousMouseY == -1) {
        previousMouseX = x;
        previousMouseY = y;
        return;
    }

    int pathY = y - previousMouseY;

    if (rotationSpeed != 0)
        rotationSpeed += pathY * 0.001;
    else if (translateSphereSpeed != 0)
        translateSphereSpeed += pathY * 0.0001;
    else if (torusScaleSpeed != 0)
        torusScaleSpeed += pathY * 0.00001;

    previousMouseX = x;
    previousMouseY = y;
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            exit(0);
        case '0': 
            firstLightEnabled = !firstLightEnabled;
            glutPostRedisplay();
            break;
        case '1': 
            secondLightEnabled = !secondLightEnabled;
            glutPostRedisplay();
            break;
        case '2':
            torusTextureEnabled = !torusTextureEnabled;
            glutPostRedisplay();
            break;
        case '3': 
            teapotTextureEnabled = !teapotTextureEnabled;
            glutPostRedisplay();
            break;
        case '4': 
            heartTextureEnabled = !heartTextureEnabled;
            glutPostRedisplay();
            break;
        default: break;
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(120, (double)w / (double)h, 1.0, 250);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(500, 500);
    glutCreateWindow("3D Scene");

    init();

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(animation);

    glutMainLoop();
    return 0;
}
