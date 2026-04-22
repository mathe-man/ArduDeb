//

//

#pragma once
#include <Arduino.h>


#define BoardName "Board"

#define MessageSeparator "|"

#define ArduDeb_MagicNumber 0xADEB
#define MessagesBufferSize 128

#define UnsignedInt uint16_t // This type can be changed depending of the needs

class Event;

struct ArduDebMessage {
    const char* sender;
    const char* content;

    ArduDebMessage(const char* sender, const char* content) : sender(sender), content(content) {}

    UnsignedInt inline Length() const {
        return strlen(sender) + strlen(content) + strlen(MessageSeparator) + 1; // +1 for the "=" separator
    }
    const char* Build() const {
        char* message = new char[strlen(sender) + strlen(content) + 2]; // +2 for "=" separator and null terminator
        
        strcpy(message, sender);
        strcat(message, "=");
        strcat(message, content);
        strcat(message, MessageSeparator);

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
    char* eventsMessagesBuffer[MessagesBufferSize]; 

    bool inline FitInBuffer(ArduDebMessage message) {
        return MessagesBufferSize - 1 > message.Length(); // -1 for null terminator
    }

    UnsignedInt inline EmptyBufferSpace() {
        return MessagesBufferSize - strlen(BoardName) - 2;
    }

    void inline ClearBuffer() {
        eventsMessagesBuffer[0] = "\0";
    }
   
    void inline AddToBuffer(ArduDebMessage message) {
        strcat(eventsMessagesBuffer[0], message.Build());   // The message build already include separator
    }

    

#define MaxLogAttempt 5
    bool LogEvent(ArduDebMessage message, uint8_t attempt = 0) {
        if (attempt >= MaxLogAttempt) {
            return false;       // Max log attempts reached, give up
        }
        // Check if the message size is acceptable
        if (message.Length() > EmptyBufferSpace()) { // -2 for null terminator and separator (board|messages)) {
            Serial.print("Message too long to fit in buffer, sender: ");
            Serial.println(message.sender);

            return false;
        }
        // Check if the message can fit in the buffer
        if (!FitInBuffer(message)) {
            FlushBuffer();
            return LogEvent(message, attempt++); // Try to log the message again after flushing the buffer
        }

        // All the test passed => Add the message to the buffer and return true
        AddToBuffer(message);
        return true;
    }

    void FlushBuffer()
    {
        // Send the serial communication with the format: MagicNumber|BoardName|message1|message2|...|messageN (Assuming the separator is '|')
        print(ArduDeb_MagicNumber); print(MessageSeparator);
        print(BoardName); print(MessageSeparator);
        
        println(eventsMessagesBuffer[0]); // Print the messages

        ClearBuffer();
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