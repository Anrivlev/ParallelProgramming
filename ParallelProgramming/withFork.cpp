// Ivlev Andrey B19 - 511
// Variant generation:
const int ID = 4; // Student ID
const int G = 511; // Group
const int X = G * 2 + ID; // X = 1026
const int A = X % 4; // A = 2
const int B = 5 + X % 5; // B = 6
volatile long totalNumberOfPixels = 0;

#include <iostream>
#include <thread>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <atomic>
#include <filesystem>

int isSuitable(int R, int G, int B)
{
    if (R * G * B < 1000) return 1;
    else return 0;
}

int main()
{
    // BMP file reading
    const char* filename = (char*) "ParallelProgramming/images/img01.bmp";

    int i;
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];

    // read the 54-byte header
    fread(info, sizeof(unsigned char), 54, f);

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    std::cout << "widht: " << width << std::endl;
    std::cout << "height: " << height << std::endl;

    // allocate 3 bytes per pixel
    int size = 3 * width * height;
    unsigned char* data = new unsigned char[size];

    // read the rest of the data at once
    fread(data, sizeof(unsigned char), size, f);
    fclose(f);
    for (i = 0; i < size; i += 3)
    {
        // flip the order of every 3 bytes in order to get RGB instead of BGR
        unsigned char tmp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = tmp;
    }
    // data is read

    int th = -1;
    pid_t c_pid;
    for (int i = 0; i < B; i++)
    {
        c_pid = fork();
        th = i;
        break;
    }
    if (c_pid == 0)
    {
        int numberOfPixels = 0;
        for (int j = 3*th; j < size; j += 3 * B)
        {
            numberOfPixels += isSuitable((int)data[j], (int)data[j + 1], (int)data[j + 2]);
        }
        std::ofstream out;
        char* filename_out = new char[50];
        sprintf( filename_out, "ParallelProgramming/infoFork/img%d.txt", th);
        out.open(filename_out);
        out << numberOfPixels << std::endl;
        out.close();
        exit(EXIT_SUCCESS);
    }
    else
    {
        for (int i = 0; i < B; i++)
        {
            wait(nullptr);
        }
    }
    std::cout << "Thread results: [";
    for (int i = 0; i < B; i++)
    {
        std::string line;
        char* filename_in = new char[50];
        sprintf( filename_in, "ParallelProgramming/infoFork/img%d.txt", i);

        std::ifstream in(filename_in);
        if (in.is_open())
        {
            getline(in, line);
            std::cout << line;
            if (i != B - 1)  std::cout << " ";
            totalNumberOfPixels += std::stoi(line);
        }
        in.close();
    }
    std::cout << "]" << std::endl;

    std::cout << "Total number of suitable pixels (with fork): " << totalNumberOfPixels << std::endl;
    std::cout << std::endl;
    return 0;
}
