#pragma once
#include <Arduino.h>


#define DefaultBoardDebugName "Board"

#define ArduDeb_MagicNumber 0xADEB
#define MessagesBufferSize 128


class Event;

struct ArduDebMessage {
    const char* sender;
    const char* content;

    ArduDebMessage(const char* sender, const char* content) : sender(sender), content(content) {}

    uint8_t inline Length() const {
        return strlen(sender) + strlen(content) + 1; // +1 for "=" separator
    }
    const char* Build() const {
        char* message = new char[strlen(sender) + strlen(content) + 2]; // +2 for "=" separator and null terminator
        strcpy(message, sender);
        strcat(message, "=");
        strcat(message, content);
        return message;
    }
};


class ArduDeb {
public:
    ArduDeb() {
        // Start serial communication
        Serial.begin(9600);
        // Initialize the events messages buffer
        eventsMessagesBuffer[0] = "\0";
    }

    template<typename T>    
    void print(const T& value) {
        Serial.print(value);
    }

    template<typename T>
    void println(const T& value) {
        Serial.println(value);
    }


private:
    char* eventsMessagesBuffer[128]; 

    bool inline FitInBuffer(ArduDebMessage message) {
        return MessagesBufferSize - 1 > message.Length(); // -1 for null terminator
    }

    UnsignedInt inline EmptyBufferSpace() {
        return MessagesBufferSize - strlen(BoardName) - 2;
    }

    bool LogEvent(ArduDebMessage message) {
        // Check if the message size is acceptable
        if (message.Length() > EmptyBufferSpace()) { // -2 for null terminator and separator (board|messages)) {
            Serial.print("Message too long to fit in buffer, sender: ");
            Serial.println(message.sender);

            return false;
        }
        // Check if the message can fit in the buffer
        if (!FitInBuffer(message)) {
            Serial.println("Buffer full, cannot log new event.");
            return false;
        }
    }
};



class Event {
public:
    Event(const char* name) : eventName(name) {}

    virtual const void flag() = 0;

    const char* getName() const {
        return eventName;
    }

    private:
        const char* eventName;
};

class TickEvent : public Event {
public:
    TickEvent() : Event("TickEvent") {}

    const void flag() override {
        // Implementation for flagging a tick event
    }
};