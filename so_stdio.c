#include "so_stdio.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 4096
#define R 0
#define W 1

struct _so_file {
    char *buffer;
    int fd;
	int bytes;
	int pos;
	int RW; // 0 read 1 write
	int eof;
	int size;
	int ftell;
};

int so_fileno(SO_FILE *stream) {
    return stream->fd;
}

SO_FILE *so_fopen(const char *pathname, const char *mode) {
    int fd = -1;

	if (!strcmp(mode, "r"))
		fd = open(pathname, O_RDONLY, 0644);

	if (!strcmp(mode, "r+"))
		fd = open(pathname, O_RDWR, 0644);

	if (!strcmp(mode, "w"))
		fd = open(pathname,	O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (!strcmp(mode, "w+"))
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0644);

	if (!strcmp(mode, "a"))
		fd = open(pathname, O_WRONLY | O_CREAT | O_APPEND, 0644);

	if (!strcmp(mode, "a+"))
		fd = open(pathname, O_RDWR | O_CREAT | O_APPEND, 0644);

	if (fd < 0)
		return NULL;

	SO_FILE *file = (SO_FILE *)malloc(sizeof(SO_FILE));

	file->fd = fd;
	file->pos = 0;
	file->bytes = 0;
	file->RW = R;
	file->eof = 0;
	file->size = BUFSIZE;
	file->ftell = 0;
	file->buffer = (char *)calloc(BUFSIZE, sizeof(char));

	return file;
}

int so_fclose(SO_FILE *stream) {
	int rc;
	int rf;

	rf = so_fflush(stream);
	rc = close(stream->fd);
	free(stream->buffer);
	free(stream);

	if (rf == SO_EOF || rc == SO_EOF)
		return SO_EOF;

	return rc;
}

int so_fflush(SO_FILE *stream) {
	if (!stream->pos)
		return 0;

    if (stream->RW == W) {
		int bytesWrite;

		bytesWrite = write(stream->fd, stream->buffer, stream->pos);
		while (bytesWrite != stream->pos) {
			if (bytesWrite < 0) {
				stream->eof = SO_EOF;
				return SO_EOF;
			}

			bytesWrite += write(stream->fd, stream->buffer + bytesWrite,
								stream->pos - bytesWrite);
		}

		stream->bytes += bytesWrite;
		stream->ftell += bytesWrite;
		stream->pos = 0;
		stream->size = BUFSIZE;
		memset(stream->buffer, 0, BUFSIZE);
	}

	return 0;
}

int so_fgetc(SO_FILE *stream) {
    unsigned char c;
	int bytesRead;

	bytesRead = 0;
	stream->RW = R;

	if (!stream)
		return SO_EOF;
	
	if (!stream->pos ||	stream->pos == BUFSIZE ||
		stream->pos == stream->size) {
		bytesRead = read(stream->fd, stream->buffer, BUFSIZE);

		if (!bytesRead || bytesRead < 0) {
			stream->eof = SO_EOF;
			return SO_EOF;
		}

		if (bytesRead > 0) {
			if (stream->pos == stream->size)
				stream->ftell += bytesRead;

			stream->size = bytesRead;
			stream->pos = 0;
		}
	}

	c = stream->buffer[stream->pos++];

	return (int)c;
}

size_t so_fread(void *ptr, size_t bytes, size_t nmemb, SO_FILE *stream) {
    int bytesToRead, i;
	unsigned char c;
	char *charPtr;

	charPtr = (char *)ptr;
	bytesToRead = nmemb * bytes;

	for (i = 0; i < bytesToRead; ++i) {
		c = (unsigned char)so_fgetc(stream);

		if (!so_feof(stream)) {
			memcpy(charPtr + i, &c, 1);
		} else {
			return i;
		}
	}

	return nmemb;
}

int so_fputc(int c, SO_FILE *stream) {	
	stream->RW = W;
	
	if (stream->pos >= stream->size) {
		if (so_fflush(stream) == SO_EOF)  {
			stream->eof = SO_EOF;
			return SO_EOF;
		}
	}

	stream->buffer[stream->pos++] = (unsigned char)c;
	
	return c;		
}

size_t so_fwrite(const void *ptr, size_t bytes, size_t nmemb, SO_FILE *stream) {
    int bytesToWrite, i;
	int c;
	char *charPtr;

	charPtr = (char *)ptr;
	bytesToWrite = nmemb * bytes;

	for (i = 0; i < bytesToWrite; ++i) {
		c = so_fputc(*(charPtr + i), stream);
		if (c == SO_EOF && *(charPtr + i) != SO_EOF)
			return i;
	}

	return nmemb;
}

int so_fseek(SO_FILE *stream, long offset, int whence) {
	if (stream->RW == W)
		so_fflush(stream);

	if (stream->RW == R) {
		memset(stream->buffer, 0, BUFSIZE);
		stream->size = BUFSIZE;
		stream->pos = 0;
	}

    stream->ftell = (whence + offset) + stream->pos;
	if (stream->ftell <= stream->bytes || stream->ftell >= 0) {
		return 0;
	}

	return SO_EOF;
}

long so_ftell(SO_FILE *stream) {
    return stream->ftell + stream->pos;
}

int so_feof(SO_FILE *stream) {
	return stream->eof;
}

SO_FILE *so_popen(const char *command, const char *type) {
    return  NULL;
}

int so_ferror(SO_FILE *stream) {
	return stream->eof;
}

int so_pclose(SO_FILE *stream) {
    return 0;    
}
