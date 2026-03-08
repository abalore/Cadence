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
}

void PboWidget::paintGL()
{
    float xOffset = 14.0f / 64.0f;
    float xSize = 48.0f / 64.0f;
    float yOffset = 3.0f / 34.0f;
    float ySize = 34.0f / 39.0f;

    glBindTexture(GL_TEXTURE_2D, ID);
    glBegin(GL_QUADS);
    glTexCoord2f(xOffset, yOffset);
    glVertex2f(-1, 1);
    glTexCoord2f(xSize + xOffset, yOffset);
    glVertex2f(1, 1);
    glTexCoord2f(xSize + xOffset, ySize + yOffset);
    glVertex2f(1, -1);
    glTexCoord2f(xOffset, ySize + yOffset);
    glVertex2f(-1, -1);
    glEnd();
    glFlush();
}

void PboWidget::updateTexture()
{
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 312, 0, GL_RGB, GL_UNSIGNED_BYTE, CRTScreen::Pixels);
    update();
}

void PboWidget::setSmoothing(bool enabled)
{
    if (enabled)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}
