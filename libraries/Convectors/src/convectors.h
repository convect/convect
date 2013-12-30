/*
 * Convect v1.0
 * (c) 2013 D.M.
 */

#include <Arduino.h>

enum ConvectRoom {
    BEDROOM,
    BATHROOM,
    KITCHEN,
    LIVINGROOM,
    BEDROOM_2,

    MAX_ROOM
};

class TempSensors
{
public:
    TempSensors(int frequency, float aref):
        _onesec(frequency),
        _time(0),
        _aref(aref)
    {
        analogReference(EXTERNAL);
        for (int room = 0; room < MAX_ROOM; room++) {
            _lastTemp[room] = getInstantTemp((ConvectRoom)room);
            _samples[room] = new int[60 * _onesec];
        }
    }

    ~TempSensors()
    {
        for (int room = 0; room < MAX_ROOM; room++) {
            delete[] _samples[room];
        }
    }

    void cycle()
    {
        if (_time == 60 * _onesec) {
            int average[MAX_ROOM][32];
            int curTemp[MAX_ROOM];
            _time = 0;
            for (int room = 0; room < MAX_ROOM; room++) {
                for (int step = 0; step < 2 * _onesec; step++) {
                    average[room][step] = 0;
                }
                curTemp[room] = 0;
            }
            for (int room = 0; room < MAX_ROOM; room++) {
                for (int step = 0; step < 2 * _onesec; step++) {
                    for (int sample = 30 * step; sample < 30 * (step + 1); sample++) {
                        average[room][step] += _samples[room][sample];
                    }
                    average[room][step] = (average[room][step] + 15) / 30;
                }
            }
            for (int room = 0; room < MAX_ROOM; room++) {
                for (int step = 0; step < 2 * _onesec; step++) {
                    curTemp[room] += average[room][step];
                }
                curTemp[room] = (curTemp[room] + _onesec) / (2 * _onesec);
                _lastTemp[room] = (((curTemp[room] * _aref) / 1023.0) * 100) - 273.15;
            }
        }

        for (int room = 0; room < MAX_ROOM; room++) {
            _samples[room][_time] = analogRead(room);
        }

        _time++;
    }

    float getTemp(ConvectRoom room)
    {
        return _lastTemp[room];
    }

    float getInstantTemp(ConvectRoom room)
    {
        int val = analogRead(room);
        return (((val * _aref) / 1023.0) * 100) - 273.15;
    }

private:

    float _aref;
    int* _samples[MAX_ROOM];
    float _lastTemp[MAX_ROOM];
    int _onesec;
    int _time;
};

struct relays {
    int pos;
    int neg;
};

static const relays pinTemplate[MAX_ROOM] = { { 3, 2}, { 5, 4}, { 7, 6}, { 9, 8}, { 11, 10} };

class Convectors {
public:
        enum Mode {
            MODE_OFF,
            MODE_COMFORT,
            MODE_ECO,
            MODE_NO_FREEZE,
            MODE_COMFORT_1,
            MODE_COMFORT_2,

            MAX_MODE
        };

        Convectors(int frequency): 
            _onesec(frequency),
            _time(0)
        {
            for (int i = 2; i < MAX_ROOM * 2 + 2; i++) {
                pinMode(i, OUTPUT);
            }
            for(int i = 0; i < MAX_ROOM; i++) {
                setMode((ConvectRoom)i, MODE_OFF);
            }
            memcpy(_pins, pinTemplate, sizeof(_pins));
        }

        void cycle()
        {
            if (_time == 300 * _onesec) {
                _time = 0;
            }

            for (int room = 0; room < MAX_ROOM; room++) {
                if (_mode[room] == MODE_COMFORT_1 || _mode[room] == MODE_COMFORT_2) {
                    if (_time / _onesec < (_mode[room] == MODE_COMFORT_1 ? 3 : 7)) {
                        digitalWrite(_pins[room].pos, LOW);
                        digitalWrite(_pins[room].neg, HIGH);
                    } else {
                        digitalWrite(_pins[room].pos, HIGH);
                        digitalWrite(_pins[room].neg, LOW);
                    }
                }
            }

            _time++;
        }

        void setMode(ConvectRoom room, Mode mode)
        {
            if (mode < 0 || mode >= MAX_MODE || room < 0 || room >= MAX_ROOM) {
                return;
            }
            _mode[room] = mode;
            switch(mode) {
                case MODE_OFF:
                    digitalWrite(_pins[room].pos, LOW);
                    digitalWrite(_pins[room].neg, LOW);
                    break;
                case MODE_COMFORT:
                    digitalWrite(_pins[room].pos, HIGH);
                    digitalWrite(_pins[room].neg, LOW);
                    break;
                case MODE_ECO:
                    digitalWrite(_pins[room].pos, LOW);
                    digitalWrite(_pins[room].neg, HIGH);
                    break;
                case MODE_NO_FREEZE:
                    digitalWrite(_pins[room].pos, HIGH);
                    digitalWrite(_pins[room].neg, HIGH);
                    break;
                case MODE_COMFORT_1:
                case MODE_COMFORT_2:
                default:
                    break;
            }
        }
private:

    struct relays _pins[MAX_ROOM];
    int _onesec;
    int _time;
    enum Mode _mode[MAX_ROOM];
};


