#include "file.h"

size_t GetFileSize(const char* filePath)
{
	size_t _file_size = 0;
	struct stat _fileStatbuff;
	int fd = open(filePath, O_RDONLY);
	if(fd == -1){
		_file_size = -1;
	}
	else{
		if ((fstat(fd, &_fileStatbuff) != 0) || (!S_ISREG(_fileStatbuff.st_mode))) {
			_file_size = -1;
		}
		else{
			_file_size = _fileStatbuff.st_size;
		}
		close(fd);
	}
	return _file_size;
}

int GetFileContent(const char* filePath, size_t fileSize, char* content)
{
	FILE* file = fopen(filePath, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "file openning error\n");
		return 1;
	}
	
	for (size_t i = 0; i < fileSize; i++)
	{
		content[i] = fgetc(file);
	}

	return 0;
}

int CheckIsFile(const char* filePath)
{
	struct stat objectStat;
	if(stat(filePath, &objectStat))
	{
		printf("невозможно получить информацию о %s\n", filePath);
		return 0;
	}
	if (S_ISREG(objectStat.st_mode))
	{
		return 1;
	}
	else
	{
		printf("%s не является файлом\n", filePath);
		return 0;
	}
}

int CheckIsDirectory(const char* dirPath)
{
	struct stat objectStat;
	if(stat(dirPath, &objectStat))
	{
		printf("невозможно получить информацию о %s\n", dirPath);
		return 0;
	}
	if (S_ISDIR(objectStat.st_mode))
	{
		return 1;
	}
	else
	{
		printf("%s не является каталогом\n", dirPath);
		return 0;
	}
}