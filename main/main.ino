#include <LiquidCrystal.h>

/*
 * Quit horsing around
 * A simple timer for controlling feed time for horses/other animals.
 */

// Display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Inputs
uint8_t HOURS_BUTTON = 6;
uint8_t MINUTES_BUTTON = 7;

// Outputs
uint8_t LED_PIN = 8;
uint8_t SOLENOID_PIN = 9;

// Helper constants
unsigned const long HOUR_MS = 60l * 60l * 1000l;
unsigned const long MINUTE_MS = 60l * 1000l;

// Configurable constants
unsigned const long debounceMillis = 400l; // Time until next input is accepted
unsigned const long renderDelayMs = 1000l; // Time between each render
unsigned const long timerDelayMs = 3000l; // Time until timer starts counting down
unsigned const long timerConfigurationDurationMs = 10000l; // Time until the timer duration resets to zero
unsigned const long solenoidImpulseDurationMs = 100l; // Duration of solenoid activation impulse

// Variables
unsigned long lastInput = 0; // Used for debounce and timer start
unsigned long lastRender = 0; // Used for render delay. This is to avoid flickering

// Variables for timer
unsigned long timerEnd = 0; // The expected end of the timer
unsigned long timerDurationMs = 0; // The duration of the timer
unsigned long solenoidActivationTime = 0; // Time when solenoid was activated

void setup()
{
    lcd.begin(16, 2);
    pinMode(HOURS_BUTTON, INPUT);
    pinMode(MINUTES_BUTTON, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(SOLENOID_PIN, OUTPUT);
    digitalWrite(SOLENOID_PIN, LOW);

    printTime(0);
}

void loop()
{
    bool acceptsInput = millis() - lastInput > debounceMillis;

    if (acceptsInput)
    {
        bool hoursButtonPressed = digitalRead(HOURS_BUTTON) == HIGH;
        if (hoursButtonPressed)
        {
            // timerDurationMs += HOUR_MS;
            timerDurationMs += MINUTE_MS;
        }
        
        bool minutesButtonPressed = digitalRead(MINUTES_BUTTON) == HIGH;
        if (minutesButtonPressed)
        {
            // timerDurationMs += MINUTE_MS;
            timerDurationMs += 1000l;
        }

        if (minutesButtonPressed || hoursButtonPressed)
        {
            lastInput = millis();
            timerEnd = millis() + timerDurationMs;
            printTime(timerDurationMs);
        }
    }

    bool timerConfigurationDurationExceeded = millis() - lastInput > timerConfigurationDurationMs;

    if (timerConfigurationDurationExceeded) 
    {
      timerDurationMs = 0;
    }

    bool timerStarted = millis() - lastInput > timerDelayMs;

    if (timerStarted)
    {
        if (millis() - lastRender > renderDelayMs && timerEnd > millis())
        {
            lastRender = millis();
            printTime(timerEnd - millis());
        }

        if (timerEnd > 0 && millis() > timerEnd)
        {
            digitalWrite(LED_PIN, HIGH);
            
            // Activate solenoid with a brief impulse (non-blocking)
            if (solenoidActivationTime == 0)
            {
                digitalWrite(SOLENOID_PIN, HIGH);
                solenoidActivationTime = millis();
            }
            
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Time's up!");
            lcd.setCursor(0, 1);
            lcd.print("Press to reset");
            while (true)
            {
                // Check if solenoid impulse duration has elapsed
                if (solenoidActivationTime > 0 && millis() - solenoidActivationTime >= solenoidImpulseDurationMs)
                {
                    digitalWrite(SOLENOID_PIN, LOW);
                    solenoidActivationTime = 0;
                }
                
                bool hoursButtonPressed = digitalRead(HOURS_BUTTON) == HIGH;
                bool minutesButtonPressed = digitalRead(MINUTES_BUTTON) == HIGH;
                if (hoursButtonPressed || minutesButtonPressed)
                {
                    timerEnd = 0;
                    timerDurationMs = 0;
                    digitalWrite(LED_PIN, LOW);
                    digitalWrite(SOLENOID_PIN, LOW);
                    solenoidActivationTime = 0;
                    break;
                }
            }
        }
    }
}

void printTime(unsigned long timeMs)
{
    int timeSeconds = (timeMs / 1000);
    int timeMinutes = (timeSeconds / 60);
    int timeHours = (timeMinutes / 60);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time: ");

    lcd.setCursor(0, 1);

    lcd.print(timeHours % 24);
    lcd.print("h ");

    lcd.print(timeMinutes % 60);
    lcd.print("m ");

    lcd.print(timeSeconds % 60);
    lcd.print("s ");
}
