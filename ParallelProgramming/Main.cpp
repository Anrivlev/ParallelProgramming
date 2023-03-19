// Ivlev Andrey B19 - 511
// Variant generation:
const int ID = 4; // Student ID
const int G = 511; // Group
const int X = G * 2 + ID; // X = 1026
const int A = X % 4; // A = 2
const int B = 5 + X % 5; // B = 6
volatile long totalNumberOfPixels = 0;

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <string>

int isSuitable(int R, int G, int B)
{
    if (R * G * B < 1000) return 1;
    else return 0;
}

int bruteForce(unsigned char* data, int size)
{
    int numberOfPixels = 0;
    for (int i = 0; i < size; i += 3)
    {
        numberOfPixels += isSuitable((int)data[i], (int)data[i+1], (int)data[i+2]);
    }
    return numberOfPixels;
}

struct TASK
{
    int start;
    int end;
    unsigned char* data;
    int numberOfPixels;
};

DWORD __stdcall countSuitablePixels(void* arg)
{
    TASK* task = (TASK*)arg;
    int numberOfPixels = 0;
    for (int i = task->start; i < task->end; i += 3 * B)
    {
        numberOfPixels += isSuitable((int)task->data[i], (int)task->data[i + 1], (int)task->data[i + 2]);
    }
    InterlockedAdd(&totalNumberOfPixels, numberOfPixels);
    task->numberOfPixels = numberOfPixels;
    return 0;
}

int withCreateThread(unsigned char* data, int size)
{
    int numberOfPixels = 0;
    HANDLE threadHandles[B];
    DWORD th_id[B];
    TASK task[B];
    for (int i = 0; i < B; i++)
    {
        task[i].start = 3 * i;
        task[i].end = size;
        task[i].data = data;
        task[i].numberOfPixels = 0;
        threadHandles[i] = CreateThread(NULL, 0, countSuitablePixels, &task[i], 0, &th_id[i]);
    }
    WaitForMultipleObjects(B, threadHandles, true, INFINITE);
    std::cout << "Thread results: [";
    for (int i = 0; i < B; i++)
    {
        std::cout << task[i].numberOfPixels;
        if (i != B-1)  std::cout << " ";
    }
    std::cout << "]" << std::endl;
    return totalNumberOfPixels;
}

int withSTDThread(unsigned char* data, int size)
{
    int numberOfPixels = 0;
    TASK task[B];
    std::vector<std::thread> my_threads;
    for (int i = 0; i < B; i++)
    {
        task[i].start = 3 * i;
        task[i].end = size;
        task[i].data = data;
        task[i].numberOfPixels = 0;
        my_threads.push_back(std::thread(countSuitablePixels, &task[i]));
    }
    auto original_thread = my_threads.begin();
    while (original_thread != my_threads.end())
    {
        original_thread->join();
        original_thread++;
    }
    std::cout << "Thread results: [";
    for (int i = 0; i < B; i++)
    {
        std::cout << task[i].numberOfPixels;
        if (i != B - 1)  std::cout << " ";
    }
    std::cout << "]" << std::endl;
    return totalNumberOfPixels;
}

int withOMP(unsigned char* data, int size)
{
    int numberOfPixels = 0;
    TASK task[B];
    #pragma omp parallel for
    for (int i = 0; i < B; i++)
    {
        task[i].start = 3 * i;
        task[i].end = size;
        task[i].data = data;
        task[i].numberOfPixels = 0;
        for (int j = task[i].start; j < task[i].end; j += 3 * B)
        {
            task[i].numberOfPixels += isSuitable((int)task[i].data[j], (int)task[i].data[j + 1], (int)task[i].data[j + 2]);
        }
    }
    for (int i = 0; i < B; i++)
    {
        totalNumberOfPixels += task[i].numberOfPixels;
    }
    std::cout << "Thread results: [";
    for (int i = 0; i < B; i++)
    {
        std::cout << task[i].numberOfPixels;
        if (i != B - 1)  std::cout << " ";
    }
    std::cout << "]" << std::endl;
    return totalNumberOfPixels;
}

int withCreateProcess()
{
    STARTUPINFO si[B];
    PROCESS_INFORMATION pi[B];
    char bff[512];
    ZeroMemory( &si, sizeof(si) );
    for (int i = 0; i < B; i++)
    {
        si[i].cb = sizeof(si[i]);
    }
    ZeroMemory( &pi, sizeof(pi) );
    for (int i = 0; i < B; i++)
    {
        std::sprintf(bff, "ProcessForCreateProcess.exe %d", i);
        std::cout << bff << std::endl;
        if( !CreateProcess( NULL,   // No module name (use command line)
                            bff,        // Command line
                            NULL,           // Process handle not inheritable
                            NULL,           // Thread handle not inheritable
                            FALSE,          // Set handle inheritance to FALSE
                            0,              // No creation flags
                            NULL,           // Use parent's environment block
                            NULL,           // Use parent's starting directory
                            &si[i],            // Pointer to STARTUPINFO structure
                            &pi[i] )           // Pointer to PROCESS_INFORMATION structure
                )
        {
            printf( "CreateProcess failed (%d).\n", GetLastError() );
            return 0;
        }
    }
    for (int i = 0; i < B; i++)
    {
        WaitForSingleObject( pi[i].hProcess, INFINITE );

        CloseHandle( pi[i].hProcess );
        CloseHandle( pi[i].hThread );
    }
    std::cout << "Thread results: [";
    for (int i = 0; i < B; i++)
    {
        std::string line;
        char* filename_in = new char[50];
        sprintf( filename_in, "ParallelProgramming/infoProcesses/img%d.txt", i);

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
    return totalNumberOfPixels;
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

    // calculating number of suitable pixels
    // using brute force
    totalNumberOfPixels = bruteForce(data, size);
    std::cout << "Total number of suitable pixels (brute force): " << totalNumberOfPixels << std::endl;
    std::cout << std::endl;

    // using threads
    totalNumberOfPixels = 0;
    totalNumberOfPixels = withCreateThread(data, size);
    std::cout << "Total number of suitable pixels (with threads): " << totalNumberOfPixels << std::endl;
    std::cout << std::endl;

    // using std::thread
    totalNumberOfPixels = 0;
    totalNumberOfPixels = withSTDThread(data, size);
    std::cout << "Total number of suitable pixels (with std::thread): " << totalNumberOfPixels << std::endl;
    std::cout << std::endl;

    // using OMP
    totalNumberOfPixels = 0;
    totalNumberOfPixels = withOMP(data, size);
    std::cout << "Total number of suitable pixels (with OMP): " << totalNumberOfPixels << std::endl;
    std::cout << std::endl;

    // using CreateProcess
    totalNumberOfPixels = 0;
    totalNumberOfPixels = withCreateProcess();
    std::cout << "Total number of suitable pixels (with CreateProcess): " << totalNumberOfPixels << std::endl;
    std::cout << std::endl;
    return 0;
}
