#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;

unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
unsigned char* xorImages(unsigned char* img1, unsigned char* img2, int totalBytes);

int main()
{
    QString archivoOriginal = "I_O.BMP";
    QString archivoModificada = "I_M.BMP";
    QString archivoSalida = "I_D.BMP";

    int width = 0, height = 0;
    unsigned char* originalData = loadPixels(archivoOriginal, width, height);
    if (originalData == nullptr) {
        cout << "No se pudo cargar la imagen original." << endl;
        return -1;
    }

    int width2 = 0, height2 = 0;
    unsigned char* modifiedData = loadPixels(archivoModificada, width2, height2);

    if (width != width2 || height != height2 || modifiedData == nullptr) {
        cout << "Las imágenes no tienen las mismas dimensiones o I_M.BMP no se pudo cargar." << endl;
        delete[] modifiedData;
        delete[] originalData;
        return -1;
    }

    int totalBytes = width * height * 3;
    unsigned char* maskData = xorImages(originalData, modifiedData, totalBytes);
    exportImage(maskData, width, height, "M.BMP");

    exportImage(maskData, width, height, "M_Rotada.BMP");

    delete[] modifiedData;
    delete[] maskData;

    // Simulación de modificación RGB artificial (puede comentarse si no se desea)
    for (int i = 0; i < width * height * 3; i += 3) {
        originalData[i] = i % 256;
        originalData[i + 1] = i % 256;
        originalData[i + 2] = i % 256;
    }

    bool exportI = exportImage(originalData, width, height, archivoSalida);
    cout << exportI << endl;

    delete[] originalData;
    originalData = nullptr;

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
    maskingData = nullptr;

    return 0;
}

unsigned char* loadPixels(QString input, int &width, int &height) {
    QImage imagen(input);
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << std::endl;
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

    if (!outputImage.save(archivoSalida, "BMP")) {
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false;
    } else {
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true;
    }
}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo." << endl;
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
        cout << "Error al reabrir el archivo." << endl;
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
    cout << "Semilla: " << seed << endl;
    cout << "Cantidad de píxeles leídos: " << n_pixels << endl;

    return RGB;
}

unsigned char* xorImages(unsigned char* img1, unsigned char* img2, int totalBytes) {
    if (img1 == nullptr || img2 == nullptr) {
        std::cout << "Error: una o ambas imágenes son nulas." << std::endl;
        return nullptr;
    }

    unsigned char* result = new unsigned char[totalBytes];
    for (int i = 0; i < totalBytes; ++i) {
        result[i] = img1[i] ^ img2[i];
    }

    return result;
}
