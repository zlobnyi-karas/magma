#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gcrypt.h>
#include <dirent.h>
#include "file.h"

#define KEY_LEN 32
#define MAC_LEN 8

int MagmaImInit(gcry_mac_hd_t* mac_handle, const char* key);
int MagmaImGetMAC(gcry_mac_hd_t mac_handle, const char* inBuffer,\
              size_t inBufferLength, char* outBuffer);
int MagmaImGetFileMAC(const char* filePath, gcry_mac_hd_t mac_handle, char* outBuffer);
int MagmaImGetDirectoryMAC(const char* directoryPath, gcry_mac_hd_t mac_handle, char* outBuffer);