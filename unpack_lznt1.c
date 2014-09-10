/*
 * unpack lznt1 command-line tool
 *
 * Author: Rustam Abdullaev
 *
 * Public domain.
 */
#include <windows.h>
// under mingw might complain about redefinition of PEXECUTION_STATE, if so comment it out in ntapi.h
#include <ddk/ntifs.h>
#include <stdio.h>
#include <stdlib.h>

void printLastError(const char*msg)
{
	DWORD lastError = GetLastError();
	LPVOID lpMsgBuf;
	if (lastError == ERROR_SUCCESS || !FormatMessage(
	        FORMAT_MESSAGE_ALLOCATE_BUFFER |
	        FORMAT_MESSAGE_FROM_SYSTEM |
	        FORMAT_MESSAGE_IGNORE_INSERTS,
	        NULL, lastError,
	        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	        (LPTSTR) &lpMsgBuf, 0, NULL ))
	{
        fprintf(stderr, "ERROR! %s\n", msg);
	} else
	{
		fprintf( stderr, "ERROR! %s: %s\n", msg, (char*)lpMsgBuf);
		LocalFree( lpMsgBuf );
	}
}

int main(int argc, char**argv) {
	if (argc < 3) {
		printf("Usage: unpack_LZNT1 <filename> <outfile> [<begin offset> [<end offset>]]\n");
		return 1;
	}
	char *in = argv[1];
	char *out = argv[2];
	int offset = argc > 2 ? atoi(argv[3]) : 0;
	int skipEnd = argc > 3 ? atoi(argv[4]) : 0;
	HANDLE hfile = CreateFile(in, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		printLastError(in);
		return 2;
	}
	int compSize = GetFileSize(hfile, NULL);
	if (compSize == INVALID_FILE_SIZE) {
		printLastError(in);
		return 3;
	}
	compSize -=  offset + skipEnd;
	if (compSize <= 0) {
		fprintf(stderr, "ERROR! Invalid offset\n");
		return 3;
	}
	SetFilePointer(hfile, offset, NULL, 0);
	BYTE *buf = malloc(compSize);
	if (!buf) {
		fprintf(stderr, "ERROR! Out of memory.\n");
		return 4;
	}
	DWORD szout;
	ReadFile(hfile, buf, compSize, &szout, NULL);
	if (szout != compSize) {
		printLastError(in);
		free(buf);
		return 5;
	}
	DWORD uncbufsize = (compSize - offset) * 5;
	BYTE *uncbuf = malloc(uncbufsize);
	if (!uncbuf) {
		fprintf(stderr, "ERROR! Out of memory.\n");
		return 4;
	}
	ULONG uncbytes = 0;
	RtlDecompressBuffer(COMPRESSION_FORMAT_LZNT1, uncbuf, uncbufsize, buf, compSize, &uncbytes);
	if (uncbytes < compSize - 100) {
		printLastError("LZNT1 decompression failed");
		free(uncbuf);
		free(buf);
		return 6;
	}
	free(buf);
	CloseHandle(hfile);
	hfile = CreateFile(out, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		printLastError(out);
		free(uncbuf);
		return 7;
	}
	WriteFile(hfile, uncbuf, uncbytes, &szout, NULL);
	free(uncbuf);
	if (szout != uncbytes) {
		printLastError(out);
		return 7;
	}
	CloseHandle(hfile);
	printf("Done: unpacked %d -> %d bytes.\n", compSize, (int)uncbytes);
	return 0;
}
