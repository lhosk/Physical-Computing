#include <LiquidCrystal.h> // Library for the LCD
#include <math.h>          // For sqrt() and pow()

// Initialize an LCD object. Pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// Function prototype for MCS
int MCS(const double (*moleculeArray)[3], int numAtoms, double radius, int trials);

// Atomic packing factors (APF)
const double redAPF = 0.523599;       // Simple cubic structure (SC)
const double yellowAPF = 0.740480;    // Face-centered cubic structure (FCC)
const double greenAPF = 0.680175;     // Body-centered cubic structure (BCC)

// Radii for each structure (scaled to lattice constant 'a')
const double radius_SC = 0.5;         // SC radius (a/2)
const double radius_FCC = 0.353553;   // FCC radius (sqrt(2)/4 * a)
const double radius_BCC = 0.433013;   // BCC radius (sqrt(3)/4 * a)

// Buttons, lights, switch, potentiometer
const int redButton = 2;    const int redLight = 3;
const int yellowButton = 4; const int yellowLight = 5;
const int greenButton = 6;  const int greenLight = 7;
const int dartButton = A0;
const int onOffSwitch = A2;
const int potentiometer = A1;
bool systemOn = true;  
bool lastShootState = HIGH;

// Monte Carlo simulation variables
int dartsHit = 0;
int dartsThrown = 0;
double calculatedAPF = 0.0;
bool dartsAllowed = false;

// Track which structure is selected
enum Structure { STRUCTURE_NONE, STRUCTURE_SC, STRUCTURE_FCC, STRUCTURE_BCC };
Structure selectedStructure = STRUCTURE_NONE;

// Molecule position arrays
const double Molecule_SC[8][3] = {
    {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0},
    {1.0, 1.0, 0.0}, {1.0, 0.0, 1.0}, {0.0, 1.0, 1.0}, {1.0, 1.0, 1.0}
};

const double Molecule_FCC[14][3] = {
    {0.0, 0.0, 0.0}, {0.5, 0.5, 0.0}, {0.5, 0.0, 0.5}, {0.0, 0.5, 0.5},
    {0.0, 0.0, 1.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0},
    {1.0, 0.0, 1.0}, {0.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {0.5, 0.5, 1.0},
    {1.0, 0.5, 0.5}, {0.5, 1.0, 0.5}
};

const double Molecule_BCC[9][3] = {
    {0.0, 0.0, 0.0}, {0.5, 0.5, 0.5}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}, {1.0, 1.0, 0.0}, {1.0, 0.0, 1.0}, {0.0, 1.0, 1.0},
    {1.0, 1.0, 1.0}
};

const double (*moleculePositions)[3] = nullptr;
double currentRadius = 0.0;

void setup() {
    pinMode(redButton, INPUT_PULLUP);
    pinMode(yellowButton, INPUT_PULLUP);
    pinMode(greenButton, INPUT_PULLUP);
    pinMode(dartButton, INPUT_PULLUP);
    pinMode(onOffSwitch, INPUT_PULLUP);
    pinMode(potentiometer, INPUT);
    pinMode(redLight, OUTPUT);
    pinMode(yellowLight, OUTPUT);
    pinMode(greenLight, OUTPUT);

    Serial.begin(9600);  // Initialize Serial Monitor
    lcd.begin(16, 2);    // Initialize LCD with 16 columns and 2 rows
    lcd.print("System Ready"); // Display initial message
    delay(2000); // Wait for the user to read the message
    lcd.clear();
}

void loop() {
    int redState = digitalRead(redButton);
    int yellowState = digitalRead(yellowButton);
    int greenState = digitalRead(greenButton);
    int shootState = digitalRead(dartButton);
    int onOffState = analogRead(onOffSwitch);
    int numDarts = analogRead(potentiometer) / 10 + 1;

    // Toggle system state
    if (onOffState < 512) {
        systemOn = true;
    } else {
        systemOn = false;
        dartsHit = 0;
        dartsThrown = 0;
        selectedStructure = STRUCTURE_NONE;
    }

    if (systemOn) {
        if (redState == LOW && selectedStructure != STRUCTURE_SC) {
            selectedStructure = STRUCTURE_SC;
            moleculePositions = Molecule_SC;
            currentRadius = radius_SC;
            reset();

            digitalWrite(redLight, HIGH);
            digitalWrite(yellowLight, LOW);
            digitalWrite(greenLight, LOW);
        } else if (yellowState == LOW && selectedStructure != STRUCTURE_FCC) {
            selectedStructure = STRUCTURE_FCC;
            moleculePositions = Molecule_FCC;
            currentRadius = radius_FCC;
            reset();

            digitalWrite(redLight, LOW);
            digitalWrite(yellowLight, HIGH);
            digitalWrite(greenLight, LOW);
        } else if (greenState == LOW && selectedStructure != STRUCTURE_BCC) {
            selectedStructure = STRUCTURE_BCC;
            moleculePositions = Molecule_BCC;
            currentRadius = radius_BCC;
            reset();

            digitalWrite(redLight, LOW);
            digitalWrite(yellowLight, LOW);
            digitalWrite(greenLight, HIGH);
        }

        if (lastShootState == HIGH && shootState == LOW && dartsAllowed) {
            dartsHit += MCS(moleculePositions,
                            (selectedStructure == STRUCTURE_SC) ? 8 : 
                            (selectedStructure == STRUCTURE_FCC) ? 14 : 9,
                            currentRadius,
                            numDarts);
            dartsThrown += numDarts;

            calculatedAPF = (double)dartsHit / dartsThrown;

            // Output to Serial Monitor
            Serial.print("Darts Thrown: ");
            Serial.println(dartsThrown);
            Serial.print("Darts Hit: ");
            Serial.println(dartsHit);
            Serial.print("Calculated APF: ");
            Serial.println(calculatedAPF);

            // Output to LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Thrown: ");
            lcd.print(dartsThrown);
            lcd.setCursor(0, 1);
            lcd.print("Hit: ");
            lcd.print(dartsHit);
            lcd.setCursor(9, 1);
            lcd.print("APF: ");
            lcd.print(calculatedAPF, 2);

            delay(350);
        }
        lastShootState = shootState;
    } else {
        digitalWrite(redLight, LOW);
        digitalWrite(yellowLight, LOW);
        digitalWrite(greenLight, LOW);
        lcd.clear();
    }
}

int MCS(const double (*moleculeArray)[3], int numAtoms, double radius, int trials) {
    int hits = 0;

    for (int t = 0; t < trials; t++) {
        double x = random(0, 1000) / 1000.0;
        double y = random(0, 1000) / 1000.0;
        double z = random(0, 1000) / 1000.0;

        for (int i = 0; i < numAtoms; i++) {
            double dx = x - moleculeArray[i][0];
            double dy = y - moleculeArray[i][1];
            double dz = z - moleculeArray[i][2];
            double distanceSquared = dx * dx + dy * dy + dz * dz;

            if (distanceSquared <= radius * radius) {
                hits++;
                break;
            }
        }
    }

    return hits;
}

void reset() {
    dartsThrown = 0;
    dartsHit = 0;
    dartsAllowed = true;
}
