#include <stdint.h>

/* number of fractional bits */
#define FIXED_POINT_BITS 14
#define FIXED_F (1 << FIXED_POINT_BITS)

/* fixed-point format */
typedef int64_t fixed_point_t;


/* conversion macros */
#define INT_TO_FIXED_POINT(x) (((x) * FIXED_F))
#define FIXED_POINT_TO_INT(x) ((x) / FIXED_F)

/* arithmetic macros */
#define FIXED_POINT_ADD(x, y) ((x) + (y))
#define FIXED_POINT_SUBTRACT(x, y) ((x) - (y))
#define FIXED_POINT_ADD_INT(x, n) ((x) + ((n) * FIXED_F))
#define FIXED_POINT_SUBTRACT_INT(x, n) ((x) - ((n) * FIXED_F))
#define FIXED_POINT_MULTIPLY(x, y) (((x) * (y)) / FIXED_F)
#define FIXED_POINT_MULTIPLY_INT(x, n) ((x) * (n))
#define FIXED_POINT_DIVIDE(x, y) (((x) * FIXED_F) / (y))
#define FIXED_POINT_DIVIDE_INT(x, n) ((x) / (n))

