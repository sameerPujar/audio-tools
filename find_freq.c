#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

struct stream_params
{
	unsigned int rate;
	unsigned int channels;
	unsigned int bits;
};

int countZeroCrossings(FILE *file, struct stream_params *params,
	int *zeroCrossings);

int countZeroCrossings(FILE *file, struct stream_params *params,
	int *freq)
{
	void *buffer;
	int tInSec = 1, bytes_per_sample = params->bits / 8;
	long int bufSize = params->rate * params->channels * (bytes_per_sample) * tInSec;
	int num_read, i, countZeroCross = 0, variation;
	int32_t data_current = 0, data_prev = 0;

	switch (params->bits) {
		case 16:
			buffer = malloc(sizeof(int16_t) * bufSize);
			break;
		case 32:
			buffer = malloc(sizeof(int32_t) * bufSize);
			break;
		case 8:
		default:
			buffer = malloc(sizeof(int8_t) * bufSize);
	}

	if (!buffer) {
		fprintf(stderr, "Unable to allocate %ld bytes\n", bufSize);
		free(buffer);
		return -1;
	}

	/* move forward by 1 sec and check */
	fseek(file, 0, bufSize);
	num_read = fread(buffer, 1, bufSize, file);

	for (i = 0; i < bufSize - ((params->channels - 1) * bytes_per_sample);
		i+=params->channels) {
		switch (params->bits) {
			case 16:
				data_current = (int16_t) ((int16_t *)buffer)[i];
				break;
			case 32:
				data_current = (int32_t) ((int32_t *)buffer)[i];
				break;
			case 8:
			default:
				data_current = (int8_t) ((int8_t *)buffer)[i];
		}

		if (((data_prev < 0) && (data_current >= 0)) ||
			((data_prev > 0) && (data_current <= 0)))
			countZeroCross++;

		data_prev = data_current;
	}

	free(buffer);

	variation = ((*freq) * 2) / 10;
	if (abs(countZeroCross - ((*freq) * 2)) < variation) {
		fprintf(stderr, "SUCCESS: signal frequency matches, zero crosses found = %d\n",
			countZeroCross);
		return 0;
	} else {
		fprintf(stderr, "FAIL: freq. not FOUND, zero crosses found = %d\n", countZeroCross);
		return -1;
	}
}

int main(int argc, char **argv)
{
	FILE *file = NULL;
	struct riff_wave_header riff_wave_header;
	struct chunk_header chunk_header;
	struct chunk_fmt chunk_fmt;
	char *filename = NULL;
	struct stream_params params;
	int more_chunks = 1, signal_freq = 0, err = -1;

	if (argc < 3) {
		fprintf(stderr, "FAIL: arguments missing, exit ! \n");
		fprintf(stderr, "Usage %s <file> <freq_component> \n", argv[0]);
		return err;
	}

	filename = argv[1];
	signal_freq = atoi(argv[2]);
	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "FAIL: Unable to open file '%s'\n", filename);
		return err;
	}

	fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
	if ((riff_wave_header.riff_id != ID_RIFF) ||
		(riff_wave_header.wave_id != ID_WAVE)) {
		fprintf(stderr, "FAIL: '%s' is not a riff/wave file\n", filename);
		fclose(file);
		return err;
	}

	do {
		fread(&chunk_header, sizeof(chunk_header), 1, file);
		switch (chunk_header.id) {
			case ID_FMT:
				fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
				/* If the format header is larger, skip the rest */
				if (chunk_header.sz > sizeof(chunk_fmt))
					fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
				break;
			case ID_DATA:
				more_chunks = 0;
				break;
			default:
				fseek(file, chunk_header.sz, SEEK_CUR);
		}
	} while (more_chunks);

	params.rate = chunk_fmt.sample_rate;
	params.channels = chunk_fmt.num_channels;
	params.bits = chunk_fmt.bits_per_sample;

	err = countZeroCrossings(file, &params, &signal_freq);
	fclose(file);

	return err;
}
