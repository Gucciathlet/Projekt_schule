//Projekt der Schule für Lüftungs Modell

#include "TouchScreen.h"                                //touch library
#include "LCDWIKI_GUI.h"                                //Core graphics library
#include "LCDWIKI_KBV.h"                                //Hardware-specific library
#include "SimpleDHT.h"                                  //Temp / humidity sensor
#include "switch_font.c"                                //Writing font and imported Pictures
LCDWIKI_KBV my_lcd(ILI9486,A3,A2,A1,A0,A4);             // Length and width for Display

//Defines the diffrent colours
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//Defines the Background Colours
#define BACKGROUND RED
#define MENUE_COLOUR BLUE

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define YP A3                                           // must be an analog pin, use "An" notation!
#define XM A2                                           // must be an analog pin, use "An" notation!
#define YM 9                                            //9 can be a digital pin
#define XP 8                                            //8 can be a digital pin

#define TS_MINX 906
#define TS_MAXX 116

#define TS_MINY 92
#define TS_MAXY 952

// We have a status line for like, is FONA working
#define STATUS_X 10
#define STATUS_Y 65

//PWM Motor pins
#define pwmPin1 44
#define pwmPin2 45


//Menue shit 
#define menue_options 7                                 //Ammount of Options for first Menue
#define info_menue_options 3                            //Ammount of Options for Info Menue (Temp,light, etc.)
#define menue_Xoffset 15                                //Offset for Menue to the right 
bool switch_flag1 = false;              
bool menue_toggle[menue_options];                       //Array, if menue option is toggled 

//Temp, humidity Sensor
#define pinBrightness 49                                //Input Pin for Brightness 
#define pinDHT22 47                                     //Data Pin for Temp, Humidity Sensor                                       
SimpleDHT22 dht22(pinDHT22);  
char temp[10];                                          //string for Temp output 
char hum[10];                                           //string for humidity output

//system time 
unsigned long time;                                     //Time for On/off buttons
unsigned long time1;                                    //Time for Temp, Humidity Sensor
#define break_time 350


TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//Displays a string
void show_string(uint8_t *str,int16_t x,int16_t y,uint8_t csize,uint16_t fc, uint16_t bc,bool mode){
    my_lcd.Set_Text_Mode(mode);
    my_lcd.Set_Text_Size(csize);
    my_lcd.Set_Text_colour(fc);
    my_lcd.Set_Text_Back_colour(bc);
    my_lcd.Print_String(str,x,y);
    }

//Displays a picture
void show_picture(const uint8_t *color_buf,int16_t buf_size,int16_t x1,int16_t y1,int16_t x2,int16_t y2){
    my_lcd.Set_Addr_Window(x1, y1, x2, y2); 
    my_lcd.Push_Any_Color(color_buf, buf_size, 1, 1);
    }

//Check whether to press or not
bool is_pressed(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t px,int16_t py){
    if((px > x1 && px < x2) && (py > y1 && py < y2))
    {
        return true;  
    } 
    else
    {
        return false;  
    }
}

void setup(void) 
{    
    //Initialize Screen
    Serial.begin(9600);                                     //Communication Refresh rate
    my_lcd.Init_LCD();                                          
    Serial.println(my_lcd.Read_ID(), HEX);                  //LCD display

    //set Input / output pin
    pinMode(pinBrightness, INPUT);                          //For brightness

    //Colour of the Background 
    my_lcd.Fill_Screen(BACKGROUND); 

    //Declarations
    time = 0;                                               //time for on/off buttons
    time1 = 0;                                              //time for Temp, Humidity Sensor


    char* menue_names [menue_options]=
        {
            "Setting 1", 
            "Setting 2", 
            "Setting 3", 
            "Setting 4", 
            "Setting 5",
            "Setting 6",
            "Setting 7"
        };
    
    char* info_menue[info_menue_options]=
        {
            "Temperatur:",
            "Feuchte   :",
            "Modus     :"
        };


    //Show title
    show_string("Steuerung", 40, 5, 4, BLACK, BLACK, true);

    //Draw Settings-Menue with lines inbetween
    for (int a = 0; a < menue_options; a++)
    {
        show_string(menue_names[a], menue_Xoffset, 30 * (a + 1) + 40, 3, BLACK, BLACK, true);
        my_lcd.Set_Draw_color(BLACK);
        my_lcd.Draw_Fast_HLine( 0, 30 * (a + 1) + 65, my_lcd.Get_Display_Width());
        show_picture(switch_off_2, sizeof(switch_off_2)/2, menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 269, 30 * (a + 1) + 64);

        //Filling array with false (offline) for diffrent functions
        menue_toggle[a] = false;
    }

    //Draw Info-Menue (temp, feuchtigkeit, helligkeit)
    for (int a = 0; a < info_menue_options; a++)
    {
        show_string(info_menue[a], menue_Xoffset, 30 * (a + 1) + (menue_options * 30) + 60, 3, BLACK, BLACK, true);
    }

}


void loop(void)
{
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);

    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    //First function 
    if (menue_toggle[1] == true)
    {

    }
    

    //Temp, Humidity Sensor
    float temperature = 0;
    float humidity = 0;
    int brightness = 0;
    int err = SimpleDHTErrSuccess;

    if (time1 + 2500 <= millis())
    {
        //draw over old input a rectangle
        my_lcd.Set_Draw_color(BACKGROUND);
        my_lcd.Fill_Rectangle(menue_Xoffset + 197, 30 * 1 + (menue_options * 30) + 60, menue_Xoffset + 300, 30 * 1 + (menue_options * 30) + 85);    //Temp overwrite
        my_lcd.Fill_Rectangle(menue_Xoffset + 197, 30 * 2 + (menue_options * 30) + 60, menue_Xoffset + 300, 30 * 2 + (menue_options * 30) + 85);    //Hum overwrite
        my_lcd.Fill_Rectangle(menue_Xoffset + 197, 30 * 3 + (menue_options * 30) + 60, menue_Xoffset + 300, 30 * 3 + (menue_options * 30) + 85);    //Bright. overwrite
        
        //if error appears suddenly 
        if ((err = dht22.read2 (&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) 
        {
            show_string("err", menue_Xoffset + 197, 30 * 1 + (menue_options * 30) + 60, 3, BLACK, BLACK, true);
            delay(2000);
        }                               
        
        //write Float into string 
        //dtostrf(float, minWidth, afterDec, buf)
        dtostrf(humidity, 3, 1, hum);
        strcat(hum," %");
        dtostrf(temperature, 3, 1, temp);
        strcat(temp," C");
        
        Serial.print(temp);                                                                                             // Console output temp
        Serial.print(hum);                                                                                              // Console output humidity
        show_string(temp, menue_Xoffset + 197, 30 * 1 + (menue_options * 30) + 60, 3, BLACK, BLACK, true);              //Temp write
        show_string(hum, menue_Xoffset + 197, 30 * 2 + (menue_options * 30) + 60, 3, BLACK, BLACK, true);               //hum write

        //brightness Sensor 
        brightness = digitalRead(pinBrightness);
        if (brightness == 0)
        {
            show_string("Tag", menue_Xoffset + 197, 30 * 3 + (menue_options * 30) + 60, 3, BLACK, BLACK, true);         //Day write
        }
        else
        {
            show_string("Nacht", menue_Xoffset + 197, 30 * 3 + (menue_options * 30) + 60, 3, BLACK, BLACK, true);       //Night write
        }
        time1 = millis();
    }
    


    //Checks for system time and if pressed with the right pressure 
    if (time + break_time <= millis() && p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {
        p.x = map(p.x, TS_MINX, TS_MAXX, my_lcd.Get_Display_Width(),0);
        p.y = map(p.y, TS_MINY, TS_MAXY, my_lcd.Get_Display_Height(),0);

        //Area where to press
        for (int a = 0; a < menue_options; a++)
        {
            if(is_pressed(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 269, 30 * (a + 1) + 64, p.x,p.y))
            {
                
                if(switch_flag1)
                    {   
                        show_picture(switch_off_2, sizeof(switch_off_2)/2, menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 269, 30 * (a + 1) + 64);
                        switch_flag1 = false;
                    }
                    else
                    {  
                        show_picture(switch_on_2, sizeof(switch_on_2)/2, menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 269, 30 * (a + 1) + 64);
                        switch_flag1 = true;
                        
                    }
                // get system time 
                time = millis();
            }
        }
    }
}
