#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;

unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
unsigned char* xorImages(unsigned char* img1, unsigned char* img2, int totalBytes);
unsigned char* rotateBits(unsigned char* imageData, int totalBytes, int bits);

int main()
{
    QString archivoEntrada = "I_M.bmp";  // Imagen original
    QString archivoSalida = "I_D.bmp";   // Imagen modificada

    int height = 0;
    int width = 0;

    unsigned char *pixelData = loadPixels(archivoEntrada, width, height);
    if (pixelData == nullptr) {
        cout << "No se pudo cargar la imagen original." << endl;
        return -1;
    }

    // Cargar segunda imagen: imagen de distorsión (IM.bmp)
    QString archivoDistorsion = "IM.bmp";
    int width2 = 0, height2 = 0;
    unsigned char* distortionData = loadPixels(archivoDistorsion, width2, height2);

    if (width != width2 || height != height2 || distortionData == nullptr) {
        cout << "Las imágenes no tienen las mismas dimensiones o IM.bmp no se pudo cargar." << endl;
        delete[] distortionData;
        delete[] pixelData;
        return -1;
    }

    int totalBytes = width * height * 3;
    unsigned char* xorResult = xorImages(pixelData, distortionData, totalBytes);
    exportImage(xorResult, width, height, "XOR.bmp");

    // Aplicar rotación de bits a la derecha (8 bits)
    unsigned char* rotatedResult = rotateBits(xorResult, totalBytes, 8);  // Rotar 8 bits
    exportImage(rotatedResult, width, height, "XOR_Rotada_8bits.bmp");

    delete[] distortionData;
    delete[] xorResult;
    delete[] rotatedResult;

    // Cargar y procesar los datos de enmascarado de M1.txt
    int seed = 0;
    int n_pixels = 0;
    unsigned int *maskingData = loadSeedMasking("M1.txt", seed, n_pixels);
    if (maskingData == nullptr) {
        cout << "No se pudieron cargar los datos de enmascaramiento." << endl;
        return -1;
    }

    // Simulación de visualización de datos de enmascarado
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

bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida){
    QImage outputImage(width, height, QImage::Format_RGB888);

    for (int y = 0; y < height; ++y) {
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    if (!outputImage.save(archivoSalida, "BMP")) {
        cout << "Error: No se pudo guardar la imagen BMP modificada." << endl;
        return false;
    } else {
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true;
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

unsigned char* rotateBits(unsigned char* imageData, int totalBytes, int bits) {
    unsigned char* rotatedData = new unsigned char[totalBytes];
    for (int i = 0; i < totalBytes; ++i) {
        rotatedData[i] = (imageData[i] >> bits) | (imageData[i] << (8 - bits));
    }
    return rotatedData;
}
