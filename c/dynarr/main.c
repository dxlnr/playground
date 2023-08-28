#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_bits(size_t const size, void const * const ptr)
{
  unsigned char *b = (unsigned char*) ptr;
  unsigned char byte;
  int i, j;
  
  for (i = size-1; i >= 0; i--) {
    for (j = 7; j >= 0; j--) {
      byte = (b[i] >> j) & 1;
      printf("%u", byte);
    }
  }
  puts("");
}

typedef struct file_spec {
  char type;
  /* The intention is behind n[1] is to allocate a variable-length array at the end of the struct.
   * manually allocating more memory than the size of the struct to accommodate a longer array.
   * This is a common technique in C, but it is not part of the C standard. 
   */
  char name[1];
} file_spec_t;

/* 
 *  files:  Array of file_spec_t pointers.
 *  fc:     Number of files in the array.
 */
typedef struct {
  file_spec_t **files;
  int fc;
} state_t;

/*
 *  ptab:   Pointer to the table (array) of pointers. 
 *  sptr:   Pointer to an integer that holds the current number of elements in the dynamic array (current size).
 *  data:   Pointer to the data that needs to be added to the dynamic array.
 */
static void dynarray(void* ptab, int *sptr, void *data)
{
  /* nb: Number of elements in the table. */
  int nb = *sptr;
  /* nb_alloc: Number of elements that will be allocated in the table if necessary. */
  int nb_alloc;
  /* Double Pointer (Why?)
   *
   * Memory Reallocation: When memory is reallocated, the starting address of the array is changed. 
   * If you were only passing a single pointer (void *p), any changes made inside the function to this pointer would be local to the function, 
   * and the caller wouldn't see these changes. 
   * By passing a double pointer (void **p), the function can modify the underlying single pointer, 
   * allowing it to reflect changes to the starting address of the array back to the caller.
   * */
  void **pp;
  printf("number of elements (nb): %d.\n", nb);
  /*
   * *(void ***)ptab dereferences the triple pointer, 
   * which gives us a double pointer (i.e., a pointer to a pointer of type void).
   * */
  pp = *(void ***)ptab;
  if ((nb & (nb - 1)) == 0) {
    printf("(nb) is a power of 2.\n");
    if (!nb) {
      nb_alloc = 1;
      printf("number of allocations: %d. (nb) + 1 \n", nb_alloc);
    } else {
      nb_alloc = nb * 2;
      printf("number of allocations: %d. (nb) * 2 \n", nb_alloc);
    }
    /* Reallocate the table size. */
    pp = realloc(pp, nb_alloc * sizeof(void *));
    /*
     * Double Indirection: 
     * The reason we need to use double indirection (void ***) is because we want to modify the original pointer that was passed to the function. 
     * C passes function parameters by value, not by reference. 
     * If you pass a pointer to a function and then modify that pointer inside the function, the original pointer outside the function remains unchanged. 
     * However, if you pass a pointer to a pointer (i.e., a double pointer) and modify the pointer it points to, 
     * you effectively update the original pointer outside the function. 
     * */
    *(void***) ptab = pp;
  }
  /* Add the data to the table. */
  pp[nb++] = data;
  /* Update the number of elements in the table. */
  *sptr = nb;
}

static void write_strs_to_table(state_t *s, char *strs)
{
  file_spec_t *fs = malloc(sizeof(file_spec_t) + strlen(strs));
  fs->type = 0;
  strcpy(fs->name, strs);
  dynarray(&s->files, &s->fc, fs);
}

int main()
{
  state_t *s = malloc(sizeof(state_t));
  s->fc = 0;

  printf("First set of char.\n");
  // First char str.
  char *str1 = "HelloWorld";
  write_strs_to_table(s, str1);
  printf("(fc) : %d.\n", s->fc);

  printf("\nSecond set of char.\n");
  // Second char str.
  char *str2 = "ShedSomeLight";
  write_strs_to_table(s, str2);
  printf("(fc) : %d.\n", s->fc);

  printf("\nThird set of char.\n");
  // Second char str.
  char *str3 = "GazingIntoTheAbyss";
  write_strs_to_table(s, str3);
  printf("(fc) : %d.\n", s->fc);

  printf("\nFourth set of char.\n");
  // Second char str.
  char *str4 = "Void.";
  write_strs_to_table(s, str4);
  printf("(fc) : %d.\n", s->fc);

  printf("\nFifth set of char.\n");
  // Second char str.
  char *str5 = "OneLastQuestion?";
  write_strs_to_table(s, str5);
  printf("(fc) : %d.\n", s->fc);

  return 0;
}
