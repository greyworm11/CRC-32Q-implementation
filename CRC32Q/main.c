#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>
#include <math.h>

typedef unsigned char uchar;

int* GetBinaryChar(uchar symbol, int step);
void GetBinaryFileFromText(uchar* text, int* bin, int file_size);
void PrintArray(int* arr, int size);
void left_shift_crc(int* crc_bin, int start_index, int crc_len);
void PrintResultSimpleCRC(int* crc_bin, int crc_bin_size);
uint32_t UpdatedCRC(uchar* text, size_t length);
size_t FillRegistryWithFirstFourBits(uint32_t* reg, uchar* text, size_t length);
void PrintResultUpdatedCRC(uint32_t crc);
int* convert_dec_to_bin(uint32_t crc, int* bin_size);

#define MAXINPUTFILENAME 128
#define TRUE 1
#define FALSE 0
#define MAX_POLY_DEGREE 32
#define CRC_BITE_SIZE 32


#define BIT32 0x80000000
#define BIT8 0x80
#define POLY 0x814141AB
#define REG_INIT 0x0 
#define XOR_OUT 0x0



/*
* Gets input filename from stdout
* You can change this by adding, for example, argc and argv params
*/

char* GetInputFilename()
{

	char* filename = (char*)malloc(sizeof(char) * MAXINPUTFILENAME);
	if (filename == NULL) return NULL;

	memset(filename, 0, MAXINPUTFILENAME);

	fprintf(stdout, "Enter the filename for calculating CRC: ");

	fflush(stdout);

	int scanf_status = scanf("%s", filename);

	if (scanf_status != 1)
	{
		return NULL;
	}
	else
	{
		return filename;
	}
}

/*
* Opens file with the name FILENAME
* Returns pointer to the opened file
* Or NULL if file was not opened
*/

FILE* OpenFile(char* filename)
{

	FILE* file = NULL;

	file = fopen(filename, "r");

	if (file == NULL)
	{
		return NULL;
	}
	else {
		return file;
	}
}

/*
* Returns size of the file
*/

int GetFileSize(FILE* file)
{

	fseek(file, 0, SEEK_END);
	int file_size = (int)ftell(file);
	fseek(file, 0, SEEK_END);

	return file_size;
}

/*
* Reads all file data
*/

int ReadDataFromFile(FILE* fstream, uchar* data, char* filename, int file_length)
{

	fstream = fopen(filename, "rb");

	uchar symbol = 0;

	for (int i = 0; i < file_length; ++i)
	{
		symbol = fgetc(fstream);
		if (symbol < 0)
		{
			return EXIT_FAILURE;
		}
		if (symbol == 0x00)
		{
			data[i] = 0x01;
		}
		else
		{
			data[i] = symbol;
		}
	}

	return EXIT_SUCCESS;
}

/*
* Starting point of the program
*/

int main()
{
	setlocale(LC_ALL, "");

	char* input_filename = (char*)malloc(sizeof(char) * MAXINPUTFILENAME); 
	if (input_filename == NULL) return EXIT_FAILURE;

	memset(input_filename, 0, (size_t)MAXINPUTFILENAME);

	input_filename = GetInputFilename();

	FILE* file_in = OpenFile(input_filename);

	if (file_in == NULL)
	{
		fprintf(stderr, "Error while opening file\n");
		return EXIT_FAILURE;
	}

	int file_size = GetFileSize(file_in);

	uchar* text = (uchar*)malloc(sizeof(uchar) * (file_size + 1));

	if (text == NULL)
	{
		return EXIT_FAILURE;
	}

	memset(text, 0, (size_t)(file_size + 1) * sizeof(uchar));


	int file_read_status = ReadDataFromFile(file_in, text, input_filename, file_size);

	if (file_read_status == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	uchar* text_with_dop_null = (uchar*)malloc(sizeof(uchar*) * (strlen(text) + 4));
	memset(text_with_dop_null, 0, (strlen(text) + 4));
	memcpy(text_with_dop_null, text, strlen(text));

	uint32_t crc_updated = UpdatedCRC(text_with_dop_null, (strlen(text) + 4));

	PrintResultUpdatedCRC(crc_updated);
}

/*
* Turns file to a binary introduction
*/
void GetBinaryFileFromText(uchar* text, int* bin, int file_size)
{
	int step = 8;
	int ptr = 0;

	for (int i = 0; i < file_size; ++i)
	{
		int* bin_char = (int*)malloc(sizeof(int) * step);
		if (bin_char == NULL) return;
		memset(bin_char, 0, (size_t)step * sizeof(int));
		bin_char = GetBinaryChar(text[i], step);

		for (int j = 0; j < step; ++j)
		{
			bin[ptr++] = bin_char[j];
		}

		free(bin_char);
	}
}

/*
* Turns symbol to binary introduction
*/
int* GetBinaryChar(uchar symbol, int step)
{

	int* bin = (int*)malloc(sizeof(int) * step);
	if (bin == NULL) return NULL;

	memset(bin, 0, (size_t)step * sizeof(int));

	int ptr = step - 1;

	while (symbol)
	{
		int rem = symbol % 2;
		bin[ptr--] = rem;
		symbol /= 2;
	}

	return bin;
}

/*
* Shifts index of the binary crc introduction
*/
int shift_index(int* bin, int ptr, int len)
{

	for (int i = ptr; i < len; ++i)
	{
		if (bin[i] == 1)
		{
			return i;
		}
	}
}


/*
* Prints all of the elements of ARR with size SIZE
* ** You can use it for debugging
*/

void PrintArray(int* arr, int size)
{

	for (int i = 0; i < size; ++i)
	{
		printf("%d", arr[i]);
	}

	printf("\n\n");
}


/*
* Converts binary that represented as an array CRC_BIN with size CRC_BIN_SIZE to the decimal
* And return its value
*/

uint32_t convert_bin_to_dec(int* crc_bin, int crc_bin_size)
{
	uint32_t result = 0;

	int degree = crc_bin_size - 1;

	for (int i = 0; i < crc_bin_size; ++i)
	{
		result += (uint32_t)pow(2, degree) * (crc_bin[i]);
		degree--;
	}

	return result;
}


/*
* Main CRC32-Q calculation function
* Returns crc value
*/

uint32_t UpdatedCRC(uchar* text, size_t length)
{
	uint32_t reg = REG_INIT;
	uint32_t poly = POLY;

	uchar c;
	int j = 0;
	int first_byte_one_or_zero = FALSE;
	while (length != 0)
	{
		c = text[j];
		for (int i = 7; i >= 0; --i)
		{
			first_byte_one_or_zero = FALSE;
			if (reg & BIT32)
			{
				first_byte_one_or_zero = TRUE;
			}
			if (c & (uint32_t)BIT8)
			{
				reg <<= 1;
				reg += 1;
			}
			else
			{
				reg <<= 1;
			}
			if (first_byte_one_or_zero)
			{
				reg = reg ^ poly;
			}

			c <<= 1;
		}
		--length; j++;
	}

	reg ^= XOR_OUT;

	return reg;
}


/*
* Fills the first 4 bytes of crc
* Returns number of the added bytes
*/

size_t FillRegistryWithFirstFourBits(uint32_t* reg, uchar* text, size_t length)
{
	size_t counter = 0;
	uchar c;
	for (int i = 0; i != 32 && (length - counter) != 0; ++i)
	{
		if (i % 8 == 0)
		{
			c = text[counter];
			counter++;
		}
		if (c & BIT8)
		{
			*reg <<= 1;
			*reg += 1;
		}
		else
		{
			*reg <<= 1;
		}
		c <<= 1;
	}

	return counter;
}

/*
* Prints results to stdout
*/
void PrintResultUpdatedCRC(uint32_t crc)
{
	fprintf(stdout, "\n\t=== CRC-32Q VALUES ===\n\n");
	if (crc == 0)
	{
		fprintf(stdout, "\t0");
	}
	else {
		fprintf(stdout, "Binary: ");
		int* crc_bin = (int*)malloc(sizeof(int) * CRC_BITE_SIZE);
		if (crc_bin == NULL) return;
		int bin_size = 0;
		crc_bin = convert_dec_to_bin(crc, &bin_size);
		PrintArray(crc_bin, bin_size);
		fprintf(stdout, "Decimal: %u\n\n", crc);
		fprintf(stdout, "Hexadecimal: %0x\n", crc);
	}

	fprintf(stdout, "\n\n  note: you can simply add calculated CRC to the end of the file and calculate CRC again,\n  if there was a zero, so it works great!\n");
}

/*
* Conveting decimal value CRC to binary (holded in array which is returning)
*/
int* convert_dec_to_bin(uint32_t crc, int* bin_size)
{
	int* res = (int*)malloc(sizeof(int) * CRC_BITE_SIZE);
	if (res == NULL) return NULL;

	int ptr = CRC_BITE_SIZE - 1;
	int temp;
	while (crc)
	{
		temp = crc % 2;
		res[ptr--] = temp;
		crc /= 2;
		(*bin_size)++;
	}

	int start_ptr = CRC_BITE_SIZE - *bin_size;

	for (int j = start_ptr; j < *bin_size; ++j)
	{
		res[j - start_ptr] = res[j];
	}

	return res;
}