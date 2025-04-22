#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;

unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
unsigned char* xorImages(unsigned char* img1, unsigned char* img2, int totalBytes);
void rotateRightBits(unsigned char* data, int totalBytes, int bits);

int main() {
    QString archivoEntrada = "I_O.bmp";
    QString archivoDistorsion = "I_M.bmp";
    QString archivoSalida = "I_D.bmp";

    int width = 0, height = 0;
    unsigned char *pixelData = loadPixels(archivoEntrada, width, height);
    if (pixelData == nullptr) {
        cout << "No se pudo cargar la imagen I_O.bmp." << endl;
        return -1;
    }

    int width2 = 0, height2 = 0;
    unsigned char *distortionData = loadPixels(archivoDistorsion, width2, height2);
    if (distortionData == nullptr || width != width2 || height != height2) {
        cout << "No se pudo cargar la imagen I_M.bmp o tiene dimensiones distintas." << endl;
        delete[] pixelData;
        return -1;
    }

    int totalBytes = width * height * 3;
    unsigned char* xorResult = xorImages(pixelData, distortionData, totalBytes);
    exportImage(xorResult, width, height, "XOR.bmp");

    rotateRightBits(xorResult, totalBytes, 3);
    exportImage(xorResult, width, height, "XOR_Rotada.bmp");

    delete[] pixelData;
    delete[] distortionData;
    delete[] xorResult;

    int seed = 0;
    int n_pixels = 0;
    unsigned int* maskingData = loadSeedMasking("M1.txt", seed, n_pixels);
    if (maskingData == nullptr) {
        cout << "No se pudieron cargar los datos de enmascaramiento." << endl;
        return -1;
    }

    for (int i = 0; i < n_pixels * 3; i += 3) {
        cout << "Pixel " << i / 3 << ": ("
             << maskingData[i] << ", "
             << maskingData[i + 1] << ", "
             << maskingData[i + 2] << ")" << endl;
    }

    delete[] maskingData;

    return 0;
}

unsigned char* loadPixels(QString input, int &width, int &height) {
    QImage imagen(input);
    if (imagen.isNull()) {
        return nullptr;
    }

    imagen = imagen.convertToFormat(QImage::Format_RGB888);
    width = imagen.width();
    height = imagen.height();
    int dataSize = width * height * 3;
    unsigned char* pixelData = new unsigned char[dataSize];

    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);
        unsigned char* dstLine = pixelData + y * width * 3;
        memcpy(dstLine, srcLine, width * 3);
    }

    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida) {
    QImage outputImage(width, height, QImage::Format_RGB888);

    for (int y = 0; y < height; ++y) {
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    return outputImage.save(archivoSalida, "BMP");
}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        return nullptr;
    }

    archivo >> seed;
    int r, g, b;
    while (archivo >> r >> g >> b) {
        n_pixels++;
    }
    archivo.close();

    archivo.open(nombreArchivo);
    if (!archivo.is_open()) {
        return nullptr;
    }

    unsigned int* RGB = new unsigned int[n_pixels * 3];
    archivo >> seed;

    for (int i = 0; i < n_pixels * 3; i += 3) {
        archivo >> r >> g >> b;
        RGB[i] = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }

    archivo.close();
    return RGB;
}

unsigned char* xorImages(unsigned char* img1, unsigned char* img2, int totalBytes) {
    if (img1 == nullptr || img2 == nullptr) {
        return nullptr;
    }

    unsigned char* result = new unsigned char[totalBytes];
    for (int i = 0; i < totalBytes; ++i) {
        result[i] = img1[i] ^ img2[i];
    }

    return result;
}

void rotateRightBits(unsigned char* data, int totalBytes, int bits) {
    for (int i = 0; i < totalBytes; ++i) {
        unsigned char byte = data[i];
        data[i] = (byte >> bits) | (byte << (8 - bits));
    }
}
