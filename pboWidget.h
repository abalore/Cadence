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
protected:
    void initializeGL() override;
    void paintGL() override;
private:
    unsigned int ID;
};
