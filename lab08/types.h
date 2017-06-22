#ifndef __TYPES_H__
#define __TYPES_H__

#ifndef FALSE
#define FALSE 0x00
#endif

#ifndef TRUE
#define TRUE 0x01
#endif

#define BV(bit) (1 << (bit))
#define CLEARBIT(BIT)		(BIT = 0)
#define SETBIT(BIT)		(BIT = 1)
#define TOGGLEBIT(BIT) 		(BIT ^= 1)
#define CLEARLED(BIT)		BIT = 0; Nop();
#define SETLED(BIT)		BIT = 1; Nop();
#define TOGGLELED(BIT) 		BIT ^= 1; Nop();

typedef unsigned char uint8_t;		/**< Unsigned 8 bit intgerer. */
typedef unsigned short int uint16_t;	/**< Unsigned 16 bit intgerer. */
typedef unsigned long int uint32_t;	/**< Unsigned 32 bit intgerer. */
typedef signed char int8_t;		/**< signed 8 bit intgerer. */
typedef signed short int int16_t;	/**< signed 16 bit intgerer. */
typedef signed long int int32_t;	/**< signed 32 bit intgerer. */

typedef struct {
    uint16_t x_min;
    uint16_t x_max;
    uint16_t y_min;
    uint16_t y_max;
} extrema_t;

typedef struct {
    int16_t error;
    int16_t integral;
    int16_t derivative;
    int16_t output;
    int16_t previous_error;
    double kp;
    double ki;
    double kd;
} pid_control_t;

#endif //__TYPES_H__
