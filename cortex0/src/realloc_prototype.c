// #include "realloc_prototype.h"

// void *pvPortRealloc(void *ptr, size_t s)
// {
// uint8_t *puc = ( uint8_t * ) ptr;
// BlockLink_t *pxLink;
// void *newBlock;
// extern void memcpy( void*, void*, size_t );
// size_t blockSize;

//     if (ptr == NULL)
//     {
//         newBlock = pvPortMalloc(s);
//     }
//     else
//     {
//         puc -= heapSTRUCT_SIZE;
//         pxLink = (void*) puc;
//         blockSize = pxLink->xBlockSize - heapSTRUCT_SIZE;
//         if (s == 0)
//         {
//             newBlock = NULL;
//         }
//         else if (s > blockSize)
//         {
//             vTaskSuspendAll();
//             {
//                 newBlock = pvPortMalloc(s);
//                 if (newBlock != NULL)
//                 {
//                     memcpy(newBlock, ptr, blockSize);
//                     vPortFree(ptr);         
//                 }
//             }
//             xTaskResumeAll();

//         }
//         else //s < blockSize
//         {
//             vTaskSuspendAll();
//             {
//                 size_t newBlockSize = s + heapSTRUCT_SIZE + ( portBYTE_ALIGNMENT - ( s & portBYTE_ALIGNMENT_MASK ) );
//                 //if right segment is to few, do nothing
//                 if (pxLink->xBlockSize - newBlockSize > heapSTRUCT_SIZE)
//                 {
//                     //split current block
//                     pxLink->xBlockSize = newBlockSize;
//                     BlockLink_t *blockToFree = (void*)((uint8_t*)pxLink + pxLink->xBlockSize);
//                     blockToFree->xBlockSize = blockSize + heapSTRUCT_SIZE - pxLink->xBlockSize;
//                     //free second part
//                     prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) blockToFree ) );
//                     xFreeBytesRemaining += blockToFree->xBlockSize;
//                 }
//                 newBlock = ptr;
//             }
//             xTaskResumeAll();           
//         }
//     }
//     return newBlock;
// }