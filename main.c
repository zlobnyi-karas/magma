#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gcrypt.h>
#include "magma_im.h"

int main(int argc, char** argv)
{
    if (argc != 5) //проверка количества переданных аргументов
    {
        printf("Использование:\nmac -f <путь к файлу> -k <32ух байтовый ключ>\nmac -d <путь к каталогу> -k <32ух байтовый ключ>\n");
        return 1;
    }
    
    if (!strcmp(argv[3], "-k")) // проверка наличия обязательного параметра (ключа)
    {
        if (strlen(argv[4]) != KEY_LEN) // проверка длины ключа
        {
            printf("ключ должен быть длиной 32 байта (32 символа)\n");
            return 1;
        }
    }
    else
    {
        printf("Использование:\nmac -f <путь к файлу> -k <32ух байтовый ключ>\nmac -d <путь к каталогу> -k <32ух байтовый ключ>\n");
        return 1;
    }

    char mac[2*MAC_LEN+1]; // буффер под контрольную сумму
    gcry_mac_hd_t mac_handle;
    if (MagmaImInit(&mac_handle, argv[4])) // иниицализация контектса шифрования 
    {
        return 1;
    }
    gcry_error_t error;

    if (!strcmp(argv[1], "-f")) // файл 
    {
        if (CheckIsFile(argv[2])) // проверка является ли файлом
        {
            if (MagmaImGetFileMAC(argv[2], mac_handle, mac)) //ошибка
            {
                return 1;
            }
            else                                             //успешное выполнение
            {
                printf("%s\n", mac);
            }
        }
    }
    else if (!strcmp(argv[1], "-d")) // каталог
    {
        if (CheckIsDirectory(argv[2])) // проверка является ли каталогом
        {
            if (MagmaImGetDirectoryMAC(argv[2], mac_handle, mac)) //ошибка
            {
                return 1;
            }
            else                                                  //успешное выполнение
            {
                printf("%s\n", mac);
            }
        }
    }

    return 0;
}