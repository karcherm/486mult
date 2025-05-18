/*  Part of 486mult, trying to determine the 486 FSB multiplier using a microbenchmark
    Copyright (C) 2025 Michael Karcher

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

#define ARRAYSIZE(x) ((sizeof(x))/(sizeof((x)[0])))

typedef unsigned far measure_t(void);
measure_t x0, x1, x2, x3, x4, x5, x6, x7, x8;
extern int iterations;

static measure_t* const procs[] = {x0, x1, x2, x3, x4, x5, x6, x7, x8};

unsigned measure(measure_t* proc)
{
    proc();         // Make sure "proc" is in L1
    return proc();  // now use the result
}

unsigned g_minor_retries;
unsigned g_major_retries;

long measure_precise(measure_t* proc, int average_amount)
{
    unsigned minimum = 0xFFFF;
    unsigned close_to_minimum_count = 0;
    unsigned failure_count = 0;
    unsigned retry_count = 0;
    long sum = 0;
    do
    {
        unsigned sample = measure(proc);
        if (sample < minimum)
        {
            minimum = sample;
            close_to_minimum_count = 1;
            failure_count = 0;
            sum = minimum;
        }
        else if (sample < minimum + 5)
        {
            sum += sample;
            close_to_minimum_count++;
            failure_count = 0;
        }
        else
        {
            g_minor_retries++;
            failure_count++;
            if (failure_count == 128)
            {
                // got 128 successive measurements significantly exceeding
                // "minimum".

                // possibly "minimum" is excessively low due to
                // timer overflow (measurement interrupted for more than
                // 55ms. Retry, hoping to not get an invalid minimum.
                retry_count++;
                g_major_retries++;
                if (retry_count == 8)
                {
                    // abort instead of retrying an indefinite time.
                    return -1;
                }
                failure_count = 0;
                minimum = 0xFFFF;
                close_to_minimum_count = 0;
            }
        }
    } while(close_to_minimum_count < average_amount);
    return sum;
}

int calibrate(long target_speed, int average_amount)
{
    long initial_speed, calibrated_speed;
    iterations = 512;                        // Initial guess
    initial_speed = measure_precise(x0, average_amount);

    iterations = (target_speed * iterations) / initial_speed;

    calibrated_speed = measure_precise(x0, average_amount);
    if (calibrated_speed + calibrated_speed/8 < target_speed ||
        calibrated_speed - calibrated_speed/8 > target_speed)
    {
        fprintf(stderr, "Calibration failed. Goal: %u, actual %u\n", target_speed, calibrated_speed);
        return -1;
    }

    return 0;
}


int main(int argc, char* argv[])
{
    long timings[ARRAYSIZE(procs)];
    int deltas[ARRAYSIZE(procs)-1];
    int i;
    int verbose = 0;
    int average_amount = 32;
    int step_width = -1;
    int step_begin = -1;
    int step_count = 0;
    long step_height_sum = 0;
    long fsb_khz;
    const long measure_duration = 32768L;

    if (argc > 1)
    {
        char* arg = argv[1];
        if (*arg == '/' || *arg == '-')
            arg++;
        switch (tolower(*arg))
        {
            case 'h':
            case '?':
                puts("486MULT Version 1.99 - Copyright (c) 2025 Michael Karcher\n"
                     "/h  - print this help\n"
                     "/v  - verbose measurement mode\n"
                     "/c  - copyright and (no) warranty information");
                return 0;
            case 'v':
                verbose = 1;
                break;
            case 'c':
                puts("486MULT Version 1.99 - Copyright (c) 2025 Michael Karcher\n"
                     "This program comes with ABSOLUTELY NO WARRANTY:\n"
                     "  THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\n"
                     "  APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT\n"
                     "  HOLDERS AND/OR OTHER PARTIES PROIDE THE PROGRAM \"AS IS\" WITHOUT WARANTY\n"
                     "  OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,\n"
                     "  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"
                     "  PURPOSE.\n"
                     "This program is free software:\n"
                     "  You can redistribute it and/or modify it under the terms of the GNU\n"
                     "  General Public License as published by the Free Software Foundation,\n"
                     "  either version 3 of the License, or (at your option) any later version.\n"
                     "  You should have received a copy of the GNU General Public Licenses along\n"
                     "  with this program. If not, see <https://www.gnu.org/licenses/>");
                return 0;
        }
    }

    if (calibrate(measure_duration, average_amount) < 0)
    {
        return 1;
    }
    if (verbose)
    {
        printf("Calibrated to %dx%d iterations\n", average_amount, iterations);
    }

    g_minor_retries = 0;
    g_major_retries = 0;
    for (i = 0; i < ARRAYSIZE(procs); i++)
    {
        timings[i] = measure_precise(procs[i], average_amount);
        if (timings[i] == -1)
        {
            fprintf(stderr, "failed to get any reliable measurement at step %d\n", i);
            return 1;
        }
        if (i != 0)
        {
            long delta = timings[i] - timings[i-1];
            if (delta < INT_MIN || delta > INT_MAX)
            {
                fprintf(stderr, "Delta doesn't fit 16 bits: %ld\n", delta);
                return 1;
            }
            deltas[i-1] = (int)delta;
        }
    }
    if (verbose)
    {
        printf("Timing values: ");
        for (i = 0; i < ARRAYSIZE(timings); i++)
            printf("%ld ", timings[i]);
        printf("\nDelta values: ");
        for (i = 0; i < ARRAYSIZE(deltas); i++)
            printf("%d ", deltas[i]);
        puts("");
        printf("%d values rejected due to obvious interruptions\n", g_minor_retries);
        printf("%d measurement processes restarted due to guessed timer overflow\n", g_major_retries);
    }
    for (i = 0;i < ARRAYSIZE(deltas); i++)
    {
        if (deltas[i] < -(measure_duration >> 8))
        {
            fprintf(stderr, "negative step size %d at %d\n", deltas[i], i);
            fputs("To get raw data, re-run with /v", stderr);
            return 1;
        }
        if (deltas[i] >= (measure_duration >> 8))
        {
            // end of a step, and start of next step
            if (verbose)
            {
                if (step_begin == -1)
                {
                    printf("First full step starts at %d\n", i);
                }
                else
                {
                    printf("Step from %d to %d\n", step_begin, i);
                }
            }
            if (step_begin != -1)
            {
                int this_step_width = i - step_begin;
                if (step_width != -1 && this_step_width != step_width)
                {
                    fprintf(stderr, "Inconsistent step width: %d vs. %d\n",
                            this_step_width, step_width);
                    fputs("To get raw data, re-run with /v", stderr);
                    return 1;
                }
                step_width = this_step_width;
            }
            step_begin = i;
            if (labs(step_count * (long)deltas[i] - step_height_sum)
                > (measure_duration >> 8) * step_count)
            {
                fprintf(stderr, "Inconsistent step height: %d vs. %d\n",
                        deltas[i], step_height_sum / step_count);
                fputs("To get raw data, re-run with /v", stderr);
                return 1;
            }
            step_count++;
            step_height_sum += deltas[i];
        }
    }
    if (step_width == -1)
    {
        fprintf(stderr,
                "All timings approximately equal to %d, no information obtained\n",
                timings[0]);
        return -1;
    }
    fsb_khz = 1193L*average_amount*iterations*step_count/step_height_sum;
    printf("Detected clock: %d*%dMHz -> %dMHz\n", step_width, (int)((fsb_khz+500) / 1000), (int)((fsb_khz*step_width + 500) / 1000));
    return 0;
}