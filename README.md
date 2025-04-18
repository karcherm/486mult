486MULT
=======

Why is measuring the multiplier interesting?
--------------------------------------------
It is quite easy to measure the internal clock freuqency of a 486 processor, which may be 1 to 4
times the external front-side bus frequency. For system initialization, knowing the front-side bus
frequency is relevant, too. Typically, the mainboard BIOS of a 486 board uses the CPU indentifier
that you can find in DX after reset, or (for Cyrix CPUs) a cpu-specific register to find the CPU
model. On newer 486 processors, the CPUID instruction is also a possible way to detect the CPU model
information. Knowing the CPU model, the BIOS *typically* knows the multiplier and can calculate the
FSB frequency from the processor clock.

Most processors that can run at multiple clock mulitpliers report a different CPU ID. E.g. an Intel
80486DX4 jumpered for clock doubling instead of clock tripling identifies itself as 80486DX2 instead
of 80486DX4. There is one notable exception, though: The AMD 80486DX4 NV8T: It always identifies
itself with a code that typically indicates a 80486DX2. Just looking at the core clock, a BIOS can not
distinguish whether the processor operates at 3\*33MHz or 2\*50MHz.

Furthermore, it is hard to get the CPU identifier at runtime from user programs if the processor does
not support CPUID, as there is no universal interface in the BIOS that propagates the ID information in
DX to the operating system or application programs. So measuring the multiplier might be the easier way
to detect the CPU configuration.

The approach of this tool
-------------------------
If the 486 processor misses an read in the L1 cache, the execution unit is delayed until the data has
been fetched from the FSB. This means that after the execution of an instruction that causes an L1 miss,
the execution unit is synchronized with the FSB clock. Assuming a clock-tripled processor, executing
the instruction that misses the L1 cache will take zero, one or two extra CPU clocks for synchronization.

If you match up the core clock with the FSB at some point in the program, and then a re-synchronziation
happens at a later point, the execution of the whole fragment is always a whole number of core clocks. Thus
the execution time of the code between the two synchronization points is quantized to a multiple of 3 clocks
(for clock-trippled processors), a multiple of 2 clocks (for clock-doubled processors) or even a multiple
of four clocks (for clock-quadrupled processors like the Am5x86). This means if you can control how many
core cycles the intervening code needs, you should be able to observe the quantization effect.

This is exactly what this tool does: It places a different number of NOPs in a loop that needs to resynchronzie
on each iteration, and then measures the execution time for different NOP counts. On a processor without clock
multiplication, the loop gets slower for every NOP included in it. For clock-doubled processors, the loop only
gets slower every 2 inserted nops and so on.

Testing results
---------------
This tool has been tested to successfully identify the multiplier of a great variety of processors,
but I didn't verify whether the detection works perfectly at all L2/RAM timings. The tool fails if L1 cache
is disabled, likely because this causes extra synchronization. The tool also fails if the chipset is not
willing to execute RAM cycles on every FSB clock (which seems to be the case in de-turbo mode on the UMC8881).

The following processors had their multiplier correctly identified:
- Intel 486DX (1x)
- Intel 486DX2 (2x)
- Cyrix Cx486 (1x)
- Cytix Cx486DX4 (2x/3x)
- AMD Am486DX4 SV8B (2x/3x)
- AMD 5x86 "ADZ" (3x/4x)

The tool failed on a Cx5x86, and errorneously reported 1x multiplier, which is likely due to the advanced
architecture providing better decoupling from the FSB and instruction execution.