#include "graphicsinspector.h"
#include "ui_graphicsinspector.h"
#include "Emulator/Headers/defs.h"
#include "Emulator/Headers/GateArray.h"
#include "Emulator/Headers/CPC.h"

GraphicsInspector::GraphicsInspector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GraphicsInspector)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 640, 400);
    ui->graphicsView->setScene(scene);
    pixItem = 0;

    connect(ui->btnUpdate, &QPushButton::clicked, this, &GraphicsInspector::UpdateGraphics);
}

GraphicsInspector::~GraphicsInspector()
{
    delete ui;
}

void GraphicsInspector::UpdateGraphics()
{
    BYTE xSize = 32;
    BYTE ySize = 32;
    int byteSize = xSize * 16 * ySize * 16 * 3;
    word baseAddress = 0xC000;
    if (ui->radioButton->isChecked()) baseAddress = 0x0000;
    if (ui->radioButton_2->isChecked()) baseAddress = 0x4000;
    if (ui->radioButton_3->isChecked()) baseAddress = 0x8000;
    int mode = GateArray::mode;
    static BYTE *Pixels = (BYTE *)malloc(byteSize);
    for (int i = 0; i < xSize * 2; i++)
        for  (int j = 0; j < ySize; j++)
            for (int k = 0; k < 8; k++)
            {
                word address =  ((k * 0x0800 + j * xSize * 2 + i) % 0x4000);
                BYTE bank = baseAddress >> 14;
                BYTE b = CPC::RAM[bank][address];
                int line = j * 8 + k;
                for (int l = 0; l < 8; l++)
                {
                    BYTE pen = GateArray::GetPenForPixel(mode, b, l);
                    const BYTE *color = GateArray::GetPaletteEntry(pen);
                    long pixelBase = (line * xSize * 16 * 2 + i * 8 + l) * 3;
                    if (pixelBase < byteSize - 2)
                        for (int m = 0; m < 3; m++)
                        {
                            Pixels[pixelBase + m] = color[m];
                            Pixels[pixelBase + m + xSize * 16 * 3] = color[m];
                        }
                }
            }
    if (pixItem) scene->removeItem(pixItem);
    QImage *image = new QImage(&Pixels[0], xSize * 16, ySize * 16, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(*image, Qt::NoFormatConversion | Qt::NoOpaqueDetection);
    pixItem = scene->addPixmap(pixmap);
    pixItem->setPos(0, 0);
}

