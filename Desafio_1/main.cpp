#include <QCoreApplication>
#include <QImage>
#include <QString>
#include <fstream>
#include <iostream>

using namespace std;

unsigned char* loadPixels(QString input, int &width, int &height) {
    QImage image(input);
    if (image.isNull()) return nullptr;
    QImage img = image.convertToFormat(QImage::Format_RGB888);
    width = img.width();
    height = img.height();
    int size = width * height * 3;
    unsigned char* pixels = new unsigned char[size];
    memcpy(pixels, img.bits(), size);
    return pixels;
}

bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida) {
    QImage image(pixelData, width, height, QImage::Format_RGB888);
    return image.save(archivoSalida);
}

bool validarConArchivoDeSuma(const unsigned char* imagen, const unsigned char* mascara, int anchoImg, int altoImg, const QString& archivoSuma, int anchoMascara, int altoMascara) {
    ifstream archivo(archivoSuma.toStdString());
    if (!archivo.is_open()) return false;
    int semilla;
    archivo >> semilla;
    int totalPixelesMascara = anchoMascara * altoMascara * 3;
    int limite = anchoImg * altoImg * 3;
    if (semilla + totalPixelesMascara > limite) return false;
    for (int i = 0; i < totalPixelesMascara; i += 3) {
        int r_sum, g_sum, b_sum;
        if (!(archivo >> r_sum >> g_sum >> b_sum)) return false;
        int idx = semilla + i;
        int r_calc = imagen[idx] + mascara[i];
        int g_calc = imagen[idx + 1] + mascara[i + 1];
        int b_calc = imagen[idx + 2] + mascara[i + 2];
        if (r_calc != r_sum || g_calc != g_sum || b_calc != b_sum) return false;
    }
    return true;
}

void aplicarXOR(unsigned char* out, const unsigned char* img1, const unsigned char* img2, int size) {
    for (int i = 0; i < size; i++) out[i] = img1[i] ^ img2[i];
}

void desplazamientoIzquierda(unsigned char* out, const unsigned char* in, int size, int bits) {
    for (int i = 0; i < size; i++) out[i] = (in[i] << bits) & 0xFF;
}

void desplazamientoDerecha(unsigned char* out, const unsigned char* in, int size, int bits) {
    for (int i = 0; i < size; i++) out[i] = (in[i] >> bits);
}

void rotacionIzquierda(unsigned char* out, const unsigned char* in, int size, int bits) {
    for (int i = 0; i < size; i++) out[i] = ((in[i] << bits) | (in[i] >> (8 - bits))) & 0xFF;
}

void rotacionDerecha(unsigned char* out, const unsigned char* in, int size, int bits) {
    for (int i = 0; i < size; i++) out[i] = ((in[i] >> bits) | (in[i] << (8 - bits))) & 0xFF;
}

void generarArchivoSuma(const unsigned char* imagen, const unsigned char* mascara, int anchoImg, int altoImg, int anchoMascara, int altoMascara, int semilla, const QString& rutaSalida) {
    ofstream archivo(rutaSalida.toStdString());
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo para escritura." << endl;
        return;
    }

    archivo << semilla << endl;
    int total = anchoMascara * altoMascara * 3;
    int limite = anchoImg * altoImg * 3;
    if (semilla + total > limite) {
        cout << "Error: la semilla + mascara excede el tamano de la imagen." << endl;
        return;
    }

    for (int i = 0; i < total; i += 3) {
        int idx = semilla + i;
        int r = imagen[idx] + mascara[i];
        int g = imagen[idx + 1] + mascara[i + 1];
        int b = imagen[idx + 2] + mascara[i + 2];
        archivo << r << " " << g << " " << b << endl;
    }

    archivo.close();
    cout << "Archivo generado exitosamente en: " << rutaSalida.toStdString() << endl;
}

bool detectarOperacion(unsigned char* imagenActual, const unsigned char* referencia, const unsigned char* mascara, int width, int height, int widthM, int heightM, const QString& archivoSuma, QString& operacionDetectada) {
    int size = width * height * 3;
    unsigned char* transformada = new unsigned char[size];

    aplicarXOR(transformada, imagenActual, referencia, size);
    if (validarConArchivoDeSuma(transformada, mascara, width, height, archivoSuma, widthM, heightM)) {
        memcpy(imagenActual, transformada, size);
        operacionDetectada = "XOR";
        delete[] transformada;
        return true;
    }

    for (int b = 2; b <= 8; b++) {
        desplazamientoIzquierda(transformada, imagenActual, size, b);
        if (validarConArchivoDeSuma(transformada, mascara, width, height, archivoSuma, widthM, heightM)) {
            memcpy(imagenActual, transformada, size);
            operacionDetectada = "desplazamiento izquierda " + QString::number(b) + " bits";
            delete[] transformada;
            return true;
        }
        desplazamientoDerecha(transformada, imagenActual, size, b);
        if (validarConArchivoDeSuma(transformada, mascara, width, height, archivoSuma, widthM, heightM)) {
            memcpy(imagenActual, transformada, size);
            operacionDetectada = "desplazamiento derecha " + QString::number(b) + " bits";
            delete[] transformada;
            return true;
        }
        rotacionIzquierda(transformada, imagenActual, size, b);
        if (validarConArchivoDeSuma(transformada, mascara, width, height, archivoSuma, widthM, heightM)) {
            memcpy(imagenActual, transformada, size);
            operacionDetectada = "rotacion izquierda " + QString::number(b) + " bits";
            delete[] transformada;
            return true;
        }
        rotacionDerecha(transformada, imagenActual, size, b);
        if (validarConArchivoDeSuma(transformada, mascara, width, height, archivoSuma, widthM, heightM)) {
            memcpy(imagenActual, transformada, size);
            operacionDetectada = "rotacion derecha " + QString::number(b) + " bits";
            delete[] transformada;
            return true;
        }
    }

    delete[] transformada;
    return false;
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QString basePath = "C:/Users/gutierrezm/Documents/DESAFIO1_2025-1/Desafio_1/build/Desktop_Qt_6_9_0_MinGW_64_bit-Debug/";
    QString archivoEntrada1 = basePath + "I_O.bmp";
    QString archivoEntrada2 = basePath + "I_M.bmp";
    QString archivoMascara  = basePath + "M.bmp";
    QString archivoM2       = basePath + "M2.txt";
    QString archivoM1       = basePath + "M1.txt";
    QString archivoSalida   = basePath + "reconstruida.bmp";

    int width1, height1, width2, height2, widthM, heightM;

    unsigned char* img1 = loadPixels(archivoEntrada1, width1, height1);
    unsigned char* img2 = loadPixels(archivoEntrada2, width2, height2);
    unsigned char* mascara = loadPixels(archivoMascara, widthM, heightM);
    if (!img1 || !img2 || !mascara) return -1;

    int size = width1 * height1 * 3;

    // Paso 1: rotación derecha a partir de img1
    unsigned char* imagenTransformada = new unsigned char[size];
    rotacionDerecha(imagenTransformada, img1, size, 3);
    int semillaM2 = 15;
    QString archivoM2Generado = basePath + "M2_generado.txt";
    generarArchivoSuma(imagenTransformada, mascara, width1, height1, widthM, heightM, semillaM2, archivoM2Generado);

    // Paso 2: aplicar XOR con img2 sobre imagenTransformada
    unsigned char* imagenTransformada2 = new unsigned char[size];
    aplicarXOR(imagenTransformada2, imagenTransformada, img2, size);
    int semillaM1 = 30;
    QString archivoM1Generado = basePath + "M1_generado.txt";
    generarArchivoSuma(imagenTransformada2, mascara, width1, height1, widthM, heightM, semillaM1, archivoM1Generado);

    // Proceso de detección
    unsigned char* imagenActual = new unsigned char[size];
    memcpy(imagenActual, img1, size);

    QString opM2, opM1;

    if (detectarOperacion(imagenActual, imagenTransformada, mascara, width1, height1, widthM, heightM, archivoM2Generado, opM2)) {
        cout << "Operacion detectada para M2: " << opM2.toStdString() << endl;

        if (detectarOperacion(imagenActual, img2, mascara, width1, height1, widthM, heightM, archivoM1Generado, opM1)) {
            cout << "Operacion detectada para M1: " << opM1.toStdString() << endl;

            exportImage(imagenActual, width1, height1, archivoSalida);
            cout << "La imagen ID.bmp fue modificada." << endl;
            cout << "Imagen reconstruida exportada exitosamente." << endl;

            ifstream archivoSemilla(archivoM1Generado.toStdString());
            if (archivoSemilla.is_open()) {
                int semilla;
                archivoSemilla >> semilla;
                cout << "Semilla M1: " << semilla << endl;
                int r, g, b;
                int contador = 0;
                while (archivoSemilla >> r >> g >> b && contador < 5) {
                    cout << "RGB suma " << contador << ": (" << r << ", " << g << ", " << b << ")" << endl;
                    contador++;
                }
            }
        } else {
            cout << "No se detecto operacion valida para M1." << endl;
        }
    } else {
        cout << "No se detecto operacion valida para M2." << endl;
    }

    delete[] img1;
    delete[] img2;
    delete[] mascara;
    delete[] imagenActual;
    delete[] imagenTransformada;
    delete[] imagenTransformada2;

    return 0;
}
