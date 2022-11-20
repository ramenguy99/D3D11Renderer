typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32   bool32;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float float32;
typedef double float64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s32   b32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define InvalidCodePath Assert(0);

#define internal static
#define local_persist static
#define global_variable static

#define ArrayCount(x) (sizeof((x)) / sizeof((x[0])))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define ALIGN_UP(x, a) ALIGN_DOWN((x) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((size_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((size_t)(p), (a)))

#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)

#define Kilobytes(x) (1024LL * (x))
#define Megabytes(x) (1024LL * Kilobytes(x))
#define Gigabytes(x) (1024LL * Megabytes(x))
#define Terabytes(x) (1024LL * Gigabytes(x))

#define ZeroAlloc(x) calloc(1, (x))
#define Free(x) free(x)

#define For(Index, Count) for(u32 (Index) = 0; (Index) < (Count); (Index)++)

#ifdef _MSC_VER
#define COMPILER_MSVC
#endif

#ifdef DISABLE_ASSERT
#define Assert(x)
#else

#ifdef COMPILER_MSVC
#define Assert(x) if(!(x)) { __debugbreak(); }
#else
#define Assert(x) if(!(x)) { u8 a = *(u8*)0; }
#endif
#endif
