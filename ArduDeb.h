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
        eventsMessagesBuffer[0] = '\0';
    }

    bool Flush() {
        return FlushBuffer();
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
    char eventsMessagesBuffer[MessagesBufferSize]; 

    bool inline FitInBuffer(ArduDebMessage message) {
        return strlen(eventsMessagesBuffer) + message.Length() + 1 < MessagesBufferSize; // +1 for null terminator
    }

    UnsignedInt inline  EmptyBufferSpace() {
        return MessagesBufferSize - 1; // -1 for null terminator
    }

    void inline ClearBuffer() {
        eventsMessagesBuffer[0] = '\0';
    }
   
    void inline AddToBuffer(ArduDebMessage message) {
        strcat(eventsMessagesBuffer, message.Build());   // The message build already include separator
    }

    
    // Event class can have access to logging
    friend class Event;

#define MaxLogAttempt 5
    bool LogEvent(ArduDebMessage message, uint8_t attempt = 0) {
        if (attempt >= MaxLogAttempt) {
            return false;       // Max log attempts reached, give up
        }
        // Check if the message size is acceptable
        if (message.Length() > EmptyBufferSpace()) {
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

    bool FlushBuffer()
    {
        if (strlen(eventsMessagesBuffer) == EmptyBufferSpace()) {
            return false; // Buffer is empty, no need to flush
        }
        // Send the serial communication with the format: MagicNumber|BoardName|message1|message2|...|messageN (Assuming the separator is '|')
        print(ArduDeb_MagicNumber); print(MessageSeparator);
        print(BoardName); print(MessageSeparator);
        
        println(eventsMessagesBuffer); // Print the messages

        ClearBuffer();
        return true;
    }
};



class Event {
public:
    Event(const char* name, ArduDeb* deb) : eventName(name), deb(deb) {}

    virtual const void flag() = 0;

    const char* getName() const {
        return eventName;
    }
    

protected:
    ArduDeb* deb;

    bool inline Log(const char* content) {
        ArduDebMessage message(getName(), content);
        return deb->LogEvent(message);
    }

private:
    const char* eventName;
};

class TickEvent : public Event {
public:
    TickEvent(ArduDeb* deb) : Event("TickEvent", deb) {}

    const void flag() override {
        Log("Tick");
    }
};