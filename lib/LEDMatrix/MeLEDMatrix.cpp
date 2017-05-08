/**
 * \par Copyright (C), 2012-2015, MakeBlock
 * \class   MeLEDMatrix
 * \brief   Driver for Me LED Matrix module.
 * @file    MeLEDMatrix.cpp
 * @author  MakeBlock
 * @version V1.0.3
 * @date    2016/01/29
 * @brief   Driver for Me LED Matrix module.
 *
 * \par Copyright
 * This software is Copyright (C), 2012-2015, MakeBlock. Use is subject to license \n
 * conditions. The main licensing options available are GPL V2 or Commercial: \n
 *
 * \par Open Source Licensing GPL V2
 * This is the appropriate option if you want to share the source code of your \n
 * application with everyone you distribute it to, and you also want to give them \n
 * the right to share who uses it. If you wish to use this software under Open \n
 * Source Licensing, you must contribute all your source code to the open source \n
 * community in accordance with the GPL Version 2 when your application is \n
 * distributed. See http://www.gnu.org/copyleft/gpl.html
 *
 * \par Description
 * This file is a drive for Me LED Matrix device
 *
 * \par Method List:
 *
 *    1.    void MeLEDMatrix::clearScreen();
 *    2.    void MeLEDMatrix::setBrightness(uint8_t Bright);
 *    3.    void MeLEDMatrix::setColorIndex(bool Color_Number);
 *    4.    void MeLEDMatrix::drawBitmap(int8_t x, int8_t y, uint8_t Bitmap_Width, uint8_t *Bitmap);
 *    5.    void MeLEDMatrix::drawStr(int16_t X_position, int8_t Y_position, const char *str);
 *    6.    void MeLEDMatrix::showClock(uint8_t hour, uint8_t minute, bool point_flag);
 *    7.    void MeLEDMatrix::showNum(float value,uint8_t digits);
 *    8.    void MeLEDMatrix::reset(uint8_t port);
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * forfish         2015/11/11     1.0.0            Add description
 * Mark Yan        2016/01/19     1.0.1            Add some new symbol
 * Mark Yan        2016/01/27     1.0.2            Add digital printing
 * Mark Yan        2016/01/29     1.0.3            Fix issue when show integer number
 * </pre>
 *
 */
 
#include "MeLEDMatrix.h"

/**
 * Alternate Constructor which can call your own function to map the LED Matrix to arduino port,
 * no pins are used or initialized here.
 * \param[in]
 *   None
*/
MeLEDMatrix::MeLEDMatrix()
{

}


/**
 * Alternate Constructor which can call your own function to map the Buzzer to arduino port,
 * you can set any slot for the buzzer device. 
 * \param[in]
 *   SCK_Pin - The SCk of LED Matrix.
 * \param[in]
 *   DIN_Pin - Put in parameter.
 */
MeLEDMatrix::MeLEDMatrix(uint8_t SCK_Pin, uint8_t DIN_Pin)
{
	u8_SCKPin = SCK_Pin; 
	u8_DINPin = DIN_Pin;

	pinMode(u8_SCKPin, OUTPUT);
	pinMode(u8_DINPin, OUTPUT);
	digitalWrite(u8_SCKPin,HIGH);
	digitalWrite(u8_DINPin,HIGH);

    writeByte(Mode_Address_Auto_Add_1);
    setBrightness(Brightness_5);
    clearScreen();
}


/**
 * \par Function
 *    writeByte
 * \par Description
 *    Write byte to LED Matrix.
 * \param[in]
 *    data - The data wrote to LED Matrix.
 * \par Output
 *    None
 * \Return
 *    None.
 * \par Others
 *    None
 */
void MeLEDMatrix::writeByte(uint8_t data)
{
    //Start
    digitalWrite(u8_SCKPin, HIGH);
    digitalWrite(u8_DINPin, LOW);

    for(char i=0;i<8;i++)
    {
        digitalWrite(u8_SCKPin, LOW);
        digitalWrite(u8_DINPin, (data & 0x01));
        digitalWrite(u8_SCKPin, HIGH);
        data = data >> 1;
    }

    //End
    digitalWrite(u8_SCKPin, LOW);
    digitalWrite(u8_DINPin, LOW);
    digitalWrite(u8_SCKPin, HIGH);
    digitalWrite(u8_DINPin, HIGH);
    // delayMicroseconds(1);
}

/**
 * \par Function
 *    writeBytesToAddress
 * \par Description
 *    Write byte to LED Matrix's address.
 * \param[in]
 *    Address - The address you want to write in LED Matrix.
 * \param[in]
 *    P_data - The pointer points to data.
 * \param[in]
 *    count_of_data - The length of data.
 * \par Output
 *    None
 * \Return
 *    None.
 * \par Others
 *    None
 */
void MeLEDMatrix::writeBytesToAddress(uint8_t Address, const uint8_t *P_data, uint8_t count_of_data)
{
    uint8_t T_data;

    if(Address > 15 || count_of_data==0)
        return;

    Address = ADDRESS(Address);

    //Start
    digitalWrite(u8_SCKPin, HIGH);
    digitalWrite(u8_DINPin, LOW);

    //write Address
    for(char i=0;i<8;i++)
    {
        digitalWrite(u8_SCKPin, LOW);
        digitalWrite(u8_DINPin, (Address & 0x01));
        digitalWrite(u8_SCKPin, HIGH);
        Address = Address >> 1;
    }


    //write data
    for(uint8_t k=0; k<count_of_data; k++)
    {
        T_data = *(P_data + k);

        for(char i=0;i<8;i++)
        {
            digitalWrite(u8_SCKPin, LOW);
            digitalWrite(u8_DINPin, (T_data & 0x80));
            digitalWrite(u8_SCKPin, HIGH);
            T_data = T_data << 1;
        }
    }

    //End
    digitalWrite(u8_SCKPin, LOW);
    digitalWrite(u8_DINPin, LOW);
    digitalWrite(u8_SCKPin, HIGH);
    digitalWrite(u8_DINPin, HIGH);
    // delayMicroseconds(1);
}

/**
 * \par Function
 *    clearScreen
 * \par Description
 *    Clear the screen.
 * \param[in]
 *    None
 * \par Output
 *    None
 * \Return
 *    None.
 * \par Others
 *    None
 */
void MeLEDMatrix::clearScreen()
{
    for(uint8_t i=0;i<LED_BUFFER_SIZE;i++)
    {
        u8_Display_Buffer[i] = 0x00;
    }

    b_Color_Index = 1;
    b_Draw_Str_Flag = 0;

    writeBytesToAddress(0,u8_Display_Buffer,LED_BUFFER_SIZE);
}

/**
 * \par Function
 *    setBrightness
 * \par Description
 *    Set the brightness of LED Matrix.
 * \param[in]
 *    Bright - The brightness of LED Matrix.
 * \par Output
 *    None
 * \Return
 *    None.
 * \par Others
 *    None
 */
void MeLEDMatrix::setBrightness(uint8_t Bright)
{
    if((uint8_t)Bright>8)
    {
        Bright = Brightness_8;
    }

    if((uint8_t)Bright != 0)
    {
        Bright = (LED_Matrix_Brightness_TypeDef)((uint8_t)(Bright-1)|0x08);
        
    }
    writeByte(0x80 | (uint8_t)Bright);

}

/**
 * \par Function
 *    setColorIndex
 * \par Description
 *    Set the color index for LED Matrix.
 * \param[in]
 *    Color_Number - The number of LED Matrix's color.
 * \par Output
 *    None
 * \Return
 *    None.
 * \par Others
 *    None
 */
void MeLEDMatrix::setColorIndex(bool Color_Number)
{
    b_Color_Index = Color_Number;
}

/**
 * \par Function
 *    drawBitmap
 * \par Description
 *    Draw a bitmap.
 * \param[in]
 *    x - The x coordinate of bitmap.
 * \param[in]
 *    y - The y coordinate of bitmap.
 * \param[in]
 *    Bitmap_Width - The width of bitmap.
 * \param[in]
 *    Bitmap - A pointer to bitmap.
 * \par Output
 *    None
 * \Return
 *    None.
 * \par Others
 *    None
 */
void MeLEDMatrix::drawBitmap(int8_t x, int8_t y, uint8_t Bitmap_Width, uint8_t *Bitmap)
{

    if(x>15 || y>7 || Bitmap_Width==0)
        return;


    if(b_Color_Index == 1)
    {
        for(uint8_t k=0;k<Bitmap_Width;k++)
        {
          if(x+k>=0){
            u8_Display_Buffer[x+k] = (u8_Display_Buffer[x+k] & (0xff << (8-y))) | (y>0?(Bitmap[k] >> y):(Bitmap[k] << (-y)));
          }
        }
    }
    else if(b_Color_Index == 0)
    {
        for(uint8_t k=0;k<Bitmap_Width;k++)
        {
            if(x+k>=0){
              u8_Display_Buffer[x+k] = (u8_Display_Buffer[x+k] & (0xff << (8-y))) | (y>0?(~Bitmap[k] >> y):(~Bitmap[k] << (-y)));
            }
        }
    }

    writeBytesToAddress(0,u8_Display_Buffer,LED_BUFFER_SIZE);
}

