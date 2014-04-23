#ifndef _SOUND_H
#define _SOUND_H

#include <pulse/simple.h>
#include <pulse/error.h>

/**
 * @file G-2301-03-P2-sound.h
 * @details Sound recording/playing functions.
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/

int sampleFormat(enum pa_sample_format format, int channels);
int openRecord	(char *identificacion);
int openPlay	(char *identificacion);
int recordSound	(char * buf, int size);
int playSound	(char * buf, int size);
void closeRecord(void);
void closePlay	(void);

#endif
