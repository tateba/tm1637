/*
* @file     tmlib.c 
* @brief    TM1637 C library
* @author   Theodore ATEBA
* @version  1.0
* @date 04  Jully 2015
*/

//=============================================================================
// Include Files
//=============================================================================
    #include "tmlib.h"

    // Declaration of brightness
    uint8_t brightness;

    
const uint8_t digitToSegments[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // b
    0x39, // C
    0x5E, // d
    0x79, // E
    0x71, // F
};


//=============================================================================
// Functions
//=============================================================================

/*
 * @fn      void bitDelay (void)
 * @brief   Width of a bit on DIO bus
 */
void bitDelay (void)
{
    usDelay (50);
}

/*
 * @fn      void start (void)
 * @brief   Generation of the start condition on the DIO bus.
 */
void start (void)
{
    palSetPadMode(GPIOC, DIO_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    bitDelay();
}

/*
 * @fnn     void stop (void)
 * @brief   Generation of the stop condition on the DIO bus.
 */
void stop (void)
{
    palSetPadMode(GPIOC, DIO_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    bitDelay();
    palSetPadMode(GPIOC, CLK_PIN, PAL_MODE_INPUT_PULLUP);
    bitDelay();
    palSetPadMode(GPIOC, DIO_PIN, PAL_MODE_INPUT_PULLUP);
    bitDelay();
}

/*
 * @fn          uint8_t encodeDigit (uint8_t digit)
 * @brief       Encode the data to print on the 4-digit Display
 * @param[in]   digit the data to convert for the Display
 */
uint8_t encodeDigit(uint8_t digit)
{
    return digitToSegments [digit & 0x0F];
}

/*
 * @fn          void digitBrightness (uint8_t brightness)
 * @brief       set the brightness of the 4-Digit Display.
 * @note        0 is the dimmest and 255 is the brightest
 * @note        set before calling four_digit_display
 * @param[in]   brightness The level of the brightness we want to set
 */
void setBrightness (uint8_t  bright)
{
    brightness = bright;
}
    
/*
 * @fn          bool writeByte (uint8_t byte)
 * @brief       write a byte to the GPIO pin used as DIO
 * @param[in]   byte Data to write on the DIO pin
 */
bool writeByte(uint8_t byte)
{
    uint8_t data = byte;
    uint8_t i;

    // 8 Data bits
    for(i = 0; i < 8; i++)
    {
        // CLK low
        palSetPadMode(GPIOC, CLK_PIN, PAL_MODE_OUTPUT_PUSHPULL);
        bitDelay ();

        // Set data bit
        if(data & 0x01)
            palSetPadMode(GPIOC, DIO_PIN, PAL_MODE_INPUT_PULLUP);
        else
            palSetPadMode(GPIOC, DIO_PIN, PAL_MODE_OUTPUT_PUSHPULL);

        bitDelay ();

        // CLK high
        palSetPadMode(GPIOC, CLK_PIN, PAL_MODE_INPUT_PULLUP);
        bitDelay ();
        data = data >> 1;
    }

    // Wait for Acknowlege
    // CLK to Zero
    palSetPadMode(GPIOC, CLK_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOC, DIO_PIN, PAL_MODE_INPUT_PULLUP);
    bitDelay ();

    // CLK high
    palSetPadMode(GPIOC, CLK_PIN, PAL_MODE_INPUT_PULLUP);
    bitDelay ();
    uint8_t ack = palReadPad(GPIOC, DIO_PIN);
    if(ack == 0)
        palSetPadMode(GPIOC, DIO_PIN, PAL_MODE_OUTPUT_PUSHPULL);

    bitDelay ();
    palSetPadMode(GPIOC, CLK_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    bitDelay();

    return ack;
}

//void four_digit_clear (void)

/*
 * @fn          void setSegments (const uint8_t segments[], uint8_t length, uint8_t pos)
 * @brief       print the data on the segments
 * @param[in]   segments array of data to print on the 4-digit Display
 * @param[in]   length Size of the array of data to print
 * @param[in]   pos Position of the first digit to update
 */
void setSegments (const uint8_t segments[], uint8_t length, uint8_t pos)
{
    uint8_t i;

    // Write COMM1
    start ();
    writeByte (TM1637_I2C_COMM1);
    stop ();

    // Wirte COMM2 + first digit address
    start ();
    writeByte (TM1637_I2C_COMM2 + (pos & 0x03));
        
    // Write the data byte
    for(i = 0; i < length; i++)
        writeByte (segments [i]);
    stop ();

    // Write COMM3 + Brightness
    start ();
    writeByte (TM1637_I2C_COMM3 + (brightness & 0x0F));
    stop ();
}

/*
 * @fn          void showNumberDec (uint16_t num, bool leading_zero, uint8_t length, uint8_t pos)
 * @brief       Print Decimal number on the 4-Digit Display
 * @param[in]   num number to print
 * @param[in]   leading_zero 
 * @param[in]   length
 * @param[in]   pos position of the digit
 */
void showNumberDec(uint16_t num, bool leading_zero, uint8_t length, uint8_t pos)
{
    uint8_t digits[4], i;
    const uint16_t divisors[] = {1, 10, 100, 1000};
    bool leading = true;

    for(i = 0; i < 4; i++)
    {
        uint8_t divisor = divisors[4 - 1 - i];
        uint8_t d = num / divisor;

        if( d == 0)
        {
            if( leading_zero || !leading || (i == 3))
                digits[i] = encodeDigit(d);
            else
                digits[i] = 0;
        }
        else
        {
            digits[i] = encodeDigit(d);
            num -= d * divisor;
            leading = false;
        }
    }

    setSegments (digits + (4 - length), length, pos);
}
