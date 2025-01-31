#include <stdio.h>
#include <math.h>

#include "portaudio/include/portaudio.h"
#include "portmidi/pm_common/portmidi.h"

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE (200)
typedef struct
{
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
    char message[20];
}
paTestData;

static int patestCallback( const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
  paTestData* data = (paTestData*)userData;
  float* out = (float*) outputBuffer;
  unsigned long i;

  (void) timeInfo;
  (void) statusFlags;
  (void) inputBuffer;

  for( i=0; i<framesPerBuffer; i++) {
    *out++ = data->sine[data->left_phase];
    *out++ = data->sine[data->right_phase];
    data->left_phase += 1;
    if( data->left_phase >= TABLE_SIZE ) data->left_phase -= TABLE_SIZE;
    data->right_phase += 3; //higher pitch
    if( data->right_phase >= TABLE_SIZE ) data->right_phase -= TABLE_SIZE;
  }

  return paContinue;
}

static void StreamFinished( void* userData )
{
  paTestData* data = (paTestData *) userData;
  printf( "Stream Completed: %s\n", data->message );
}

int main(void)
{
  PaStreamParameters outputParameters;
  PaStream* stream;
  PaError err;
  paTestData data;
  int i;

  printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

  // Init sine wave
  for( i=0; i<TABLE_SIZE; i++ )
  {
    data.sine[i] = (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. );
  }
  data.left_phase = data.right_phase = 0;

  err = Pa_Initialize();
  if( err != paNoError ) goto error;

  outputParameters.device = Pa_GetDefaultOutputDevice();
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr, "Error: No default output device.\n");
    goto error;
  }
  outputParameters.channelCount = 2;
  outputParameters.sampleFormat = paFloat32;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(
    &stream,
    NULL, // No input
    &outputParameters,
    SAMPLE_RATE,
    FRAMES_PER_BUFFER,
    paClipOff, // We won't output out of range samples so don't clip them
    patestCallback,
    &data
  );
  if( err != paNoError ) goto error;

  sprintf( data.message, "No Message" );
  err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
  if( err != paNoError ) goto error;

  err = Pa_StartStream( stream );
  if( err != paNoError ) goto error;

  printf("Play for %d seconds.\n", NUM_SECONDS );
  Pa_Sleep( NUM_SECONDS * 1000 );

  err = Pa_StopStream( stream );
  if( err != paNoError ) goto error;

  err = Pa_CloseStream( stream );
  if( err != paNoError ) goto error;

  Pa_Terminate();
  printf("Test finished.\n");

  return err;
error:
  Pa_Terminate();
  fprintf( stderr, "An error occurred while using the portaudio stream\n");
  fprintf( stderr, "Error number: %d\n", err);
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ));
  return err;
}