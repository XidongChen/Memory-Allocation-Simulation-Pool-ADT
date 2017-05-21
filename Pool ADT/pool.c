#include "pool.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////
// INTEGRITY INSTRUCTIONS

// Explicitly state the level of collaboration on this question
// Examples:
//   * I discussed ideas with classmate(s) [include name(s)]
//   * I worked together with classmate(s) in the lab [include name(s)]
//   * Classmate [include name] helped me debug my code
//   * I consulted website [include url]
//   * None
// A "None" indicates you completed this question entirely by yourself
// (or with assistance from course staff)
/////////////////////////////////////////////////////////////////////////////
// INTEGRITY STATEMENT:
// I received help from the following sources:

// None

// Name: Xidong Chen
// login ID: x545chen
/////////////////////////////////////////////////////////////////////////////

struct pool {
  char *item;
  int len;
  int left;
  struct info *memo;
  int mlen;
};

struct info {
  int cur_pos;
  int end_pos;
  struct info * next;
};

// pool_create() creates a new pool of size characters
// effects: allocates memory (caller must call pool_destroy)
// time: O(1)
struct pool *pool_create(int size) {
  struct pool *p = malloc(sizeof (struct pool));
  p->item = malloc(sizeof (char) * size);
  p->len = size;
  p->left = size;
  p->memo = NULL;
  p->mlen = 0;

  return p;

}

// pool_destroy(p) destroys the pool p if there are no active allocations
//   returns true if successful or false if there are active allocations
// effects:  the memory at p is invalid (freed) if successful
// time: O(1)
bool pool_destroy(struct pool *p) {
  assert(p);
  
  //printf("TEST: P->LEFT IS %d\n", p->left);
  if (p->mlen == 0) {
    free(p->memo);
    free(p->item);
    free(p);
    return true;
  } else {
    return false;
  }
}


// pool_alloc(p, size) returns a pointer to an uninitialized char array
//   of size from within pool p, or NULL if no block of size is available
// effects: modifies p if successful
// time: O(n) where n is the number of info in the p
// for the loop: sum:[1...n]:O(1)
//               = O(n)
char* pool_alloc(struct pool *p, int size) {
  assert(p);
  assert(size > 0);

  //printf("left before alloc is %d\n", p->left);
  // not enough memory is left
  if (size > p->left) {
    return NULL;
  }

  // added to a new-initialized pool
  if (p->mlen == 0) {
    p->mlen++;
    p->memo = malloc(sizeof (struct info));
    p->memo->cur_pos = 0;
    p->memo->end_pos = size - 1;
    p->memo->next = NULL;

    p->left -= size;
    return &p->item[0];
  }

  int max_avai = p->memo->cur_pos - 0;

  if (max_avai >= size) {// insert before the first block
    struct info *newmemo = malloc(sizeof (struct info));
    newmemo->cur_pos = 0;
    newmemo->end_pos = size-1;
    newmemo->next = p->memo;
    p->memo = newmemo;
    p->left -= size;
    p->mlen++;
    return &p->item[0];

  } else {// if the space is not enough in the beginning
    struct info *curmemo = p->memo;
    struct info *nextmemo = curmemo->next;

    for (int i = 0; i < p->mlen - 1; i++) {
      int gap = nextmemo->cur_pos - curmemo->end_pos - 1;
      //printf("gap is %d\n", gap);
      if (gap >= size) {
        struct info *newmemo = malloc(sizeof (struct info));
        newmemo->cur_pos = curmemo->end_pos + 1;
        newmemo->end_pos = newmemo->cur_pos + size - 1;
        curmemo->next = newmemo;
        newmemo->next = nextmemo;
        p->left -= size;
        p->mlen++;
        return &p->item[newmemo->cur_pos];

      }
      curmemo = nextmemo;
      nextmemo = nextmemo->next;
    }

    // if there is no enough space in between then find the last pos
    curmemo = p->memo;
    int endpos = curmemo->end_pos;
    for (int i = 0; i < p->mlen - 1; i++) {
      curmemo = curmemo->next;
      endpos = curmemo->end_pos;
    }


    //printf("end pos is %d\n", endpos);
    // now check where there is enough space in the back to insert
    int end_avai = p->len - endpos - 1;
    // printf("end avaiable space is %d\n", end_avai);
    if (end_avai >= size) {
      struct info *newmemo = malloc(sizeof (struct info));
      newmemo->cur_pos = endpos + 1;
      newmemo->end_pos = endpos + size;
      //printf("newmemo end pos is %d\n", newmemo->end_pos);
      newmemo->next = curmemo->next;
      curmemo->next = newmemo;

      p->mlen++;
      p->left -= size;
      //printf("Now avaible space is %d\n", p->left);
      return &p->item[newmemo->cur_pos];


    } else {
      return NULL;
    }

  }

}


// pool_free(p, addr) makes the active allocation at addr available in the pool.
//   returns true if successful (addr corresponds to an active allocation
//   from a previous call to pool_alloc or pool_realloc) or false otherwise
// effects: modifies p if successful
// time: O(n)
// for the loop: sum:[1...n]:O(1)
//               = O(n)
bool pool_free(struct pool *p, char *addr) {
  assert(p);
  assert(addr);
  //printf("space left before free is %d\n", p->left);
  int index = addr - p->item;
  //printf("index is %d\n", index);
  // addr is not within the pool
  if(index > p->len) {
    return false;
  }

  bool exist = false;

  struct info *prememo = NULL;
  struct info *curmemo = p->memo;

  while(1) {
    if (curmemo == NULL) break;
    if (index == curmemo->cur_pos) {
      exist = true;
      break;
    }
    prememo = curmemo;
    curmemo = curmemo->next;
  }

  if (!exist) {
    return false;
  }

  struct info *backup = curmemo;
  int space = curmemo->end_pos - curmemo->cur_pos + 1;

  if (prememo) {
    prememo->next = curmemo->next;
  } else {
    p->memo = curmemo->next;
  }
  //printf("cuememo->next is NULL? %d\n", curmemo->next == NULL);
  free(backup);
  p->mlen--;
  p->left += space;
  
  return true;
}


// pool_realloc(p, addr, size) changes the size of the active allocation at
//   addr and returns the new address for the allocation.
//   returns NULL if addr does not correspond to an active allocation 
//   or the pool can not be resized (in which case the original allocation
//   does not change)
// effects: modifies p if successful
// time: O(n) + O(k) where k is min(size, m) and 
//       m is the size of the original allocation
// for the loop: sum:[1...n]:O(1)
//               = O(n)
char *pool_realloc(struct pool *p, char *addr, int size) {
  assert(p);
  assert(addr);
  assert(size > 0);

  int index = addr - p->item;
  // addr is not within the pool
  if(index > p->len) {
    return NULL;
  }

  // there is not active memory
  if (p->mlen == 0) {
    return NULL;
  }

  bool exist = false;

  struct info *prememo = NULL;
  struct info *curmemo = p->memo;

  while(curmemo) {
    if (index == curmemo->cur_pos) {
      exist = true;
      break;
    }
    prememo = curmemo;
    curmemo = curmemo->next;
  }

  // address is not found in the block memory
  if (!exist) {
    return NULL;
  }

  int old_size = curmemo->end_pos - curmemo->cur_pos + 1;
  

  // if size is smaller than original size
  if (size < old_size) {
    curmemo->end_pos = curmemo->cur_pos + size - 1;
    p->left += old_size - size;
    return &p->item[curmemo->cur_pos];
  }

  // if size is equal to original size
  if (size == old_size) {
    return &p->item[curmemo->cur_pos];
  }

  // if size is greater than original size

  
  // insert right after the block 
  if (curmemo->next == NULL) {
  
    int avai = p->len - curmemo->cur_pos;
    //printf("avai is %d\n", avai);
    if (avai >= size - old_size) {
      curmemo->end_pos = curmemo->cur_pos + size - 1;
      p->left -= size - old_size;
      return &p->item[curmemo->cur_pos];
    }
  }
  
  if (curmemo->next != NULL) {
   
    int avai = curmemo->next->cur_pos - curmemo->end_pos - 1;
     //printf("avai is %d\n", avai);
    if (avai >= size - old_size) {
      curmemo->end_pos = curmemo->cur_pos + size - 1;
      p->left -= size - old_size;
      return &p->item[curmemo->cur_pos];
    }
  }
  
  bool result = false;
  // insert in around the block
  //printf("is curmemo->next a null pointer? : %d\n\n", curmemo->next == NULL);
  
  if (prememo == NULL && curmemo->next == NULL) {
    
    if (p->len >= size) {
      result = true;
    }
  }
  
  if (prememo == NULL && curmemo->next != NULL) {
   
    int  avai = curmemo->next->cur_pos - 0;
    //printf("avai is %d\n", avai);
    if (avai >= size) {
      result = true;
    }
  }
  
  if (prememo != NULL && curmemo->next == NULL) {
   
    int  avai = p->len - prememo->end_pos - 1;
    if (avai >= size) {
      result = true;
    }
  }
  
  if (prememo != NULL && curmemo->next != NULL) {
    
    int avai = curmemo->next->cur_pos - prememo->end_pos - 1;
    //printf("avai is %d\n", avai);
    if (avai >= size) {
      result = true;
    }
  }

  // c) insert in between

  struct info *re_pre = p->memo;
  struct info *re_cur = p->memo->next;

  while(1){
    if (re_cur->next == NULL) break;

    int avai = re_cur->cur_pos - re_pre->end_pos - 1;
    if (avai >= size) {
      result = true;
    }

    re_pre = re_cur;
    re_cur = re_cur->next;
  }
  
  // insert in the end
  int end_avai = p->len - re_cur->end_pos - 1;
  if(end_avai >= size) {
    result = true;
  }

  if (result) {
    pool_free(p, addr);
    return pool_alloc(p, size);
  } else {
    return NULL;
  }
}



// pool_print_active(p) prints out a description of the active allocations 
//   in pool p using the following format:
//   "active: index1 [size1], index2 [size2], ..., indexN [sizeN]\n" or
//   "active: none\n" if there are no active allocations
//   where the index of an allocation is relative to the start of the pool
// effects: displays a message
// time: O(n)
// for the loop: sum:[1...n]:O(1)
//               = O(n)
void pool_print_active(struct pool *p) {
  assert(p);

  if (p->mlen == 0) {
    printf("active: none\n");
  } else if (p->mlen == 1) {
    int index = p->memo->cur_pos;
    int size = p->memo->end_pos - p->memo->cur_pos + 1;

    printf("active: %d [%d]\n", index, size);

  } else {
    struct info *curmemo = p->memo;
    printf("active: ");
    for (int i = 0; i < p->mlen - 1; i++) {
      int index = curmemo->cur_pos;
      int size = curmemo->end_pos - curmemo->cur_pos + 1;

      printf("%d [%d], ", index, size);
      curmemo = curmemo->next;
    }
    int index = curmemo->cur_pos;
    int size = curmemo->end_pos - curmemo->cur_pos + 1;
    printf("%d [%d]\n", index, size);
  }
}


// pool_print_available(p) prints out a description of the available 
//   contiguous blocks of memory still available in pool p:
//   "available: index1 [size1], index2 [size2], ..., indexM [sizeM]\n" or
//   "available: none\n" if all of the pool has been allocated
// effects: displays a message
// time: O(n)
// for the loop: sum:[1...n]:O(1)
//               = O(n)
void pool_print_available(struct pool *p){
  assert(p);
  if (p->mlen == 0) {
    printf("available: 0 [%d]\n", p->len);
    return;
  }

  if (p->left == 0) {
    printf("available: none\n");
    return;
  }

  // count how many available block exists
  int avai = 0;
  struct info *curmemo = p->memo;
  struct info *nextmemo = p->memo->next;

  if (curmemo->cur_pos - 0 > 0) {
    avai++;
  }

  while(1) {
    if(nextmemo == NULL) break;
    if (nextmemo->cur_pos - curmemo->end_pos - 1 > 0) {
      avai++;
    }
    curmemo = nextmemo;
    nextmemo = nextmemo->next;
  }

  if (p->len - curmemo->end_pos - 1 > 0) {
    avai++;
  }



  if (avai == 0) {
    printf("available: none\n");
    return;
  }

  int count = 0;
  curmemo = p->memo;
  nextmemo = p->memo->next;

  if (curmemo->cur_pos - 0 > 0) {
    count++;
    if (count == avai) {
      printf("available: %d [%d]\n", 0, curmemo->cur_pos);
    } else {
      printf("available: %d [%d], ", 0, curmemo->cur_pos);
    }
  }

  while(1) {
    if(nextmemo == NULL) break;
    int space = nextmemo->cur_pos - curmemo->end_pos - 1;
    if (space > 0) {
      count++;
      if (count == avai) {
        printf("%d [%d]\n",curmemo->end_pos+1, space);
      } else if (count == 1) {
        printf("available: %d [%d], ",curmemo->end_pos+1, space);
      } else {
        printf("%d [%d], ",curmemo->end_pos+1, space);
      }
    }
    curmemo = nextmemo;
    nextmemo = nextmemo->next;
  }

  int space = p->len - curmemo->end_pos - 1;
  //printf("p->len: %d\n", p->len);
  if (space > 0) {
    if (count == 0) {
      printf("available: %d [%d]\n", curmemo->end_pos+1, space);
    } else {
      printf("%d [%d]\n",curmemo->end_pos+1, space);
    }
  }
}