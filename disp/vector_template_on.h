
#define CONCAT2x(a,b) a ## _ ## b 
#define CONCAT2(a,b) CONCAT2x(a,b)
#define CONCAT3x(a,b,c) a ## _ ## b ## _ ## c
#define CONCAT3(a,b,c) CONCAT3x(a,b,c)

#define FUNCTION(a,c) CONCAT3(a,BASE_TYPE,c)
#define TYPE(dir) struct CONCAT2(dir,BASE_TYPE)
