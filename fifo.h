/*
 * 2010-2014 Michal Demin
 *
 */

#ifndef FIFO_H_
#define FIFO_H_

#include "platform.h"

#define FIFO_EMPTY(x) ((((struct fifo_t *)x)->write == (x)->read))
#define FIFO_FULL(x)  (((((struct fifo_t *)x)->write + 1) % (x)->e_num) == (x)->read)

struct fifo_t {
	uint32_t read;   // read pointer
	uint32_t write;  // write pointer
	uint32_t e_num;  // number of elements
	uint32_t e_size; // element size
	void *buffer;
};

void FIFO_Init(struct fifo_t *b, void *buffer, uint32_t e_size, uint32_t e_num);
void FIFO_Reset(struct fifo_t *);

uint32_t FIFO_Empty(struct fifo_t *b);
uint32_t FIFO_GetAvailable(struct fifo_t *b);
uint32_t FIFO_GetFreeSpace(struct fifo_t *b);

void *FIFO_GetReadAddr(struct fifo_t *b);
void FIFO_ReadDone(struct fifo_t *b);

void *FIFO_GetWriteAddr(struct fifo_t *b);
void FIFO_WriteDone(struct fifo_t *b);

uint32_t FIFO_GetReadSizeCont(struct fifo_t *b);
void FIFO_ReadDoneSize(struct fifo_t *b, uint32_t size);

void FIFO_WriteMore(struct fifo_t *b, const void *buf, uint32_t cnt);

#endif
