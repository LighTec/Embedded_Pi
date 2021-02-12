#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


#define LOWER_LIMIT  0.5
#define UPPER_LIMIT  100.0
#define INCREMENT    0.5
#define FILENAME     "input.bin"


int main()
{
  double value;
  int fd;

  /*  Open a file for writing  */
  fd = openat(AT_FDCWD, FILENAME, O_WRONLY | O_CREAT, 0666);

  /*  Error check file creation  */
  if (fd < 0) {
    fprintf(stderr, "Can't open file for writing.  Aborting\n");
    exit(-1);
  }

  /*  Write double values to file  */
  for (value = LOWER_LIMIT; value <= UPPER_LIMIT; value += INCREMENT) {
    /*  Write out a double to file  */
    int n_written = write(fd, (char *)&value, sizeof(double));

    /*  Error check the write  */
    if (n_written != sizeof(double)) {
      fprintf(stderr, "Error writing file.  Aborting.\n");
      close(fd);
      exit(-1);
    }
  }

  /*  Close binary file  */
  close(fd);

  return 0;
}

  

