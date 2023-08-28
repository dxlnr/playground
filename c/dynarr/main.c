#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct file_spec {
  char type;
  /* the intention is behind n[1] is to allocate a variable-length array at the end of the struct.
   * manually allocating more memory than the size of the struct to accommodate a longer array.
   * This is a common technique in C, but it is not part of the C standard. 
   */
  char name[1];
} file_spec_t;

typedef struct {
  file_spec_t **files;
} state_t;

typedef struct {
    void *data;
    size_t size;
} n_buffer_t;

void *dynarray(void* ptab, int size, void *data)
{
  /* void *ptab = malloc(size); */
}

int main()
{
  char *str = "HelloWorld!";

  state_t *s = malloc(sizeof(state_t));
  /* s->files = str; */

  file_spec_t *fs = malloc(sizeof(file_spec_t) + strlen(str));
  fs->type = 0;
  strcpy(fs->name, str);
  /* dynarray(s->files, , ); */

  return 0;
}
