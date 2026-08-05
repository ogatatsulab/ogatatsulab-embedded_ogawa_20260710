#include <stdio.h>

/* ŠÖ”ƒ|ƒCƒ“ƒ^”z—ñ‚Ì“Yš */
typedef enum {
    TASU,       /* 0 */
    HIKU,       /* 1 */
    KAKERU,     /* 2 */
    WARU,       /* 3 */
    KEISAN_SU   /* ŒvZ•û–@‚ÌŒÂ” */
} Keisan;



int main( void );
int tasu( int a, int b );
int hiku( int a, int b );
int kakeru( int a, int b );
int waru( int a, int b );



int main(void)
{
    int a = 20;
    int b = 5;

    /*
     * intŒ^‚Ìˆø”‚ğ2ŒÂó‚¯æ‚èA
     * intŒ^‚ğ•Ô‚·ŠÖ”‚Ìƒ|ƒCƒ“ƒ^”z—ñ
     */
    // int (*func[KEISAN_SU])(int, int) = {
    //     [TASU]   = tasu,
    //     [HIKU]   = hiku,
    //     [KAKERU] = kakeru,
    //     [WARU]   = waru
    // };

    int (*func[KEISAN_SU])(int, int) = {
        tasu,
        hiku,
        kakeru,
        waru
    };

    printf("%d + %d = %d\n", a, b, func[TASU](a, b));
    printf("%d - %d = %d\n", a, b, func[HIKU](a, b));
    printf("%d * %d = %d\n", a, b, func[KAKERU](a, b));

    if (b != 0) {
        printf("%d / %d = %d\n", a, b, func[WARU](a, b));
    } else {
        printf("0‚Å‚ÍœZ‚Å‚«‚Ü‚¹‚ñB\n");
    }

    return 0;
}



/* ‰ÁZ */
int tasu(int a, int b)
{
    return a + b;
}



/* Œ¸Z */
int hiku(int a, int b)
{
    return a - b;
}



/* æZ */
int kakeru(int a, int b)
{
    return a * b;
}



/* œZ */
int waru(int a, int b)
{
    return a / b;
}
