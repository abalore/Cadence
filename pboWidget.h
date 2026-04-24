#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QImage>

#define imageWidth 1024
#define imageHeight 312
#define offset_x 0.20f
#define offset_y 0.05f
#define size_x 0.75f
#define size_y 0.75f

class PboWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    PboWidget(QWidget *parent);
    void updateTexture();
    void setSmoothing(bool enabled);
    void applySmoothing();
    void setPersistence(int level);
protected:
    void initializeGL() override;
    void paintGL() override;
private:
    unsigned int ID = 0;
    bool smoothing = true;
    int persistence = 0;
    unsigned char prev[imageWidth * imageHeight * 3] = {};
    unsigned char blended[imageWidth * imageHeight * 3];
};
