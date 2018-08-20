/******************************************************************/
/*!
    @file     Velleman_VMW100.h
    @author   B. Nuyttens (Velleman nv)
    @license  MIT (see license file)

    This is a library for the Velleman VMW100 Arduino base watch
    -> http://www.velleman.eu/products/view/?id=438852

    v1.0  - Initial release
*/
/******************************************************************/
#include "VMW100.h"
#include "Adafruit_NeoPixel.h"
//#define DEBUG
Velleman_VMW100::Velleman_VMW100() {
    _watchButtonPin = A0;
    pinMode(_watchButtonPin,INPUT);
    buttoncounter = 0;                                 //variable to count LONGPRESSes
    specialhour = true;                                        //variable that remembers if the "special" hour indicator needs to be shown or not
    clockshow = false;                                         //variable that remembers if clockshow is in progress (is needed to break out of show routine to set the time on long press)
    strip = Adafruit_NeoPixel(24, 12, NEO_GRB + NEO_KHZ800);
}

void Velleman_VMW100::begin() {

#ifdef DEBUG
    Serial.begin(57600);
    while (!Serial) {
        delay(20);
    }
#endif

    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    if (! rtc.begin()) {
#ifdef DEBUG
        Serial.println("Couldn't find RTC, check connections and assembly");
#endif
        while (!isButtonPressed()) {
            //anim_vertical();
            playAnimation(3);
        }
    }
    strip.begin();
	/***************************
	*
	*
	* BRIGHTNESS MAXIMUM 128
	*
	*
	****************************/
    
    checkReset();
}

Adafruit_NeoPixel Velleman_VMW100::getStrip()
{
	return strip;
}

//------------------------------------------------------------------
/*
 * Function:  checkreset
 * --------------------
 *  Checks if the stored year in the RTC chip is either 2017, 2018 or 2019. If this is the case, state will be set to "SHOW_TIME"
 *  This means the year is "sensible" and the RTC should have the correct data. (Battery can last no longer than 2 years...)
 *
 *  If the stored year is any other year we assume the date in the chip is incorrect (like after a batteryswap) and the rtc time will be reset to "2017, 0, 0, 0, 0, 0" (Start year will always be 2017 - "set time" routine also uses 2017)
 *
 *  This routine should be called in setup to detect if an erroneous reset (esd spike) has occured. When the RTC chip still has the correct time but the µ-controller just had a reset the watch will just do a show time instead of asking the user to re-enter the time.
 *
 *  returns: nothing
 */
//------------------------------------------------------------------
void Velleman_VMW100::checkReset() {
    DateTime now = rtc.now();
    if ((now.year() >= 2017) && (now.year() <= 2019))
    {
        Serial.println("Correct Time");
        setState(SHOW_TIME);
    }
    else
    {
        Serial.println("Incorrect Time");
        playAnimation(1);
        //anim_pop_out();
        rtc.adjust(DateTime(2017, 0, 0, 0, 0, 0));
        setState(SET_TIME);
    }
}

void Velleman_VMW100::clearArrays() {
    setAllLeds(0,true,true);
}

//------------------------------------------------------------------
/*
 * Function:  setHand
 * --------------------
 *  Sets the arrays up to create hour or minute hands. Use this function (or multiple of these) before show() to display your desired hands.
 *
 *  led = 0 -> 11
 *  brightness = 0 -> 255
 *  minutehand = true/false
 *  hourhand = true/false
 *
 *  returns: nothing
 */
//------------------------------------------------------------------
void Velleman_VMW100::setHand(int led, int brightness, bool minutehand, bool hourhand) {

    int handbrightness = brightness;
    //led--;
    if (led > 11)
        led -= 12;
    if (minutehand) {
        _setMinuteLeds[led] = handbrightness;
    }

    if (hourhand) {
        _setHourLeds[led] = handbrightness;
    }
}

void Velleman_VMW100::setAllLeds(int brightness,bool minutes,bool hours) 
{	
    //int allbrightness = map(brightness, 0, 255, 0, step);
    int allbrightness = brightness;
    if (minutes) {
        for (int i=0; i < ledcount; i++) {
            _setMinuteLeds[i] = allbrightness;
        }
    }

    if (hours) {
        for (int i=0; i < ledcount; i++) {
            _setHourLeds[i] = allbrightness;
        }
    }
}

void Velleman_VMW100::setState(watchState state) 
{
    _state = state;
}

watchState Velleman_VMW100::getState() 
{
    return _state;
}

void Velleman_VMW100::executeState()
{
    switch (_state) {
    case SHOW_TIME:
        showClock(clocktime);
        break;
    case SET_TIME:
        configureTime();
        break;
    case DO_GAME:
        executeGame();
        break;
    case GO_TO_SLEEP:
        sleep();
        break;
    default:
        _state = SHOW_TIME;
        break;
    }
}

DateTime Velleman_VMW100::getTime() 
{
    return rtc.now();
}

void Velleman_VMW100::setTime(DateTime time) 
{
    rtc.adjust(time);
}

//------------------------------------------------------------------
/*
 * Function:  showArray
 * --------------------
 *  Displays the values in the "set..." arrays for an amount of time (runtime)
 *  Multiplexes all the leds so dimming is possible.
 *
 *  runtime =  how long this function wil run/display
 *
 *  returns: nothing
 */
//------------------------------------------------------------------
void Velleman_VMW100::showArray(int runtime) 
{
    if (!(isButtonPressed()&& (clockshow == true)))
	{
		strip.clear();
        for (int i = 0; i<24;i++)
        {
            if (i < 12)
            {
                strip.setPixelColor(i,0,_setHourLeds[i],_setHourLeds[i]);
            } else {
                strip.setPixelColor(i,0,_setMinuteLeds[i-12],0);
            }

        }
        strip.show();
    }
    for (int a=0; a <= runtime; a++) {

    if (isButtonPressed()&&(clockshow == true)) {
            buttoncounter++;
            if (buttoncounter >= longpress) {
                setState(SET_TIME);
                break;
            }
        }
        delay(1);
    }
    strip.clear();
    strip.show();
    /*
    digitalWrite(_minuteSinkPin, HIGH);
    for (int i=0; i < ledcount; i++){
      if (_setMinuteLeds[i] != 0)
      {
        digitalWrite(_ledpins[i], HIGH);
      }
      delayMicroseconds(_setMinuteLeds[i]);
      digitalWrite(_ledpins[i], LOW);
      delayMicroseconds(step-_setMinuteLeds[i]);
    }
    digitalWrite(_minuteSinkPin, LOW);

    digitalWrite(_hourSinkPin, HIGH);
    for (int i=0; i < ledcount; i++){
      if (_setHourLeds[i] != 0)
      {
        digitalWrite(_ledpins[i], HIGH);
      }
      delayMicroseconds(_setHourLeds[i]);
      digitalWrite(_ledpins[i], LOW);
      delayMicroseconds(step-_setHourLeds[i]);
    }
    digitalWrite(_hourSinkPin, LOW);*/
}

//------------------------------------------------------------------
/*
 * Function:  doclock
 * --------------------
 *  Displays the current time with roll in/out animations. Communicates with rtc to get actual time. Wehn bogus data is received "state" will be set to "SET_TIME"
 *  Exit out of routine by buttonpress, go to set time routine by longpress
 *
 *  showtime = how long actual time wil be displayed between animation
 *
 *  returns: nothing
 */
//------------------------------------------------------------------
void Velleman_VMW100::showClock(int showtime) 
{

    int animationspeed = 50;
    int wait = 100;
    int hours;
    int minutes;

    clockshow = true;
    setState(GO_TO_SLEEP);

    //anim_pop_out();
    playAnimation(1);

    DateTime now = getTime();
    hours = now.hour();
    minutes = now.minute();
	printDate();
    //bringing back from 24h notation
    if (hours > 12) {
        hours = hours-12;
    }

    //scaling down to array size
    hours = hours;
    minutes = (minutes/5);

    //fixing zero

    if (minutes < 0) {
        minutes = 1;
    }
    if (hours < 0) {
        hours = 11;
    }

    if ((hours > 11) || (minutes > 11)) {
        setState(SET_TIME);
        return;
    }

    clearArrays();

    //animating in
    setHand(11,255, false, true);
    showArray(animationspeed);
    clearArrays();
    for (int h=0; h < hours; h++) {
        setHand(h,255, false, true);
        showArray(animationspeed);
        clearArrays();
    }
    setHand(hours,255, false, true);

    //wait between hands
    showArray(wait);

    setHand(11,255, true, true);
    setHand(hours,255, false, true);
    showArray(animationspeed);
    clearArrays();
    for (int m=0; m < minutes; m++) {
        setHand(m,255, true, true);
        setHand(hours,255, false, true);
        showArray(animationspeed);
        clearArrays();
    }

    //setting hours
    setHand(hours,255, false, true);

    if (specialhour == true) {
        int hourplusone = hours+1;
        if (hourplusone == 12) {
            hourplusone = 0;
        }
        if ((minutes > 5) && (minutes < 11)) {
            setHand(hourplusone,10, false, true);
        }
    }

    //setting minutes
    setHand(minutes,255, true, true);

    //displaying time
    for (long i=0; i <= showtime; i++) {
        showArray(0);
        if (isButtonPressed())
        {
            break;
        }
    }
    if (getState() != SET_TIME) {
        if (!isButtonPressed())
        {
            clockshow = false;
        }
        //animating out
        for (int h=hours; h <= ledcount; h++) {
            setHand(h,255, false, true);
            setHand(minutes,255, true, true);
            showArray(animationspeed);
            clearArrays();
        }

        for (int m = minutes; m <= ledcount; m++) {
            setHand(m,255, true, true);
            showArray(animationspeed);
            clearArrays();
        }
        setHand(12,255, true, true);
        showArray(wait);
        clearArrays();
        delay(wait);
        //anim_pop_in();
        playAnimation(2);

        clockshow = false;
        buttoncounter = 0;
	}
        while (isButtonPressed()) {
            //wait...
        }
        //Serial.println("end showclock");
}

void Velleman_VMW100::notShowingTime()
{
    clockshow = false;
}

//------------------------------------------------------------------
/*
 * Function:  setclock
 * --------------------
 *  Sets the time
 *  1. set the hours (longpress to confirm)
 *  2. set the minutes (longpress to confirm)
 *   - Watch will start counting time from this point.
 *  3. set the "special hour " indicator (see user-manual to learn what this indicator does)
 *
 *  returns: nothing
 */
//------------------------------------------------------------------
void Velleman_VMW100::configureTime()
{
    int newhours = 11;
    int newminutes = 11;

    DateTime now = getTime();
    printDate();
    newhours = now.hour();
    newminutes = now.minute();

    //bringing back from 24h notation
    if (newhours > 12) {
        newhours = newhours-12;
    }

    //scaling down to array size
    //newhours = newhours-1;
    newminutes = newminutes/5;

    //fixing zero
    if (newminutes < 0) {
        newminutes = 0;
    }
    if (newhours < 0) {
        newhours = 0;
    }

    if ((newhours > 11) || (newminutes > 11)) {
        setState(SET_TIME);
        return;
    }

    clearArrays();
    //setting hours
    buttoncounter = 0;
    while (1) {
        while (isButtonPressed()) {
            buttoncounter++;
            setHand(newhours,255, false, true);
            setHand(newminutes,255, true, true);
            if (buttoncounter < longpress)
            {
                showArray(0);
            }
        }
        clearArrays();
        if ((buttoncounter < longpress)  && (buttoncounter > 0))
        {
            newhours++;
            buttoncounter = 0;
        }
        if (buttoncounter >= longpress)
        {
            buttoncounter = 0;
            break;
        }

        if (newhours >= 12) {
            newhours = 0;
        }
        setHand(newhours,255, false, true);
        setHand(newminutes,255, true, true);
        showArray(75);
        delay(75);
    }
    //anim_pop_in();
    playAnimation(2);
    buttoncounter =0;
    buttoncounter = 0;
    //setting minutes
    while (1) {
        while (isButtonPressed()) {
            buttoncounter++;
            setHand(newhours,255, false, true);
            setHand(newminutes,255, true, true);
            if (buttoncounter < longpress)
            {
                showArray(0);
            }
        }
        clearArrays();
        if ((buttoncounter < longpress)  && (buttoncounter > 0))
        {
            newminutes++;
            buttoncounter = 0;
        }
        if (buttoncounter >= longpress)
        {
            buttoncounter = 0;
            break;
        }

        if (newminutes >= 12) {
            newminutes = 0;
        }
        setHand(newhours,255, false, true);
        setHand(newminutes,255, true, true);
        showArray(75);
        delay(75);
		buttoncounter = 0;
    }
    //anim_pop_in();
    playAnimation(2);

    //newminutes++;
    //newhours++;

    if (newminutes >= 12) {
        newminutes = 0;
    }
    if (newhours >= 12) {
        newhours = 12;
    }
	Serial.println("New Data");
    Serial.println(newminutes);
	Serial.println(newhours);
	newminutes = newminutes*5;

    setTime(DateTime(2017, 0, 0, newhours, newminutes, 0));
	

	
	printDate();
    buttoncounter = 0;
    //setting special hours
    while (1) {
        while (isButtonPressed()) {
            buttoncounter++;
            setHand(10,255, true, true);
            setHand(6,255, false, true);
            if (specialhour == true) {
                setHand(7,100, false, true);
            }
            if (buttoncounter < longpress)
            {
                showArray(0);
            }
        }
        clearArrays();
        if ((buttoncounter < longpress)  && (buttoncounter > 0))
        {
            specialhour = !specialhour;
            buttoncounter = 0;
        }
        if (buttoncounter >= longpress)
        {
            buttoncounter = 0;
            break;
        }
        setHand(10,255, true, true);
        setHand(6,255, false, true);
        if (specialhour == true) {
            setHand(7,100, false, true);
        }
        showArray(75);
        delay(75);
    }
    //anim_pop_in();
    playAnimation(2);
    setState(GO_TO_SLEEP);
}

//------------------------------------------------------------------
/*
 * Function:  sleepnow
 * --------------------
 *  When this function is called an intterupt will be attached to watchbutton, processor will be put in low power config, and will enter sleepmode.
 *  The watch will use very little power now.
 *  Processor can only be woken up by either a reset or pressing the watchbutton.
 *
 *  When waking up processor will continue from "sleep_disable();" line
 *
 *  1 press -> state = SHOW_TIME
 *
 *  2 presses -> state = DO_GAME
 *
 *  returns: nothing
 */
//------------------------------------------------------------------
void Velleman_VMW100::sleep() 
{

    // debugging
#ifdef DEBUG
    //Serial.println("Watch going to sleep");
    //delay(5);
#endif
    /*
    // Attach interrupt to pin so we can wakup the device
    //attachPinChangeInterrupt(_watchButtonPin, wakeup, RISING);
    enableInterrupt(_watchButtonPin,wakeup,RISING);

    // Choose our preferred sleep mode:
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

    // Set sleep enable (SE) bit:
    sleep_enable();

    // Put the device to sleep:
    sleep_mode();

    // Upon waking up, watch continues from this point.
    sleep_disable();

    // Detaching interrupt so we can use the button
    //detachPinChangeInterrupt(_watchButtonPin);
    disableInterrupt(_watchButtonPin);

    // debugging
    #ifdef DEBUG
      Serial.println("Watch woke up");
    #endif
    */
    buttoncounter = 0;
    if (isButtonPressed())
    {
		
        setState(SHOW_TIME);
        // check for second press after wakeup
		//delay(750);
		while(isButtonPressed());
        while (isButtonPressed() || buttoncounter < 500) {
            buttoncounter++;
            delay(1);
            if (isButtonPressed())
            {
                buttoncounter = 0;
                setState(DO_GAME);
                break;
            }
        }
    }
	buttoncounter = 0;
}


bool Velleman_VMW100::isButtonPressed() 
{
    return digitalRead(_watchButtonPin);
}

void Velleman_VMW100::setBeginAnimation(anim_ptr_t fptr)
{
    anim_ptr_array[0] = fptr;
}

void Velleman_VMW100::setEndAnimation(anim_ptr_t fptr)
{
    anim_ptr_array[1] = fptr;
}

void Velleman_VMW100::playAnimation(int posAnimation)
{
    if (posAnimation > 0)
    {
        anim_ptr_array[posAnimation-1]();
    }
}

void Velleman_VMW100::addGame(game_ptr_t fptr) 
{
    game = fptr;
}

void Velleman_VMW100::executeGame() 
{
    if (game != NULL)
        game();
}

void Velleman_VMW100::printDate() 
{
#ifdef DEBUG
    char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
#endif
}

void Velleman_VMW100::setBrightness(uint8_t brightness)
{
	strip.setBrightness( brightness > 128 ? 128 : brightness);
}