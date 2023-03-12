// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define HTSIZE 107

struct hash_bucket {
	struct spinlock lock;
	struct buf head;
};

uint hash(uint a, uint b)
{
	return a % HTSIZE;
}

struct {
  struct buf buf[NBUF];
  struct hash_bucket hash_bucket[HTSIZE];
} bcache;

void
binit(void)
{
  for (int i = 0; i < HTSIZE; i++) {
	bcache.hash_bucket[i].head.next = &bcache.hash_bucket[i].head;
	bcache.hash_bucket[i].head.prev = &bcache.hash_bucket[i].head;
	initlock(&bcache.hash_bucket[i].lock, "bcache");
  }
  for (int i = 0; i < NBUF; i++) {
	initsleeplock(&bcache.buf[i].lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct hash_bucket *hash_bucket = bcache.hash_bucket + hash(dev, blockno);
  struct buf *b;

  acquire(&hash_bucket->lock);

  // Is the block already cached?
  for(b = hash_bucket->head.next; b != &hash_bucket->head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&hash_bucket->lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  for(int i = 0; i < NBUF; i++){
    b = bcache.buf + i;
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      b->next = hash_bucket->head.next;
      b->prev = &hash_bucket->head;
      b->next->prev = b;
      b->prev->next = b;
      release(&hash_bucket->lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  struct hash_bucket *hash_bucket = bcache.hash_bucket + hash(b->dev, b->blockno);

  releasesleep(&b->lock);

  acquire(&hash_bucket->lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
  }
  
  release(&hash_bucket->lock);
}

void
bpin(struct buf *b) {
  struct hash_bucket *hash_bucket = bcache.hash_bucket + hash(b->dev, b->blockno);
  acquire(&hash_bucket->lock);
  b->refcnt++;
  release(&hash_bucket->lock);
}

void
bunpin(struct buf *b) {
  struct hash_bucket *hash_bucket = bcache.hash_bucket + hash(b->dev, b->blockno);
  acquire(&hash_bucket->lock);
  b->refcnt--;
  release(&hash_bucket->lock);
}


