//Projekt der Schule für Lüftungs Modell

#include "TouchScreen.h"                                    //touch library
#include "LCDWIKI_GUI.h"                                    //Core graphics library
#include "LCDWIKI_KBV.h"                                    //Hardware-specific library
#include "SimpleDHT.h"                                      //Temp / humidity sensor
#include "switch_font.c"                                    //Writing font and imported Pictures
LCDWIKI_KBV my_lcd(ILI9486,A3,A2,A1,A0,A4);                 // Length and width for Display

//Defines the diffrent colours
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define BACKGROUND MAGENTA
#define MENUE_COLOUR CYAN

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define YP A3                                               // must be an analog pin, use "An" notation!
#define XM A2                                               // must be an analog pin, use "An" notation!
#define YM 9                                                //9 can be a digital pin
#define XP 8                                                //8 can be a digital pin

#define TS_MINX 906
#define TS_MAXX 116

#define TS_MINY 92
#define TS_MAXY 952

// We have a status line for like, is FONA working
#define STATUS_X 10
#define STATUS_Y 65
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//PWM Motor pins
#define pwmPin1 44                                          //pwm Pin 1
#define pwmPin2 45                                          //Motor Pin 2
#define pwmPin3 46                                          //Motor Pin 3

//Digital pins
#define pinDigital1 22                                      //Pin 1  
#define pinDigital2 23                                      //Pin 2  
#define pinDigital3 24                                      //Pin 3  
#define pinDigital4 25                                      //Pin 4  
#define pinDigital5 26                                      //Pin 5
#define pinDigital6 27                                      //Pin 6
#define pinDigital7 28                                      //Pin 7
#define pinDigital8 29                                      //Pin 8
#define pinDigital9 30                                      //Pin 9

//Menue
#define menue_options       6                               //Normal Menue
#define info_menue_options  3                               //Info   Menue (Temp, Hum & Light)
#define mode_menue_options  5                               //Mode   Menue (Plant type)
#define menue_Xoffset       15                              //Offset for Menue to the right 
#define break_time          350                             //Break time between each user input
bool menue_toggle[menue_options];                           //Normal Menue flag
bool mode_menue_toggle[mode_menue_options];                 //Mode   Menue flag
bool old_flag[menue_options];                               //Normal Menue old flag
bool mode_menue_flag  = false;                              //Mode   Menue activated
int mode_menue_select = 0;                                  //Mode   Menue selected option
bool watering_switch  = false;               
bool light_switch     = true;
bool automatic_switch = true;   

typedef struct 
{
    float temperature;
    float humidity;
    int light_time;
    int light_break;
    int watering_time;
    int watering_break;
}plant_modes;
plant_modes autonom[mode_menue_options];

const char* menue_names [menue_options]=
{
    "Ventilator", 
    "Dach", 
    "Umluft", 
    "Wasser", 
    "Automatik",
    "Licht"
};
    
const char* info_menue[info_menue_options]= 
{
    "Temperatur:",
    "Feuchte   :",
    "Modus     :"
};

const char* mode_menue[mode_menue_options] =
{
    "Tomate",
    "Zuchini",
    "Ananas",
    "Traube",
    "Baum"
};


//Variables for temp, hum & brightness
#define pinBrightness   49                                  //Data Pin for Day & Night Sensor
#define pinDHT22        51                                  //Data Pin for Temp, Humidity Sensor                                       
SimpleDHT22 dht22(pinDHT22);                                //Struct for Temp, Humidity Sensor
char temp[10];                                              //string for Temp output 
char hum[10];                                               //string for humidity output
float temperature = 0;                                      
float humidity    = 0;
int brightness    = 0;
int err = SimpleDHTErrSuccess;                              //Error feedback for Humidity & Temperature

//system time 
unsigned long time  = 0;                                    //Time for On/off buttons
unsigned long time1 = 0;                                    //Time for Temp, Humidity Sensor
unsigned long time2 = 0;                                    //Time for Mode menue
unsigned long time3 = 0;                                    //Time for Active Automatic Menue
unsigned long time4 = 0;                                    //Time for light
unsigned long time5 = 0;                                    //Time for light break
unsigned long time6 = 0;                                    //Time for watering
unsigned long time7 = 0;                                    //Time for watering break
unsigned long time8 = 0;                                    //Time for turning other options off


//Show String
void show_string(uint8_t* const str,int16_t x,int16_t y,uint8_t csize,uint16_t fc, uint16_t bc,bool mode)
{
    my_lcd.Set_Text_Mode(mode);
    my_lcd.Set_Text_Size(csize);
    my_lcd.Set_Text_colour(fc);
    my_lcd.Set_Text_Back_colour(bc);
    my_lcd.Print_String(str,x,y);
}

//Show Picture
void show_picture(const uint8_t *color_buf,int16_t buf_size,int16_t x1,int16_t y1,int16_t x2,int16_t y2)
{
    my_lcd.Set_Addr_Window(x1, y1, x2, y2); 
    my_lcd.Push_Any_Color(color_buf, buf_size, 1, 1);
}

//Check if Display pressed
bool is_pressed(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t px,int16_t py)
{
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
    //temp, hum, light_time, light_break, watering_time, watering_break
    autonom[0] = {25.0, 80.0, 5000,  10000, 10000,  30000};
    autonom[1] = {25.0, 80.0, 1000,  3000,  10000,  5000};
    autonom[2] = {22.5, 90.0, 10000, 5000,  20000,  8000};
    autonom[3] = {28.0, 75.0, 30000, 5000,  5000,   30000};
    autonom[4] = {25.0, 80.0, 30000, 5000,  5000,   30000};

    //Initialize Screen
    Serial.begin(9600);                                     
    my_lcd.Init_LCD();                                     
    Serial.println(my_lcd.Read_ID(), HEX);
    my_lcd.Fill_Screen(BACKGROUND);                   

    //Pin modes
    pinMode(pinDigital1,   OUTPUT);                         //Output for Relai 1 Ventilator 
    pinMode(pinDigital2,   OUTPUT);                         //Output for Relai 2 WindSimulation
    pinMode(pinDigital3,   OUTPUT);                         //Output for Relai 3 WaterPump
    pinMode(pinDigital4,   OUTPUT);                         //Output for Relai 4 Light 1
    pinMode(pinDigital5,   OUTPUT);                         //Output for Relai 5 Light 2
    pinMode(pinDigital6,   OUTPUT);                         //Output for Relai 6 Light 3                               
    pinMode(pinDigital7,   OUTPUT);                         //Output for Relai 7
    pinMode(pinBrightness, INPUT);                          //Input for Brightness

    //Show title
    show_string("Mode>", menue_Xoffset, 5, 4, BLACK, BLACK, true);

    //Draw Settings-Menue with lines inbetween
    for (int a = 0; a < menue_options; a++)
    {
        //Text for Menue
        show_string(menue_names[a], menue_Xoffset, 30 * (a + 1) + 40, 3, BLACK, BLACK, true);
        my_lcd.Set_Draw_color(BLACK);
        my_lcd.Draw_Fast_HLine( 0, 30 * (a + 1) + 65, my_lcd.Get_Display_Width());

        //Off Button
        my_lcd.Set_Draw_color(RED);
        my_lcd.Fill_Round_Rectangle(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, 10);

        //Line for Button
        my_lcd.Set_Draw_color(BLACK);
        my_lcd.Draw_Round_Rectangle(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, 10);

        //All options on Off
        menue_toggle[a] = false;
        old_flag[a] = false;
    }

    //Draw Info-Menue (temp, humidity, brightness)
    for (int a = 0; a < info_menue_options; a++)
    {
        show_string(info_menue[a], menue_Xoffset, 30 * (a + 1) + (menue_options * 30) + 60, 3, BLACK, BLACK, true);
    }

    //Mode Menue Options Off
    for (int a = 0; a < mode_menue_options; a++)
    {
        mode_menue_toggle[a] = false;
    }
}


void temp_hum()
{
    if (time1 + 2500 <= millis())
    {
        //draw over old input a rectangle
        my_lcd.Set_Draw_color(BACKGROUND);
        my_lcd.Fill_Rectangle(menue_Xoffset + 197, 30 * 1 + (menue_options * 30) + 60, menue_Xoffset + 300, 30 * 4 + (menue_options * 30) + 85); 

        //Error Handler
        if ((err = dht22.read2 (&temperature, &humidity, NULL))  != SimpleDHTErrSuccess) 
        {
            //Shows Info Pin
            show_string("err", menue_Xoffset + 197, 30 * 1 + (menue_options * 30) + 60, 3, BLACK, BACKGROUND, true);
            show_string("pin51", menue_Xoffset + 197, 30 * 2 + (menue_options * 30) + 60, 3, BLACK, BACKGROUND, true);
            show_string("pin49", menue_Xoffset + 197, 30 * 3 + (menue_options * 30) + 60, 3, BLACK, BACKGROUND, true);
            delay(2000);
        }
        else
        {                                  
           //Write Float to String & output
            dtostrf(humidity, 3, 1, hum);
            strcat(hum," %");
            dtostrf(temperature, 3, 1, temp);
            strcat(temp," C");
            show_string(temp, menue_Xoffset + 197, 30 * 1 + (menue_options * 30) + 60, 3, BLACK, BACKGROUND, true);
            show_string(hum,  menue_Xoffset + 197, 30 * 2 + (menue_options * 30) + 60, 3, BLACK, BACKGROUND, true); 

            //Day & Night
            brightness = digitalRead(pinBrightness);
            if (brightness == 0)
            {
                show_string("Tag", menue_Xoffset + 197, 30 * 3 + (menue_options * 30) + 60, 3, BLACK, BACKGROUND, true);
            }
            else
            {
                show_string("Nacht", menue_Xoffset + 197, 30 * 3 + (menue_options * 30) + 60, 3, BLACK, BACKGROUND, true); 
            }
         }
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
    
    //Info
    temp_hum();
    
    //Menue Option 1
    if (menue_toggle[0] != old_flag[0] && !menue_toggle[4])
    {
        if (menue_toggle[0])
        {   
            digitalWrite(pinDigital1, HIGH);
            old_flag[0] = menue_toggle[0];
            Serial.print("Menue Option 1 On\n");
        }
        else
        {
            digitalWrite(pinDigital1, LOW);
            old_flag[0] = menue_toggle[0];
            Serial.print("Menue Option 1 Off\n");
        }
    }

    //Menue Option 2
    if (menue_toggle[1] != old_flag[1] && !menue_toggle[4])
    {
        if (menue_toggle[1])
        {   
            //rechts
            analogWrite(pwmPin2, 230);

            //links
            analogWrite(pwmPin3, 125); 
            old_flag[1] = menue_toggle[1];
            Serial.print("Menue Option 2 On\n");
        }
        else
        {
            analogWrite(pwmPin2, 125);

            analogWrite(pwmPin3, 230);
            old_flag[1] = menue_toggle[1];
            Serial.print("Menue Option 2 Off\n");
        }
    }

    //Menue Option 3
    if (menue_toggle[2] != old_flag[2] && !menue_toggle[4])
    {
        if (menue_toggle[2])
        {   
            digitalWrite(pinDigital2, HIGH);
            old_flag[2] = menue_toggle[2];
            Serial.print("Menue Option 3 On\n");
        }
        else
        {
            digitalWrite(pinDigital2, LOW);
            old_flag[2] = menue_toggle[2];
            Serial.print("Menue Option 3 Off\n");
        }
    }

    //Menue Option 4
    if (menue_toggle[3] != old_flag[3] && !menue_toggle[4])
    {
        if (menue_toggle[3])
        {   
            digitalWrite(pinDigital3, HIGH);
            old_flag[3] = menue_toggle[3];
            Serial.print("Menue Option 4 On\n");
        }
        else
        {
            digitalWrite(pinDigital3, LOW);
            old_flag[3] = menue_toggle[3];
            Serial.print("Menue Option 4 Off\n");
        }
    }

    //Menue Option 5
    if(menue_toggle[4] != old_flag[4])
    {
        //Overwrite Activated Options
        if(automatic_switch)
        {
            for (int a = 0; a < menue_options; a++)
            {
                if(a != 4)
                {
                    menue_toggle[a] = false;
                    my_lcd.Set_Draw_color(RED);
                    my_lcd.Fill_Round_Rectangle(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, 10);
                    my_lcd.Set_Draw_color(BLACK);
                    my_lcd.Draw_Round_Rectangle(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, 10); 
                }
            }
            automatic_switch = false;
        }

        //Temp & Hum check
        if ((temperature > autonom[mode_menue_select].temperature || humidity > autonom[mode_menue_select].humidity) && time3 + 1000 < millis())
        {
            //Vent
            digitalWrite(pinDigital1, HIGH);

            //If it's day
            if(!brightness)
            {
                //Servo motors left & right open
                analogWrite(pwmPin2, 50 );                
                analogWrite(pwmPin3, 250);
            }
            //Slow air fan
            digitalWrite(pinDigital2, LOW);
            time3 = millis();
            Serial.print("TEMP/HUM zu hoch\n");
        } 
        else if ((temperature < autonom[mode_menue_select].temperature || humidity < autonom[mode_menue_select].humidity) && time3 + 1000 < millis())
        {
            //Vent 
            digitalWrite(pinDigital1, LOW);

            //Servo motors left & right
            analogWrite(pwmPin2, 250);             
            analogWrite(pwmPin3, 50 );                  

            //Slow air fan 
            digitalWrite(pinDigital2, HIGH);            
            time3 = millis();
            Serial.print("TEMP/HUM zu niedrig\n");
        }
        
        //Cycle for the light
        if(millis() > time4)
        {
            time4 = autonom[mode_menue_select].light_time + millis();

            if(light_switch)
            {
                digitalWrite(pinDigital4, HIGH);
                digitalWrite(pinDigital5, HIGH);
                digitalWrite(pinDigital6, HIGH);
                light_switch = false;
            }
            else if (!light_switch)
            {
                time4 = autonom[mode_menue_select].light_break + millis();
                digitalWrite(pinDigital4, LOW);
                digitalWrite(pinDigital5, LOW);
                digitalWrite(pinDigital6, LOW);
                light_switch = true;
            }
        }

        //Cycle for the waterpump
        if(millis() > time5)
        {
            time5 = autonom[mode_menue_select].watering_time + millis();

            if(watering_switch)
            {
                digitalWrite(pinDigital3, HIGH);
                watering_switch = false;
            }
            else if (!watering_switch)
            {
                time5 = autonom[mode_menue_select].watering_break + millis();
                digitalWrite(pinDigital3, LOW);
                watering_switch = true;
            }
        }

        if (millis() > time8)
        {
            time8 = millis() + 1000;
            automatic_switch = true;
        }
    }
    else if (menue_toggle[4] != old_flag[4])
    {
        old_flag[4] = menue_toggle[4];
    } 

    //Menue Option 6
    if (menue_toggle[5] != old_flag[5] && !menue_toggle[4])
    {
        if (menue_toggle[5])
        {   
            digitalWrite(pinDigital4, HIGH);
            digitalWrite(pinDigital5, HIGH);
            digitalWrite(pinDigital6, HIGH);
            old_flag[5] = menue_toggle[5];
            Serial.print("Menue Option 6 On\n");
        }
        else
        {
            digitalWrite(pinDigital4, LOW);
            digitalWrite(pinDigital5, LOW);
            digitalWrite(pinDigital6, LOW);
            old_flag[5] = menue_toggle[5];
            Serial.print("Menue Option 6 Off\n");
        }
    }

    //Screen Touch Area
    if (time + break_time <= millis() && p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {
        p.x = map(p.x, TS_MINX, TS_MAXX, my_lcd.Get_Display_Width(),0);
        p.y = map(p.y, TS_MINY, TS_MAXY, my_lcd.Get_Display_Height(),0);

        //draw diffrent modes menue
        if (is_pressed(menue_Xoffset, 5, 270, 30, p.x, p.y))
        {
            for (int a = 0; a < mode_menue_options; a++)
            {
                my_lcd.Set_Draw_color(MENUE_COLOUR);    
                my_lcd.Fill_Round_Rectangle(5, 20 * (a + 1) + 15, menue_Xoffset + 110, 20 * (a + 1) + 35, 12);
                my_lcd.Set_Draw_color(BLACK);
                my_lcd.Draw_Round_Rectangle(5, 20 * (a + 1) + 15, menue_Xoffset + 110, 20 * (a + 1) + 35, 12);
                show_string(mode_menue[a], menue_Xoffset, 20 * (a + 1) + 20, 2, BLACK, BLACK, true);
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

                    //Overwrite old selected Mode
                    my_lcd.Set_Draw_color(BACKGROUND);  
                    my_lcd.Fill_Rectangle(150, 5, 350, 40); 
                    show_string(mode_menue[a], 150, 5, 4, MENUE_COLOUR, MENUE_COLOUR, true);
                    
                    //Saves the selected Menue for automatic Mode
                    mode_menue_select = a;          

                    //Overwrite Mode Menue
                    my_lcd.Set_Draw_color(BACKGROUND);  
                    my_lcd.Fill_Rectangle(5, 35, menue_Xoffset + 110, 20 * mode_menue_options + 35);

                    //Draw normal Menue
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
                if(is_pressed(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, p.x,p.y))
                {
                    if(menue_toggle[a])
                        {   
                            my_lcd.Set_Draw_color(RED);
                            my_lcd.Fill_Round_Rectangle(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, 10);
                            my_lcd.Set_Draw_color(BLACK);
                            my_lcd.Draw_Round_Rectangle(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, 10);                            
                            menue_toggle[a] = false;
                        }
                        else
                        {  
                            my_lcd.Set_Draw_color(GREEN);
                            my_lcd.Fill_Round_Rectangle(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, 10);
                            my_lcd.Set_Draw_color(BLACK);
                            my_lcd.Draw_Round_Rectangle(menue_Xoffset + 240, 30 * (a + 1) + 35, menue_Xoffset + 290, 30 * (a + 1) + 64, 10);                               
                            menue_toggle[a] = true;
                        }
                    time = millis();
                }
            }
        }
    }
}