#include "pboWidget.h"
#include "CPC.h"
#include <cstring>

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
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_TEXTURE_2D);
    applySmoothing();
}

void PboWidget::applySmoothing()
{
    GLint filter = smoothing ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
}

void PboWidget::paintGL()
{
    float xOffset = 15.0f / 64.0f; //14.0f / 64.0f;
    float xSize = 48.0f / 64.0f; //48.0f / 64.0f;
    float yOffset = 3.5f / 39.0f; //3.0f / 39.0f;
    float ySize = 34.0f / 39.0f; //34.0f / 39.0f;

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
    const BYTE *src = CPC::screen.Pixels;
    const BYTE *upload = src;
    if (persistence > 0)
    {
        constexpr int N = imageWidth * imageHeight * 3;
        const int a = persistence; // decay retention (0..90)
        for (int i = 0; i < N; i++)
        {
            int decayed = (prev[i] * a) / 100;
            int s = src[i];
            blended[i] = (decayed > s) ? decayed : s;
        }
        memcpy(prev, blended, N);
        upload = blended;
    }
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 312, 0, GL_RGB, GL_UNSIGNED_BYTE, upload);
    update();
}

void PboWidget::setSmoothing(bool enabled)
{
    smoothing = enabled;
    if (ID == 0) return;
    makeCurrent();
    glBindTexture(GL_TEXTURE_2D, ID);
    applySmoothing();
    doneCurrent();
}

void PboWidget::setPersistence(int level)
{
    if (level < 0) level = 0;
    if (level > 90) level = 90;
    persistence = level;
    if (level == 0) memset(prev, 0, sizeof(prev));
}
