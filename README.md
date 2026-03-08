# Remote Camera Shutter

# DE

Kamerafernauslöser für Smartphones. Der M5Stick wird vom Smartphone aus gekoppelt, dann kann mit der Taste des Sticks ein Foto aufgenommen werden, wenn die Kameraapp geöffnet ist.

## Tested with

 - Samsung Galaxy S25 Ultra
 - other Models should work to

## Known Problem

```
.pio/libdeps/m5stick-c/DFRobot_GP8XXX/DFRobot_GP8XXX.cpp: In member function 'int DFRobot_GP8XXX_PWM_SINGLE::begin()':
.pio/libdeps/m5stick-c/DFRobot_GP8XXX/DFRobot_GP8XXX.cpp:303:35: error: too many arguments to function 'void analogWriteResolution(uint8_t)'
```

```
Fix to   
#if defined(ESP32) 
    analogWriteResolution(10);  
```

## See also

 - https://docs.m5stack.com/en/core/M5StickC%20PLUS2
