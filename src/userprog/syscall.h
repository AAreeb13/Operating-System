#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/stddef.h"

void syscall_init (void);

/* Writes size bytes from buffer to the open file fd. Returns the number of bytes actually
written, which may be less than size if some bytes could not be written.
Writing past end-of-file would normally extend the file, but file growth is not implemented
by the basic file system. The expected behavior is to write as many bytes as possible up to
end-of-file and return the actual number written, or 0 if no bytes could be written at all.
Fd 1 writes to the console. Your code to write to the console should write all of buffer in
one call to putbuf(), at least as long as size is not bigger than a few hundred bytes. (It is
reasonable to break up larger buffers.) Otherwise, lines of text output by different processes
may end up interleaved on the console, confusing both human readers and our grading scripts.*/

/* Questions :
- What can cause less bytes to be written? 
- How do I check for this?
- Is it fine I return 0 if charBuffer is null
*/

int write(int fd, const void *buffer, unsigned size) {
    if (fd == 1) {
        const char *charBuffer = (const char *)buffer;

        // Checking buffer is not NULL, if NULL return 0 since no byte was written
        if (charBuffer == NULL) {
            return 0;
        }

        // Write to console using putbuf
        putbuf(charBuffer, size);
        return size;
    } else {
    }
}


#endif /* userprog/syscall.h */
