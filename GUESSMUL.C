#include <stdio.h>
#include <stdlib.h>

unsigned far x0(void);
unsigned far x1(void);
unsigned far x2(void);
unsigned far x3(void);
unsigned far x4(void);
unsigned far x5(void);
unsigned far x6(void);
unsigned far x7(void);

int main(void)
{
    unsigned timings[8];
    int i;
    //int stepsize;
    x0();
    timings[0] = x0();
    x1();
    timings[1] = x1();
    x2();
    timings[2] = x2();
    x3();
    timings[3] = x3();
    x4();
    timings[4] = x4();
    x5();
    timings[5] = x5();
    x6();
    timings[6] = x6();
    x7();
    timings[7] = x7();
    for (i = 0;i < 6; i++)
    {
        if(abs((int)(timings[i+1] - timings[i])) > 30)
        {
            int j;
            for (j = i+2; j < 7; j++)
            {
                if (abs((int)(timings[j] - timings[i+1])) > 30)
                {
                    int k;
                    printf("Guessed multiplier: %d\n", j-i-1);
                    for (k = 0; k < 8; k++)
                        printf("%d ", timings[k]);
                    printf("\n%d %d\n", i+1, j);
                    return 0;
                }
            }
            break;
        }
    }
    puts("Measurement failed. Strange de-turbo mode?");
    return 0;
}