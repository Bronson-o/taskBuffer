#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define uint8_t unsigned char

// дан циклический (кольцевой) буфер и некоторые функции работы с ним
#define BUFFER_SIZE 512

#if (BUFFER_SIZE & (BUFFER_SIZE - 1)) != 0
  #error "Incorrect buffer size"
#endif

typedef struct {
  size_t read_ptr;
  size_t write_ptr;
  uint8_t data[BUFFER_SIZE];
} CircularBuffer;

// ClearBuf очищает буфер (может также использоваться для инициализации структуры CircularBuffer)
void ClearBuf(CircularBuffer* pBuf)
{
  if (!pBuf)
    return;

  pBuf->read_ptr = 0;
  pBuf->write_ptr = 0;
}

// ReadByte читает байт из буфера.  если в буфере нет данных, возвращает -1.
int ReadByte(CircularBuffer* pBuf)
{
  if (!pBuf)
    return -1;

  if (pBuf->read_ptr == pBuf->write_ptr)
    return -1;

  int result = pBuf->data[pBuf->read_ptr];
  pBuf->read_ptr = (pBuf->read_ptr + 1) & (BUFFER_SIZE - 1);
  return result;
}

// пишет байт в буфер, возвращает true если запись прошла успешно
bool WriteByte(CircularBuffer* pBuf, uint8_t value)
{
  if (!pBuf)
    return false;

  size_t next = (pBuf->write_ptr + 1) & (BUFFER_SIZE - 1);

  if (next == pBuf->read_ptr)
    return false;

  pBuf->data[pBuf->write_ptr] = value;
  pBuf->write_ptr = next;
  return true;
}

// функция IsEmpty возвращает true если буфер пуст, иначе false
// пустым являтся буфер в котором нет данных для чтения.
bool IsEmpty(CircularBuffer* pBuf)
{
  if (!pBuf)
    return true;

  return pBuf->read_ptr == pBuf->write_ptr;
}


// функция IsFull возвращает true если буфер полон, иначе false
// попытка писать в полный буфер всегда будет завершаться неудачей.
bool IsFull(CircularBuffer* pBuf)
{
  if (!pBuf)
    return false;

  size_t next = (pBuf->write_ptr + 1) & (BUFFER_SIZE - 1);

  return next == pBuf->read_ptr;
}

// что возвращает функция GetSomething? переименуйте ее, чтобы название соответствоало возвращаемому значению 
size_t GetDataSize(CircularBuffer* pBuf)
{
  if (!pBuf)
    return 0;

  return (pBuf->write_ptr - pBuf->read_ptr) & (BUFFER_SIZE - 1);
}

// нам нужна фукнция для перемещения данных из одного циклического буфера в другой
// мы решили ее объявить так
// size_t BufMove(CicrularBufffer* pDest, CicrularBufffer* pSource)
// предполагается что pDest и pSource указывают на разные буферы
// функция должна перемещать максимально возможное кол-во байт из Source в Dest
// и возвращать число скопированных байт.
// т.е если в Dest буфере не хватет места для всего содержимого из Source, 
// переместиться должны только те байты для которых есть место, остаток должен остаться в Source буфере 
// если же места хватает, то переместиться должно все, и буфер Source остаться пуст.

// программист написал вот такую фукнцию
// соответствует ли она описаню данному выше?
// какие у нее есть недостатки?
size_t BufMoveSlow(CircularBuffer* pDest, CircularBuffer* pSource)
{
  if (!pDest || !pSource)
    return 0;

  if (pDest == pSource)
    return 0;

  int value;
  size_t result = 0;

  while ((value = ReadByte(pSource)) != -1 && WriteByte(pDest, value))
    result++;

  return result;
}

// напишите свой вариант функции перемещения данных
size_t BufMoveFast(CircularBuffer* pDest, CircularBuffer* pSource)
{
    if (!pDest || !pSource)
        return 0;

    if (pDest == pSource)
        return 0;

    size_t srcSize = GetDataSize(pSource);
    size_t destFree = (pDest->read_ptr - pDest->write_ptr - 1) & (BUFFER_SIZE - 1);

    size_t moveSize = srcSize;
    if (destFree < moveSize)
        moveSize = destFree;

    if (moveSize == 0)
        return 0;

    size_t srcPos = pSource->read_ptr;
    size_t destPos = pDest->write_ptr;
    size_t remaining = moveSize;

    while (remaining > 0)
    {
        size_t srcChunk = BUFFER_SIZE - srcPos;
        size_t destChunk = BUFFER_SIZE - destPos;

        size_t chunk = srcChunk;

        if (destChunk < chunk)
            chunk = destChunk;

        if (remaining < chunk)
            chunk = remaining;

        memcpy(&pDest->data[destPos], &pSource->data[srcPos], chunk);

        srcPos = (srcPos + chunk) & (BUFFER_SIZE - 1);
        destPos = (destPos + chunk) & (BUFFER_SIZE - 1);

        remaining -= chunk;
    }

    pSource->read_ptr = srcPos;
    pDest->write_ptr = destPos;

    return moveSize;
}

// вспомогательная функция для отладки
void PrintBuffer(CircularBuffer* pBuf) 
{ 
  if (!pBuf)
    return;

  if (pBuf->read_ptr == pBuf->write_ptr)
    printf(" Empty");

  size_t pos;    

  for (pos = pBuf->read_ptr; pos != pBuf->write_ptr; pos = (pos + 1) & (BUFFER_SIZE - 1))
    printf(" %02x", pBuf->data[pos]);

  printf("\n");
}

// код ниже можно менять по своему усмотрению для тестирования фукнций

CircularBuffer bufferA;
CircularBuffer bufferB;

int main(){
        ClearBuf(&bufferA);
        ClearBuf(&bufferB);

        WriteByte(&bufferA, 4);
        WriteByte(&bufferA, 5);

        WriteByte(&bufferB, 1);
        WriteByte(&bufferB, 2);
        WriteByte(&bufferB, 3);

        printf("BufferA before move:");
        PrintBuffer(&bufferA);
        printf("BufferB before move:");
        PrintBuffer(&bufferB);

        size_t res = BufMoveSlow(&bufferB, &bufferA);
        printf("BufMoveSlow moved %zu item(s) from BufferA to BufferB\n", res);

        printf("BufferA after move:");
        PrintBuffer(&bufferA);
        printf("BufferB after move:");
        PrintBuffer(&bufferB);
        
        ReadByte(&bufferB);
  
        printf("BufferB after read:");
        PrintBuffer(&bufferB);
  
  return 0;
}