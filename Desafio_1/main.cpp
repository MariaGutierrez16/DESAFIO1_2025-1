#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;

unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
unsigned char* xorImages(unsigned char* img1, unsigned char* img2, int totalBytes);
void rotateRight(unsigned char* data, int totalBytes, int bits);
void rotateLeft(unsigned char* data, int totalBytes, int bits);

int main()
{
    QString archivoEntrada = "I_O.bmp";
    QString archivoSalida = "I_D.bmp";

    int height = 0, width = 0;

    unsigned char *pixelData = loadPixels(archivoEntrada, width, height);
    if (pixelData == nullptr) {
        cout << "No se pudo cargar la imagen original." << endl;
        return -1;
    }

    // Cargar imagen de distorsión
    QString archivoDistorsion = "I_M.bmp";
    int width2 = 0, height2 = 0;
    unsigned char* distortionData = loadPixels(archivoDistorsion, width2, height2);

    if (width != width2 || height != height2 || distortionData == nullptr) {
        cout << "Las imágenes no tienen las mismas dimensiones o I_M.bmp no se pudo cargar." << endl;
        delete[] distortionData;
        delete[] pixelData;
        return -1;
    }

    int totalBytes = width * height * 3;
    unsigned char* xorResult = xorImages(pixelData, distortionData, totalBytes);

    exportImage(xorResult, width, height, "XOR.bmp");

    // Rotación de 3 bits a la derecha
    rotateRight(xorResult, totalBytes, 3);
    exportImage(xorResult, width, height, "XOR_Rotada.bmp");

    // Ahora la rotación inversa: 3 bits a la izquierda
    rotateLeft(xorResult, totalBytes, 3);
    exportImage(xorResult, width, height, "XOR_Revertida.bmp");

    delete[] distortionData;
    delete[] xorResult;
    delete[] pixelData;

    return 0;
}

unsigned char* loadPixels(QString input, int &width, int &height){
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

bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida){
    QImage outputImage(width, height, QImage::Format_RGB888);

    for (int y = 0; y < height; ++y) {
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    if (!outputImage.save(archivoSalida, "BMP")) {
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false;
    } else {
        cout << "Imagen guardada como " << archivoSalida.toStdString() << endl;
        return true;
    }
}

unsigned char* xorImages(unsigned char* img1, unsigned char* img2, int totalBytes) {
    if (img1 == nullptr || img2 == nullptr) {
        cout << "Error: una o ambas imágenes son nulas." << endl;
        return nullptr;
    }

    unsigned char* result = new unsigned char[totalBytes];
    for (int i = 0; i < totalBytes; ++i) {
        result[i] = img1[i] ^ img2[i];
    }

    return result;
}

void rotateRight(unsigned char* data, int totalBytes, int bits) {
    for (int i = 0; i < totalBytes; ++i) {
        data[i] = (data[i] >> bits) | (data[i] << (8 - bits));
    }
}

void rotateLeft(unsigned char* data, int totalBytes, int bits) {
    for (int i = 0; i < totalBytes; ++i) {
        data[i] = (data[i] << bits) | (data[i] >> (8 - bits));
    }
}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels){
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
