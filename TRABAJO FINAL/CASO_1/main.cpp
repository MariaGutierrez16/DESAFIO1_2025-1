#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

using namespace std;

// ---------------- FUNCIONES ----------------

// Carga los píxeles de una imagen BMP y los convierte al formato RGB888.
// Devuelve un arreglo dinámico con los datos de la imagen.
unsigned char* loadPixels(QString input, int &width, int &height) {
    QImage imagen(input);
    if (imagen.isNull()) {
        cout << "Error al cargar la imagen: " << input.toStdString() << endl;
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

// Aplica una operación XOR entre dos imágenes del mismo tamaño.
unsigned char* xorImages(unsigned char* img1Data, unsigned char* img2Data, int size) {
    unsigned char* result = new unsigned char[size];
    for (int i = 0; i < size; ++i) {
        result[i] = img1Data[i] ^ img2Data[i];
    }
    return result;
}

// Aplica rotación de bits hacia la izquierda o derecha a cada byte.
void rotateBits(unsigned char* data, int size, int bits, bool left) {
    for (int i = 0; i < size; ++i) {
        if (left)
            data[i] = (data[i] << bits) | (data[i] >> (8 - bits));
        else
            data[i] = (data[i] >> bits) | (data[i] << (8 - bits));
    }
}

// Aplica desplazamiento (shift) a izquierda o derecha a cada byte.
void shiftBits(unsigned char* data, int size, int bits, bool left) {
    for (int i = 0; i < size; ++i) {
        if (left)
            data[i] <<= bits;
        else
            data[i] >>= bits;
    }
}

// Lee la semilla (posición inicial del enmascaramiento) desde un archivo de suma.
int readSeedFromFile(const string& filename) {
    ifstream inFile(filename);
    int seed = -1;
    if (inFile.is_open()) {
        inFile >> seed;
        inFile.close();
    } else {
        cout << "Error al leer la semilla desde " << filename << endl;
    }
    return seed;
}

// Lee los valores RGB enmascarados esperados desde el archivo correspondiente.
vector<int> readMaskedValues(const string& filename) {
    ifstream inFile(filename);
    vector<int> valores;
    int temp;
    if (inFile.is_open()) {
        inFile >> temp; // Saltar la semilla
        while (inFile >> temp) {
            valores.push_back(temp);
        }
        inFile.close();
    } else {
        cout << "Error al abrir " << filename << " para leer los valores enmascarados." << endl;
    }
    return valores;
}

// Compara los valores obtenidos con la máscara contra los esperados.
// Retorna verdadero si hay una coincidencia aceptable (<= 5 diferencias).
bool verificarTransformacion(unsigned char* imagen, unsigned char* mascara, const vector<int>& valoresEsperados, int seed, int dataSize) {
    int diferencias = 0;
    for (int i = 0; i < valoresEsperados.size(); ++i) {
        if (seed + i >= dataSize) break;
        unsigned short suma = imagen[seed + i] + mascara[i];
        if (suma != valoresEsperados[i]) diferencias++;
        if (diferencias > 5) return false;
    }
    return true;
}

// Aplica una operación específica a una imagen base.
// Si es XOR, se aplica con la imagen IM; si es rotación o desplazamiento, se realiza sobre los bytes.
unsigned char* aplicarOperacion(const string& tipo, unsigned char* base, unsigned char* IM, int size, int param = 0) {
    unsigned char* copia = new unsigned char[size];
    memcpy(copia, base, size);

    if (tipo == "XOR") {
        unsigned char* resultado = xorImages(copia, IM, size);
        delete[] copia;
        return resultado;
    } else if (tipo == "RL") {
        rotateBits(copia, size, param, true);
    } else if (tipo == "RR") {
        rotateBits(copia, size, param, false);
    } else if (tipo == "SL") {
        shiftBits(copia, size, param, true);
    } else if (tipo == "SR") {
        shiftBits(copia, size, param, false);
    }

    return copia;
}

// Guarda los datos de píxeles como una imagen BMP en la ruta indicada.
void guardarImagenBMP(unsigned char* pixelData, int width, int height, const QString& rutaSalida) {
    QImage imagen(pixelData, width, height, QImage::Format_RGB888);
    if (imagen.save(rutaSalida)) {
        cout << "Imagen reconstruida guardada como: " << rutaSalida.toStdString() << endl;
    } else {
        cout << "Error al guardar la imagen reconstruida." << endl;
    }
}

// Aplica las transformaciones en orden inverso a la imagen final hasta recuperar la imagen original.
// Al finalizar, guarda la imagen recuperada como BMP.
void buscarTransformaciones(QString imagenFinalPath, QString mascaraPath, QString IMPath, const vector<string>& archivosTXT, const QString& rutaSalida) {
    int width, height;
    int size;

    // Cargar imágenes necesarias
    unsigned char* imagenActual = loadPixels(imagenFinalPath, width, height);
    unsigned char* mascara = loadPixels(mascaraPath, width, height);
    unsigned char* IM = loadPixels(IMPath, width, height);
    size = width * height * 3;

    // Invertir el orden de los archivos para aplicar transformaciones inversas
    vector<string> archivosTXTReverso = archivosTXT;
    reverse(archivosTXTReverso.begin(), archivosTXTReverso.end());

    vector<string> operaciones = {"XOR", "RL", "RR", "SL", "SR"};

    // Evaluar cada transformación posible para cada etapa
    for (int etapa = 0; etapa < archivosTXTReverso.size(); etapa++) {
        string archivo = archivosTXTReverso[etapa];
        int semilla = readSeedFromFile(archivo);
        vector<int> valores = readMaskedValues(archivo);

        bool encontrado = false;

        for (const string& op : operaciones) {
            if (op == "XOR") {
                unsigned char* resultado = aplicarOperacion(op, imagenActual, IM, size);
                if (verificarTransformacion(resultado, mascara, valores, semilla, size)) {
                    cout << "Etapa " << archivosTXT.size() - etapa << ": " << op << endl;
                    delete[] imagenActual;
                    imagenActual = resultado;
                    encontrado = true;
                    break;
                }
                delete[] resultado;
            } else {
                for (int b = 2; b <= 8; ++b) {
                    unsigned char* resultado = aplicarOperacion(op, imagenActual, IM, size, b);
                    if (verificarTransformacion(resultado, mascara, valores, semilla, size)) {
                        cout << "Etapa " << archivosTXT.size() - etapa << ": " << op << " " << b << " bits" << endl;
                        delete[] imagenActual;
                        imagenActual = resultado;
                        encontrado = true;
                        break;
                    }
                    delete[] resultado;
                }
            }
            if (encontrado) break;
        }

        if (!encontrado) {
            cout << "No se pudo identificar la transformación en la etapa " << archivosTXT.size() - etapa << endl;
        }
    }

    // Guardar la imagen recuperada como BMP
    guardarImagenBMP(imagenActual, width, height, rutaSalida);

    delete[] imagenActual;
    delete[] mascara;
    delete[] IM;
}

// ---------------- MAIN ----------------

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Rutas de archivos de entrada y salida
    QString imagenFinal = "C:/DESAFIO1_2025-1/TRABAJO FINAL/CASO_1/I_D.bmp";
    QString mascara     = "C:/DESAFIO1_2025-1/TRABAJO FINAL/CASO_1/M.bmp";
    QString IM          = "C:/DESAFIO1_2025-1/TRABAJO FINAL/CASO_1/I_M.bmp";
    QString salida      ="C:/DESAFIO1_2025-1/TRABAJO FINAL/CASO_1/I_ORIGINAL_RECUPERADA.bmp";

    vector<string> archivosTXT = {
        "C:/DESAFIO1_2025-1/TRABAJO FINAL/CASO_1/M0.txt",
        "C:/DESAFIO1_2025-1/TRABAJO FINAL/CASO_1/M1.txt",
        "C:/DESAFIO1_2025-1/TRABAJO FINAL/CASO_1/M2.txt"
    };

    // Ejecutar el proceso de búsqueda e inversión de transformaciones
    buscarTransformaciones(imagenFinal, mascara, IM, archivosTXT, salida);

    return 0;
}
