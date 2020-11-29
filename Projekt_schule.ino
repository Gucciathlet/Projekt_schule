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
#define pwmPin1 44                                      //Motor Pin 1
#define pwmPin2 45                                      //Motor Pin 2
#define pwmPin3 46                                      //Motor Pin 3

//Digital pins
#define pinDigital1 22                                     //Pin 1  Ventilator
#define pinDigital2 23                                     //Pin 2  
#define pinDigital3 24                                     //Pin 3  light
#define pinDigital4 25                                     //Pin 4  Watering
#define pinDigital5 26                                     //Pin 5
#define pinDigital6 27                                     //Pin 6
#define pinDigital7 28                                     //Pin 7
#define pinDigital8 29                                     //Pin 8
#define pinDigital9 30                                     //Pin 9


//Menue shit 
#define menue_options 5                                 //Ammount of Options for first Menue
#define info_menue_options 4                            //Ammount of Options for Info Menue (Temp,light, etc.)
#define mode_menue_options 5                            //Ammount of Options for Mode Menue (Plant type)


const char* menue_names [menue_options]=
{
    "Ventilator", 
    "Dach", 
    "Licht", 
    "Wasser", 
    "Automatik"
};
    
const char* info_menue[info_menue_options]=
{
    "Temperatur:",
    "Feuchte   :",
    "Modus     :",
    "Laufzeit  :"
};

const char* mode_menue[mode_menue_options] =
{
    "Tomate",
    "Zuchini",
    "Ananas",
    "Gras",
    "Baum"
};

#define menue_Xoffset 15                                //Offset for Menue to the right 

//Toggle 
bool menue_toggle[menue_options];                       //Array, what Menue is toggled
bool mode_menue_toggle[mode_menue_options];             //Array, what mode is selected
bool old_flag[menue_options];                           // to only send change when actually pressed the button
bool mode_menue_flag = false;                           //check if mode menue is active
int mode_menue_select = 0;                              //saves the chosen menue for the automatic Mode

//Temp, humidity Sensor
#define pinBrightness 49                                //Input Pin for Brightness 
#define pinDHT22 51                                     //Data Pin for Temp, Humidity Sensor                                       
SimpleDHT22 dht22(pinDHT22);  
char temp[10];                                          //string for Temp output 
char hum[10];                                           //string for humidity output
char runt[10];                                          //string for runtime ouput 

//Temp, Humidity Sensor
float temperature = 0;
float humidity = 0;
int brightness = 0;
int err = SimpleDHTErrSuccess;
float runtime = 0;


//system time 
unsigned long time  = 0;                                //Time for On/off buttons
unsigned long time1 = 0;                                //Time for Temp, Humidity Sensor
unsigned long time2 = 0;                                //Time for Mode menue
unsigned long time3 = 0;                                //Time for Active Automatic Menue
unsigned long time4 = 0;                                //Time for Watering 
#define break_time 350                                  //Break time between each input

typedef struct 
{
    float temperature;
    float humidity;
    int light_time;
    int watering_time;
    int watering_break;
}plant_modes;

plant_modes autonom[mode_menue_options];


TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//Displays a string
void show_string(uint8_t* const str,int16_t x,int16_t y,uint8_t csize,uint16_t fc, uint16_t bc,bool mode){
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
    //diffrent preconfigured modes
    //First mode
    //temp, hum, light_time, watering_time, watering_break
    autonom[0] = {23.0, 80.0, 30000, 5000, 30000};
    autonom[1] = {23.0, 80.0, 30000, 5000, 30000};
    autonom[2] = {23.0, 80.0, 30000, 5000, 30000};
    autonom[3] = {23.0, 80.0, 30000, 5000, 30000};
    autonom[4] = {23.0, 80.0, 30000, 5000, 30000};
    



    //Initialize Screen
    Serial.begin(9600);                                     //Communication Refresh rate
    my_lcd.Init_LCD();                                      //Initialize LCD Display
    Serial.println(my_lcd.Read_ID(), HEX);                  //Write LCD display ID

    //Pin modes
    pinMode(pinDigital1, OUTPUT);                           //Output for Vent
    pinMode(pinBrightness, INPUT);                          //Input for Brightness

    //Colour of the Background 
    my_lcd.Fill_Screen(BACKGROUND); 

    //Show title
    show_string("Mode>", menue_Xoffset, 5, 4, BLACK, BLACK, true);


    //Draw Settings-Menue with lines inbetween

    for (int a = 0; a < menue_options; a++)
    {
        show_string(menue_names[a], menue_Xoffset, 30 * (a + 1) + 40, 3, BLACK, BLACK, true);
        my_lcd.Set_Draw_color(BLACK);
        my_lcd.Draw_Fast_HLine( 0, 30 * (a + 1) + 65, my_lcd.Get_Display_Width());
        show_picture(switch_off_2, sizeof(switch_off_2)/2, menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 269, 30 * (a + 1) + 64);

        //Filling array with false (offline) for diffrent functions
        menue_toggle[a] = false;
        old_flag[a] = false;
    }

    //Draw Info-Menue (temp, humidity, brightness)
    for (int a = 0; a < info_menue_options; a++)
    {
        show_string(info_menue[a], menue_Xoffset, 30 * (a + 1) + (menue_options * 30) + 60, 3, BLACK, BLACK, true);
    }

    //set mode menue to false (offline)
    for (int a = 0; a < mode_menue_options; a++)
    {
        mode_menue_toggle[a] = false;
    }

}
//read and write temp / hum and light 
void temp_hum()
{

    if (time1 + 2500 <= millis())
    {
        //draw over old input a rectangle
        my_lcd.Set_Draw_color(BACKGROUND);
        my_lcd.Fill_Rectangle(menue_Xoffset + 197, 30 * 1 + (menue_options * 30) + 60, menue_Xoffset + 300, 30 * 1 + (menue_options * 30) + 85);    //Temp overwrite
        my_lcd.Fill_Rectangle(menue_Xoffset + 197, 30 * 2 + (menue_options * 30) + 60, menue_Xoffset + 300, 30 * 2 + (menue_options * 30) + 85);    //Hum overwrite
        my_lcd.Fill_Rectangle(menue_Xoffset + 197, 30 * 3 + (menue_options * 30) + 60, menue_Xoffset + 300, 30 * 3 + (menue_options * 30) + 85);    //Bright. overwrite
        my_lcd.Fill_Rectangle(menue_Xoffset + 197, 30 * 4 + (menue_options * 30) + 60, menue_Xoffset + 300, 30 * 4 + (menue_options * 30) + 85);    //Runtime overwrite

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
        
        //Serial.print(temp);                                                                                             // Console output temp
        //Serial.print(hum);                                                                                              // Console output humidity
        show_string(temp, menue_Xoffset + 197, 30 * 1 + (menue_options * 30) + 60, 3, BLACK, BLACK, true);              //Temp write
        show_string(hum,  menue_Xoffset + 197, 30 * 2 + (menue_options * 30) + 60, 3, BLACK, BLACK, true);               //hum write

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

        runtime = ((millis() / 1000) / 60);
        dtostrf(runtime, 3, 1, runt);
        strcat(runt, " m");

        show_string(runt, menue_Xoffset + 197, 30 * 4 + (menue_options * 30) + 60, 3, BLACK, BLACK, true);              //write Runtime 
        time1 = millis();
    }
}



void loop(void)
{

    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);


    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    
    temp_hum();                                 //Read and Write Temp, hum and light
    

    // first option manual mode
    if (menue_toggle[0] != old_flag[0] && !menue_toggle[4])
    {
        if (menue_toggle[0])
        {   
            digitalWrite(pinDigital1, HIGH);
            Serial.print("First Option ON\n");
            old_flag[0] = menue_toggle[0];
        }
        else
        {
            digitalWrite(pinDigital1, LOW);
            Serial.print("First Option OFF - DigitalPin1\n");
            old_flag[0] = menue_toggle[0];
        }
    }

    // second option manual mode
    if (menue_toggle[1] != old_flag[1] && !menue_toggle[4])
    {
        if (menue_toggle[1])
        {   
            analogWrite(pwmPin2, 50 );                   //one motor
            analogWrite(pwmPin3, 250);                   //second one on other side
            Serial.print("Second Option ON - pwmPin2 + 3\n");
            old_flag[1] = menue_toggle[1];
        }
        else
        {
            analogWrite(pwmPin2, 250);                  //one motor
            analogWrite(pwmPin3, 50 );                  //second one on other side
            Serial.print("Second Option OFF - pwmPin2 + 3\n");
            old_flag[1] = menue_toggle[1];
        }
    }

    // third option manual mode
    if (menue_toggle[2] != old_flag[2] && !menue_toggle[4])
    {
        if (menue_toggle[2])
        {   
            digitalWrite(pinDigital3, HIGH);
            Serial.print("Third Option ON - DigitalPin3\n");
            old_flag[2] = menue_toggle[2];
        }
        else
        {
            digitalWrite(pinDigital3, LOW);
            Serial.print("Third Option OFF - DigitalPin3\n");
            old_flag[2] = menue_toggle[2];
        }
    }

    // fourth option manual mode 
    if (menue_toggle[3] != old_flag[3] && !menue_toggle[4])
    {
        if (menue_toggle[3])
        {   
            digitalWrite(pinDigital4, HIGH);
            Serial.print("Fourth Option ON - DigitalPin4\n");
            old_flag[3] = menue_toggle[3];
        }
        else
        {
            digitalWrite(pinDigital4, LOW);
            Serial.print("Third Option OFF - DigitalPin4\n");
            old_flag[3] = menue_toggle[3];
        }
    }

    //temp, hum, light_time, watering_time, watering_break
    //5th option toggle, Activates the automatic mode 
    if(menue_toggle[4] != old_flag[4])
    {
        //check if temp or hum is too high
        if ((temperature > autonom[mode_menue_select].temperature || humidity > autonom[mode_menue_select].humidity) && time3 + 1000 < millis())
        {
            //activate vent
            digitalWrite(pinDigital1, HIGH);
            //activate motor for Window opener
            analogWrite(pwmPin2, 50);
            Serial.print("TEMP/HUM zu hoch\n");
            time3 = millis();
        }
        //if Temp and hum is in normal state then vent off and motor back to close position
        else if ((temperature < autonom[mode_menue_select].temperature && humidity < autonom[mode_menue_select].humidity) && time3 + 1000 < millis())
        {
            digitalWrite(pinDigital1, LOW);
            analogWrite(pwmPin2, 250);
            Serial.print("TEMP/HUM zu niedrig\n");
            time3 = millis();
        }
        
        //lööömpp time 
        

        //water pump on for specific time and off after configured time 
        //Full speed version (for Slower versions add Resistors and same if dependency just other pin)

        /*
        if (time3 + autonom[mode_menue_select].watering_break <= millis())
        {
            digitalWrite(pinDigital4, HIGH);
            time4 = millis() + autonom[mode_menue_select].watering_break;
        }
        else if (time4 <= millis())
        {
            digitalWrite(pinDigital4, LOW);
        }
        */
        //time3 = millis();
    }
    else if (menue_toggle[4] != old_flag[4])
    {
        old_flag[4] = menue_toggle[4];
    }




    //Checks for system time and if pressed with the right pressure 
    if (time + break_time <= millis() && p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {
        p.x = map(p.x, TS_MINX, TS_MAXX, my_lcd.Get_Display_Width(),0);
        p.y = map(p.y, TS_MINY, TS_MAXY, my_lcd.Get_Display_Height(),0);

        //draw diffrent modes menue
        if (is_pressed(menue_Xoffset, 5, 270, 30, p.x, p.y))
        {
            for (int a = 0; a < mode_menue_options; a++)
            {
                my_lcd.Set_Draw_color(GREEN);                                                                                   //Draw colour GREEN
                my_lcd.Fill_Rectangle(5, 20 * (a + 1) + 15, menue_Xoffset + 110, 20 * (a + 1) + 35);                            //Fills Menue Background GREEN
                show_string(mode_menue[a], menue_Xoffset, 20 * (a + 1) + 20, 2, BLACK, BLACK, true);                            //Writes mode menue options
            }
            mode_menue_flag = true;
            time = millis();
        }

        if(mode_menue_flag)
        {
            for (int a = 0; a < mode_menue_options; a++)
            {
                if(is_pressed(5, 20 * (a + 1) + 15, menue_Xoffset + 110, 20 * (a + 1) + 35, p.x, p.y))
                {
                    //activates the selected automatic mode 
                    mode_menue_toggle[a] = true;

                    //draw over old Option & writes new mode on TOP
                    my_lcd.Set_Draw_color(BACKGROUND);  
                    my_lcd.Fill_Rectangle(150, 5, 350, 40); 
                    show_string(mode_menue[a], 150, 5, 4, GREEN, GREEN, true);
                    //Saves the selected Menue for automatic Mode
                    mode_menue_select = a;          

                    //draw over old menue options
                    my_lcd.Set_Draw_color(BACKGROUND);  
                    my_lcd.Fill_Rectangle(5, 35, menue_Xoffset + 110, 20 * mode_menue_options + 35);

                    //draw normal menue again
                        for (int b = 0; b < menue_options; b++)
                        {
                            show_string(menue_names[b], menue_Xoffset, 30 * (b + 1) + 40, 3, BLACK, BLACK, true);
                            my_lcd.Set_Draw_color(BLACK);
                            my_lcd.Draw_Fast_HLine( 0, 30 * (b + 1) + 65, my_lcd.Get_Display_Width());
                        }
                    mode_menue_flag = false;
                }
            }
            time = millis();
        }

        //Normal options Menue
        if (!mode_menue_flag)
        {
            for (int a = 0; a < menue_options; a++)
            {
                if(is_pressed(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 269, 30 * (a + 1) + 64, p.x,p.y))
                {
                    if(menue_toggle[a])
                        {   
                            show_picture(switch_off_2, sizeof(switch_off_2)/2, menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 269, 30 * (a + 1) + 64);
        
                            menue_toggle[a] = false;
                        }
                        else
                        {  
                            show_picture(switch_on_2, sizeof(switch_on_2)/2, menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 269, 30 * (a + 1) + 64);
                            
                            menue_toggle[a] = true;
                        }
                    time = millis();
                }
            }
        }
    }
}