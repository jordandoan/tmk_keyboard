#include <stdbool.h>
#include <avr/io.h>
#include "wait.h"

/* Check port settings for clock and data line */
#if !(defined(IBMPC_CLOCK_PORT) && \
      defined(IBMPC_CLOCK_PIN) && \
      defined(IBMPC_CLOCK_DDR) && \
      defined(IBMPC_CLOCK_BIT))
#   error "clock port setting is required in config.h"
#endif

#if !(defined(IBMPC_DATA_PORT) && \
      defined(IBMPC_DATA_PIN) && \
      defined(IBMPC_DATA_DDR) && \
      defined(IBMPC_DATA_BIT))
#   error "data port setting is required in config.h"
#endif


/*
 * Clock
 */
void clock_init(void)
{
    IBMPC_CLOCK_PORT &= ~(1<<IBMPC_CLOCK_BIT);
    IBMPC_CLOCK_DDR  |=  (1<<IBMPC_CLOCK_BIT);
}

void clock_lo(void)
{
    IBMPC_CLOCK_PORT &= ~(1<<IBMPC_CLOCK_BIT);
    IBMPC_CLOCK_DDR  |=  (1<<IBMPC_CLOCK_BIT);
}

void clock_hi(void)
{
    /* input with pull up */
    IBMPC_CLOCK_DDR  &= ~(1<<IBMPC_CLOCK_BIT);
    IBMPC_CLOCK_PORT |=  (1<<IBMPC_CLOCK_BIT);
}

bool clock_in(void)
{
    IBMPC_CLOCK_DDR  &= ~(1<<IBMPC_CLOCK_BIT);
    IBMPC_CLOCK_PORT |=  (1<<IBMPC_CLOCK_BIT);
    wait_us(1);
    return IBMPC_CLOCK_PIN&(1<<IBMPC_CLOCK_BIT);
}

/*
 * Data
 */
void data_init(void)
{
    IBMPC_DATA_DDR  &= ~(1<<IBMPC_DATA_BIT);
    IBMPC_DATA_PORT |=  (1<<IBMPC_DATA_BIT);
}

void data_lo(void)
{
    IBMPC_DATA_PORT &= ~(1<<IBMPC_DATA_BIT);
    IBMPC_DATA_DDR  |=  (1<<IBMPC_DATA_BIT);
}

void data_hi(void)
{
    /* input with pull up */
    IBMPC_DATA_DDR  &= ~(1<<IBMPC_DATA_BIT);
    IBMPC_DATA_PORT |=  (1<<IBMPC_DATA_BIT);
}

bool data_in(void)
{
    IBMPC_DATA_DDR  &= ~(1<<IBMPC_DATA_BIT);
    IBMPC_DATA_PORT |=  (1<<IBMPC_DATA_BIT);
    wait_us(1);
    return IBMPC_DATA_PIN&(1<<IBMPC_DATA_BIT);
}
