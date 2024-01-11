#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <unistd.h>

using namespace std;

constexpr size_t CHUNK_SIZE = 500 * 1024 * 1024; // Размер чанка в байтах (500 МБ)

// Функция для генерации имени временного файла
string generateTempFilename() {
    char tempFileName[] = "/tmp/tempfileXXXXXX"; // Шаблон имени временного файла
    int fd = mkstemp(tempFileName); // Создание уникального временного файла

    if (fd != -1) {
        close(fd);
        return string(tempFileName);
    } else {
        cerr << "Failed to generate temporary filename" << endl;
        return "";
    }
}

// Функция для сортировки и записи чанка строк во временный файл
void sortAndWriteChunk(const vector<string>& chunk, const string& tempFileName) {
    vector<string> sortedChunk = chunk;
    sort(sortedChunk.begin(), sortedChunk.end()); // Сортировка чанка

    ofstream tempFile(tempFileName);
    if (tempFile.is_open()) {
        for (const auto& line : sortedChunk) {
            tempFile << line << endl; // Запись отсортированных строк во временный файл
        }
        tempFile.close();
    }
}

// Функция для слияния отсортированных файлов
void mergeSortedFiles(const vector<string>& sortedFiles, const string& outputFileName) {
    vector<ifstream> inputFiles;
    vector<string> lines;

    // Открываем все файлы для чтения
    for (const auto& file : sortedFiles) {
        inputFiles.emplace_back(file);
    }

    // Читаем первую строку из каждого файла
    for (auto& file : inputFiles) {
        string line;
        if (getline(file, line)) {
            lines.push_back(line);
        }
    }

    // Открываем выходной файл
    ofstream outputFile(outputFileName);

    // Производим слияние отсортированных данных
    while (!lines.empty()) {
        auto minPos = min_element(lines.begin(), lines.end()); // Находим минимальный элемент
        outputFile << *minPos << endl; // Записываем минимальный элемент в выходной файл

        // Получаем индекс минимального элемента
        size_t index = minPos - lines.begin();

        // Читаем следующую строку из файла с минимальным элементом
        string nextLine;
        if (getline(inputFiles[index], nextLine)) {
            lines[index] = nextLine;
        } else {
            // Файл закончился, удаляем его из списка файлов
            lines.erase(lines.begin() + index);
            inputFiles[index].close();
            inputFiles.erase(inputFiles.begin() + index);
        }
    }

    outputFile.close();

    // Закрываем все открытые файлы
    for (auto& file : inputFiles) {
        file.close();
    }
}

// Основная функция внешней сортировки
void externalSort(const string& inputFileName, const string& outputFileName) {
    ifstream inputFile(inputFileName); // Открываем файл для чтения
    if (!inputFile.is_open()) {
        cerr << "Unable to open input file" << endl;
        return;
    }

    vector<string> chunk;
    string line;
    size_t currentSize = 0;
    vector<string> sortedFiles;

    // Читаем файл построчно и разбиваем его на чанки
    while (getline(inputFile, line)) {
        // Если размер чанка превышает CHUNK_SIZE, сортируем и записываем его во временный файл
        if (currentSize + line.size() > CHUNK_SIZE) {
            string tempFileName = generateTempFilename();
            sortAndWriteChunk(chunk, tempFileName);
            sortedFiles.push_back(tempFileName);
            chunk.clear();
            currentSize = 0;
        }

        chunk.push_back(line);
        currentSize += line.size();
    }

    inputFile.close();

    // Если остались строки в последнем чанке, сортируем и записываем их в файл
    if (!chunk.empty()) {
        string tempFileName = generateTempFilename();
        sortAndWriteChunk(chunk, tempFileName);
        sortedFiles.push_back(tempFileName);
    }

    // Выполняем слияние отсортированных файлов
    mergeSortedFiles(sortedFiles, outputFileName);

    // Удаляем временные файлы
    for (const auto& file : sortedFiles) {
        remove(file.c_str());
    }
}

int main() {
    string inputFileName = "input_file.txt"; // Путь к исходному файлу
    string outputFileName = "sorted_file.txt"; // Путь к выходному файлу

    externalSort(inputFileName, outputFileName); // Вызов функции внешней сортировки

    cout << "File has been sorted." << endl;

    return 0;
}