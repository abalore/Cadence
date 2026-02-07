#include "pboWidget.h"
#include "Emulator/Headers/CRTScreen.h"

BYTE *pixels;

PboWidget::PboWidget(QWidget *parent) : QOpenGLWidget(parent)
{

}

void PboWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 1, 1);
    glColor3f(1,1,1);

    ID = 0;
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_TEXTURE_2D);

    //glBindTexture(GL_TEXTURE_2D, ID);
    //pixels = (BYTE *)malloc(100 * 100 * 3 * 2);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

}

void PboWidget::updateTexture()
{
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 312, 0, GL_RGB, GL_UNSIGNED_BYTE, CRTScreen::Pixels);
    update();
}

void PboWidget::paintGL()
{
    glBindTexture(GL_TEXTURE_2D, ID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.125f, 0.11f);
    glVertex2f(-1, 1);
    glTexCoord2f(0.875f, 0.11f);
    glVertex2f(1, 1);
    glTexCoord2f(0.875f, 0.846153846f + 0.11);
    glVertex2f(1, -1);
    glTexCoord2f(0.125f, 0.846153846f + 0.11);
    glVertex2f(-1, -1);
    glEnd();
    glFlush();
}
