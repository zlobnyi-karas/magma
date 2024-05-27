#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

size_t GetFileSize(const char* filePath);
int GetFileContent(const char* filePath, size_t fileSize, char* content);
int CheckIsFile(const char* filePath);
int CheckIsDirectory(const char* dirPath);