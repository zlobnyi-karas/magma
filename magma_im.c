#include "magma_im.h"

int MagmaImInit(gcry_mac_hd_t* mac_handle, const char* key)
{
    if (!gcry_check_version("1.9.4"))
    {
        fprintf(stderr, "gcrypt version mismatch\n");
        return 1;
    }
    gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_ENABLE_M_GUARD);
    gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);

    gcry_error_t error;

    error = gcry_mac_open(mac_handle, GCRY_MAC_CMAC_GOST28147, 0, NULL);
    if (error)
    {
        fprintf(stderr,"mac handle oppening error: %s\n", gcry_strerror(error));
        return 1;
    }

    error = gcry_mac_setkey(*mac_handle, key, KEY_LEN);
    if (error)
    {
        fprintf(stderr,"key set error: %s\n", gcry_strerror(error));
        gcry_mac_close(*mac_handle);
        return 1;
    }

    return 0;
}

int MagmaImGetMAC(gcry_mac_hd_t mac_handle, const char* inBuffer,\
              size_t inBufferLength, char* outBuffer)
{
    gcry_error_t error;

    error = gcry_mac_write(mac_handle, inBuffer, inBufferLength);
    if (error)
    {
        fprintf(stderr,"mac writing error: %s\n", gcry_strerror(error));
        return 1;
    }

    unsigned char mac[MAC_LEN];
    size_t buflen = 8;
    error = gcry_mac_read(mac_handle, mac, &buflen);
    if (error)
    {
        fprintf(stderr,"mac reading error: %s\n", gcry_strerror(error));
        return 1;
    }
    
    for (int i = 0; i < MAC_LEN; i++)
    {
        sprintf(&outBuffer[i*2], "%02x", mac[i]);
    }
    outBuffer[2 * MAC_LEN] = '\0';

    error = gcry_mac_reset(mac_handle);
    if (error)
    {
        fprintf(stderr,"mac_handle reseting error: %s\n", gcry_strerror(error));
        return 1;
    }
    

    return 0;
}

int MagmaImGetFileMAC(const char* filePath, gcry_mac_hd_t mac_handle, char* outBuffer)
{
    size_t fileSize = GetFileSize(filePath);
    if (fileSize == -1)
    {
        fprintf(stderr, "GetFileSize error\n");
        return 1;
    }
    char* fileContent = malloc(fileSize);
    if (fileContent == NULL)
    {
        fprintf(stderr, "Memory error\n");
        return 1;
    }
    
    if (GetFileContent(filePath, fileSize, fileContent))
    {
        free(fileContent);
        return 1;
    }

    if (MagmaImGetMAC(mac_handle, fileContent, fileSize, outBuffer))
    {
        free(fileContent);
        return 1;
    }

    free(fileContent);
    return 0;
}

int MagmaImGetDirectoryMAC(const char* directoryPath, gcry_mac_hd_t mac_handle, char* outBuffer)
{ 
    DIR *dir;
    struct dirent* dp;
    struct stat objectStat;

    dir = opendir(directoryPath);
    if (dir == NULL)
    {
        fprintf(stderr, "error with openning directory\n");
        return 1;
    }

    size_t bufferLength = 0; //длина буффера от которого будет браться контрольная сумма
    char* objectPath;
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
        {
            objectPath = malloc(strlen(directoryPath) + 1 + strlen(dp->d_name) + 1);
            if (objectPath == NULL)
            {
                closedir(dir);
                fprintf(stderr, "memory error\n");
                return 1;
            }
            
            sprintf(objectPath, "%s/%s\0", directoryPath, dp->d_name);
            if (stat(objectPath, &objectStat))
            {
                free(objectPath);
                closedir(dir);
                fprintf(stderr, "getting object data error\n");
                return 1;
            }
            free(objectPath);
            if (S_ISREG(objectStat.st_mode)) // файл
            {
                bufferLength += 6 + strlen(dp->d_name) + 2 * MAC_LEN + 1;
            }
            else if (S_ISDIR(objectStat.st_mode)) // катлог
            {

                bufferLength += 5 + strlen(dp->d_name) + 2 * MAC_LEN + 1;
            }
        }
    }

    char* buffer;
    rewinddir(dir);

    buffer = malloc(bufferLength); //буфер в котором храним данные для хешировнниия
    if (buffer == NULL)
    {
        closedir(dir);
        fprintf(stderr, "memory error\n");
        return 1;
    }
    size_t index = 0;
    char mac[2 * MAC_LEN + 1];
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
        {
            char* objectPath = malloc(strlen(directoryPath) + 1 + strlen(dp->d_name) + 1);
            if (objectPath == NULL)
            {
                free(buffer);
                closedir(dir);
                fprintf(stderr, "memory error\n");
                return 1;
            }
            sprintf(objectPath, "%s/%s\0", directoryPath, dp->d_name);
            if(stat(objectPath, &objectStat))
            {
                free(buffer);
                free(objectPath);
                closedir(dir);
                fprintf(stderr, "getting object data error\n");
                return 1;
            }

            if (S_ISREG(objectStat.st_mode)) //файл
            {
                if (MagmaImGetFileMAC(objectPath, mac_handle, mac))
                {   
                    free(buffer);
                    free(objectPath);
                    closedir(dir);
                    return 1;
                }
                sprintf(&buffer[index], "file:%s:%s\0", dp->d_name, mac);
                index += 6 + strlen(dp->d_name) + 2 * MAC_LEN + 1;
            }
            else if (S_ISDIR(objectStat.st_mode)) //каталог
            {
                if (MagmaImGetDirectoryMAC(objectPath, mac_handle, mac))
                {
                    free(buffer);
                    free(objectPath);
                    closedir(dir);
                    return 1;
                }
                sprintf(&buffer[index], "dir:%s:%s\0", dp->d_name, mac);
                index += 5 + strlen(dp->d_name) + 2 * MAC_LEN + 1;
            }
            free(objectPath);
        }
    }

    closedir(dir);
        
    if (MagmaImGetMAC(mac_handle, buffer, bufferLength, outBuffer)) //котнтрольная сумма от буфера
    {
        free(buffer);
        return 1;
    }
    else
    {
        free(buffer);
        return 0;
    }
}