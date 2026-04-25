#include "graphicsinspector.h"
#include "ui_graphicsinspector.h"
#include "defs.h"
#include "CPC.h"

GraphicsInspector::GraphicsInspector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GraphicsInspector)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 640, 400);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    pixItem = 0;

    connect(ui->inputWidth, &QLineEdit::editingFinished, this, &GraphicsInspector::UpdateGraphics);
    connect(ui->inputHeight, &QLineEdit::editingFinished, this, &GraphicsInspector::UpdateGraphics);
    connect(ui->inputAddress, &QLineEdit::editingFinished, this, &GraphicsInspector::UpdateGraphics);
    connect(ui->rbMode0, &QRadioButton::toggled, this, &GraphicsInspector::UpdateGraphics);
    connect(ui->rbMode1, &QRadioButton::toggled, this, &GraphicsInspector::UpdateGraphics);
    connect(ui->rbMode2, &QRadioButton::toggled, this, &GraphicsInspector::UpdateGraphics);
    connect(ui->rbMode3, &QRadioButton::toggled, this, &GraphicsInspector::UpdateGraphics);
    connect(ui->rbPaletteOriginal, &QRadioButton::toggled, this, &GraphicsInspector::UpdateGraphics);
    connect(ui->rbPaletteFalseColor, &QRadioButton::toggled, this, &GraphicsInspector::UpdateGraphics);
}

GraphicsInspector::~GraphicsInspector()
{
    delete ui;
}

void GraphicsInspector::showEvent(QShowEvent *event)
{
    ui->inputWidth->setText(QString::number(CPC::crtc.HD));
    ui->inputHeight->setText(QString::number(CPC::crtc.VD));
    switch (CPC::gateArray.GetMode())
    {
    case 0: ui->rbMode0->setChecked(true); break;
    case 1: ui->rbMode1->setChecked(true); break;
    case 2: ui->rbMode2->setChecked(true); break;
    case 3: ui->rbMode3->setChecked(true); break;
    }
    ui->rbPaletteOriginal->setChecked(true);
    QDialog::showEvent(event);
}

void GraphicsInspector::UpdateGraphics()
{
    BYTE xSize = ui->inputWidth->text().toInt(nullptr, 10);
    BYTE ySize = ui->inputHeight->text().toInt(nullptr, 10);
    int byteSize = xSize * 16 * ySize * 16 * 3;
    word baseAddress = ui->inputAddress->text().toUInt(nullptr, 16);
    BYTE bank = baseAddress >> 14;
    word offset = baseAddress & 0x3FFF;
    int mode = CPC::gateArray.GetMode();
    if (ui->rbMode0->isChecked()) mode = 0;
    else if (ui->rbMode1->isChecked()) mode = 1;
    else if (ui->rbMode2->isChecked()) mode = 2;
    else if (ui->rbMode3->isChecked()) mode = 3;
    static const BYTE debugPaletteTable[16][3] = {
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x40}, {0x00, 0x40, 0x00}, {0x40, 0x00, 0x00},
        {0x40, 0x40, 0x40}, {0x00, 0x00, 0x80}, {0x00, 0x80, 0x00}, {0x80, 0x00, 0x00},
        {0x80, 0x80, 0x80}, {0x00, 0x00, 0xFF}, {0x00, 0xFF, 0x00}, {0xFF, 0x00, 0x00},
        {0xFF, 0xFF, 0xFF}, {0x00, 0xFF, 0xFF}, {0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0xFF},
    };
    BYTE paletteTable[16][3];
    bool debugPalette = ui->rbPaletteFalseColor->isChecked();
    for (int p = 0; p < 16; p++)
    {
        const BYTE *c = debugPalette ? debugPaletteTable[p]
                                     : CPC::gateArray.GetPaletteEntry(p);
        paletteTable[p][0] = c[0];
        paletteTable[p][1] = c[1];
        paletteTable[p][2] = c[2];
    }
    pixels.resize(byteSize);
    for (int i = 0; i < xSize * 2; i++)
        for  (int j = 0; j < ySize; j++)
            for (int k = 0; k < 8; k++)
            {
                word address = ((offset + k * 0x0800 + j * xSize * 2 + i) % 0x4000);
                BYTE b = CPC::RAM[bank][address];
                int line = j * 8 + k;
                for (int l = 0; l < 8; l++)
                {
                    BYTE pen = CPC::gateArray.GetPenForPixel(mode, b, l);
                    const BYTE *color = paletteTable[pen & 0x0F];
                    long pixelBase = (line * xSize * 16 * 2 + i * 8 + l) * 3;
                    if (pixelBase < byteSize - 2)
                        for (int m = 0; m < 3; m++)
                        {
                            pixels[pixelBase + m] = color[m];
                            pixels[pixelBase + m + xSize * 16 * 3] = color[m];
                        }
                }
            }
    if (pixItem) { scene->removeItem(pixItem); delete pixItem; }
    QImage image(&pixels[0], xSize * 16, ySize * 16, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image, Qt::NoFormatConversion | Qt::NoOpaqueDetection);
    pixItem = scene->addPixmap(pixmap);
    pixItem->setPos(0, 0);
    scene->setSceneRect(0, 0, pixmap.width(), pixmap.height());
    const int fw = 2 * ui->graphicsView->frameWidth();
    ui->graphicsView->setFixedSize(pixmap.width() + fw, pixmap.height() + fw);

    QRect content = ui->graphicsView->geometry()
                        .united(ui->groupBox->geometry())
                        .united(ui->groupBoxSize->geometry());
    const int margin = 10;
    setFixedSize(content.right() + margin, content.bottom() + margin);
}

