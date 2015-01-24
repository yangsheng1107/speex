// tar zxvf speex-1.2rc2.tar.gz
// ./configure
// make

// ./bin/speex -e -i file_mono.pcm -o file_mono.spx
// ./bin/speex -d -i file_mono.spx -o file_mono1.pcm
// ./bin/speex -e -i file_stereo.pcm -o file_stereo.spx
// ./bin/speex -d -i file_stereo.spx -o file_stereo1.pcm
//
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "speex/speex.h"

#define FRAME_SIZE 160

typedef int (* CALLBACK) (const char* from, const char* to); 

int encode(const char* from, const char* to)
{
	FILE *pcm, *spx;
	short in[FRAME_SIZE];
	float input[FRAME_SIZE];
	char cbits[200];
	SpeexBits bits;
	void *state;
	int nbBytes;
	int i, tmp;

	printf ("start encoding from %s to %s.\n", from, to);
    pcm = fopen(from, "rb");
    if (pcm == NULL) {
        printf("open %s failure! \n", from);
		return -1;
    }
    spx = fopen(to, "wb");
    if (spx == NULL) {
        printf("open %s failure! \n", to);
        fclose(pcm);
		return -1;
    }	

	state = speex_encoder_init(&speex_nb_mode);

	/*Set the quality to 8 (15 kbps)*/
	tmp=8;
	speex_encoder_ctl(state, SPEEX_SET_QUALITY, &tmp);

	speex_bits_init(&bits);
	
	while(1)
	{
		fread(in, sizeof(short), FRAME_SIZE, pcm);
		if (feof(pcm))
			break;
		/*Copy the 16 bits values to float so Speex can work on them*/
		for (i=0;i<FRAME_SIZE;i++)
			input[i]=(float)in[i];

		/*Flush all the bits in the struct so we can encode a new frame*/
		speex_bits_reset(&bits);

		/*Encode the frame*/
		speex_encode(state, input, &bits);
		/*Copy the bits to an array of char that can be written*/
		nbBytes = speex_bits_write(&bits, cbits, 200);

		/*Write the size of the frame first. This is what sampledec expects but
		it's likely to be different in your own application*/
		fwrite(&nbBytes, sizeof(int), 1, spx);
		/*Write the compressed data*/
		fwrite(cbits, 1, nbBytes, spx);
	}

	speex_bits_destroy(&bits);
	speex_encoder_destroy(state);
    fclose(spx);
    fclose(pcm);

    return 0;
}

int decode(const char* from, const char* to)
{
	/*Holds the audio that will be written to file (16 bits per sample)*/
	short out[FRAME_SIZE];
	/*Speex handle samples as float, so we need an array of floats*/
	float output[FRAME_SIZE];
	char cbits[200];
	FILE *pcm, *spx;
	int nbBytes;
	/*Holds the state of the decoder*/
	void *state;
	/*Holds bits so they can be read and written to by the Speex routines*/
	SpeexBits bits;
	int i, tmp;
	printf ("start decoding from %s to %s.\n", from, to);
    spx = fopen(from, "rb");
	if(spx == NULL)
	{
		printf("failed to open file %s\n", from);
		return -1;
	}
    pcm = fopen(to, "wb");
	if(pcm == NULL)
	{
		printf("failed to open file %s\n", to);
		fclose(spx);
		return -1;
	}

	state = speex_decoder_init(&speex_nb_mode);
	
	/*Set the perceptual enhancement on*/
	tmp=1;
	speex_decoder_ctl(state, SPEEX_SET_ENH, &tmp);

	speex_bits_init(&bits);

	while (1)
	{
		/*Read the size encoded by sampleenc, this part will likely be 
		different in your application*/
		fread(&nbBytes, sizeof(int), 1, spx);
		//fprintf (stderr, "nbBytes: %d\n", nbBytes);
		if (feof(spx))
			break;
      
		/*Read the "packet" encoded by sampleenc*/
		fread(cbits, 1, nbBytes, spx);
		/*Copy the data into the bit-stream struct*/
		speex_bits_read_from(&bits, cbits, nbBytes);

		/*Decode the data*/
		speex_decode(state, &bits, output);

		/*Copy from float to short (16 bits) for output*/
		for (i=0;i<FRAME_SIZE;i++)
			out[i]=(short)output[i];

		/*Write the decoded audio to file*/
		fwrite(out, sizeof(short), FRAME_SIZE, pcm);	   
	}

	/*Destroy the decoder state*/
	speex_decoder_destroy(state);
	/*Destroy the bit-stream truct*/
	speex_bits_destroy(&bits);

	fclose(spx);
	fclose(pcm);
	
	return 0;
}

int main (int argc, char ** argv)
{
	CALLBACK func;
	char *from, *to;
    char c;

	if(argc < 2)
	{
		printf ("Usage: %s [-e:pcm 2 spx][-d:spx 2 pcm][-i <inputpath>][-o <outputpath>].\n", argv[0]);
		return 1;
	}
	
    while (1) {
        c = getopt (argc, argv, "edi:o:");
        if (c == -1) {
            /* We have finished processing all the arguments. */
            break;
        }
        switch (c) {
        case 'e':
			func = encode;
            break;
        case 'd':
			func = decode;
            break;			
        case 'i':
			from = optarg;
            break;
        case 'o':
			to = optarg;
            break;			
        case '?':
        default:
			//break;
            printf ("Usage: %s [-e:pcm 2 spx][-d:spx 2 pcm][-i <inputpath>][-o <outputpath>].\n", argv[0]);
        }
    }
	
	func(from, to);

    return 0;
}
