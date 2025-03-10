bool prevGameState = HIGH;     
bool prevPauseState = HIGH;    
bool prevButtonState = HIGH;   
bool playingAllowed = true;    
bool gameOver = false;         
int distanceTraveled = 0;      
int pauseNumber = 0;           
unsigned long lastLightChange = 0; 
bool isGreenLight = true;          
int lightDuration = 0;             
int valueToWin = 0;

void setup() {
    pinMode(2, INPUT_PULLUP); // on/off switch
    pinMode(3, INPUT_PULLUP); // pause/unpause button
    pinMode(4, INPUT_PULLUP); // walk/run button
    pinMode(5, OUTPUT);       // green led
    pinMode(6, OUTPUT);       // yellow led
    pinMode(7, OUTPUT);       // red led
    pinMode(9, OUTPUT);       // speaker
    Serial.begin(9600);       // initialize serial communication
    changeLights(1);		  // start with all lights off

    randomSeed(analogRead(0)); // seed for random light duration
    valueToWin = difficulty(); // Initialize difficulty level
}

void loop() {
    bool currentGameState = digitalRead(2);		// on/off switch
    bool currentPauseState = digitalRead(3);	// pause/unpause switch
    bool currentButtonState = digitalRead(4);	// walk/run button

    // on/off switch settings
    if (currentGameState != prevGameState) {
	    prevGameState = currentGameState;
        if (currentGameState == LOW) { 	// game turned on
            playingAllowed = true;
            gameOver = false;
            distanceTraveled = 0;
            pauseNumber = 0;
            isGreenLight = true;
            lightDuration = random(1000, 7000);
            lastLightChange = millis();
            digitalWrite(5, HIGH);
            digitalWrite(6, LOW);
            digitalWrite(7, LOW);
            Serial.println("Game Reset!");
            valueToWin = difficulty(); 	// Update difficulty when game resets
        } else {						// game turned off
            playingAllowed = false;
            Serial.println("Game Off");
            changeLights(1);
        }
    playSoundAnimation(currentGameState == LOW ? 0 : 1);
  	}

  // pause/unpause switch settings
  	if (currentGameState == LOW && currentPauseState == LOW && prevPauseState == HIGH) {
    	if (!gameOver) { // pause/unpause button only works if game is on and not over
      		playingAllowed = !playingAllowed; // toggle paused/unpaused
      		Serial.println(playingAllowed ? "Unpaused" : "Paused");
            playSoundAnimation(playingAllowed ? 0 : 2);
            digitalWrite(6, playingAllowed ? LOW : HIGH); // yellow light for paused state
            if (!playingAllowed) {
                digitalWrite(5, LOW); // turn off green LED
                digitalWrite(7, LOW); // turn off red LED
      		}
    	}
  	}
  
  	prevPauseState = currentPauseState;

  // toggles green and red lights
    if (playingAllowed) {
        if (millis() - lastLightChange > lightDuration) {
            isGreenLight = !isGreenLight;
            lastLightChange = millis();
            lightDuration = random(1000, 7000);
            digitalWrite(5, isGreenLight ? HIGH : LOW);
            digitalWrite(7, isGreenLight ? LOW : HIGH);
        }

        if (!isGreenLight && currentButtonState == LOW) { //user hits button during red = loses game
            Serial.println("Game Over!");
          	playSoundAnimation(4);
            playingAllowed = false;
            gameOver = true;
            changeLights(1);
        }
    }

  // walk/run button handling (only if green light is on, otherwise ^)
    if (playingAllowed && isGreenLight && currentButtonState == LOW && prevButtonState == HIGH) {
        prevButtonState = currentButtonState;
        Serial.println("Button Pressed!");
        distanceTraveled++;
        Serial.println(distanceTraveled);
        if (distanceTraveled >= valueToWin) { // valueToWin determined in difficulty()
            Serial.println("YOU WIN!");
          	playSoundAnimation(3);
            for (int i = 1; i <= 10; i++) {
                changeLights(0);
                if (digitalRead(2) == HIGH) {
                  	break;
                }
            }
            playingAllowed = false;
        }
    }

  	if (currentButtonState == HIGH) { // reset button state
    	prevButtonState = HIGH;
  	}
}

void changeLights(int lightsType) { // lights when game is won
  	switch (lightsType) {
  		case 0:	// lights when winning
			digitalWrite(5, HIGH); delay(250);
            digitalWrite(5, LOW);
            digitalWrite(6, HIGH); delay(250);
            digitalWrite(6, LOW);
            digitalWrite(7, HIGH); delay(250);
            digitalWrite(7, LOW);
      	case 1: // all lights off
      		digitalWrite(5, LOW); 
            digitalWrite(6, LOW);
            digitalWrite(7, LOW);
    }
}

int difficulty() { // difficulty mode (changes distance needed to win)
    int potValue = analogRead(A0);

    if (potValue < 341) {
        Serial.println("Hard Mode Selected");
        return 250;
    } else if (potValue < 682) {
        Serial.println("Medium Mode Selected");
        return 75;
    } else {
        Serial.println("Easy Mode Selected");
        return 30;
    }
}

void playSoundAnimation(int soundAnimationType) { //sound and animation types
    switch (soundAnimationType) {
    	case 0: // on or unpaused
      		tone(9, 330, 300); delay(650);
            tone(9, 330, 400); delay(650);
            tone(9, 330, 400); delay(650);
            tone(9, 659, 350); delay(350);
            break;
    	case 1: // off
            tone(9, 500, 300); delay(400);
            tone(9, 250, 450); delay(450);
            break;
      	case 2: // paused
            tone(9, 200, 200); delay(200);
            break;
    	case 3: // win
            tone(9, 500 , 200); delay(200);
            tone(9, 700 , 200); delay(200);
            tone(9, 900 , 200); delay(200);
            tone(9, 1200, 200); delay(200);
            break;
    	case 4: // lose
            tone(9, 400, 300); delay(400);
            tone(9, 300, 300); delay(400);
            tone(9, 200, 300); delay(400);
            break;
  }
}
