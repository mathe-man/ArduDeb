#pragma once
#include <Arduino.h>


#define BoardName "Board"

#define MessageSeparator "|"

#define ArduDeb_MagicNumber "\xAD\xEB" // Magic number to identify ArduDeb messages: 0xADEB
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

typedef void (*WriteFunction)(const char*);

class ArduDeb {
public:

    static void setWriteFunction(WriteFunction func) {
        writeFunction = func;
    }

    static inline bool Flush() {
        return FlushBuffer();
    }

    static inline bool Log(ArduDebMessage message)
    {
        return LogMessage(message);
    }

    static inline void print(const char* value) {
        writeFunction(value);
    }

    static inline void println(const char* value) {
        writeFunction(value);
        writeFunction("\n\r");  // Finish the line with a newline and carriage return
    }


private:

    // Function pointer for writing messages, use Serial.print by default
    static inline WriteFunction writeFunction = [](const char* message) {
        Serial.print(message);
    }; 

    static inline char eventsMessagesBuffer[MessagesBufferSize] = ""; // Buffer to hold the messages before flushing

    static inline bool FitInBuffer(ArduDebMessage message) {
        return strlen(eventsMessagesBuffer) + message.Length() + 1 < MessagesBufferSize; // +1 for null terminator
    }

    static inline UnsignedInt EmptyBufferSpace() {
        return MessagesBufferSize - 1; // -1 for null terminator
    }

    static inline void ClearBuffer() {
        eventsMessagesBuffer[0] = '\0';
    }
   
    static inline void AddToBuffer(ArduDebMessage message) {
        strcat(eventsMessagesBuffer, message.Build());   // The message build already include separator
    }

    
    // Event class can have access to logging
    friend class Event;

    #define MaxLogAttempt 5
    
    static inline bool LogMessage(ArduDebMessage message, uint8_t attempt = 0) {
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
            return LogMessage(message, attempt++); // Try to log the message again after flushing the buffer
        }

        // All the test passed => Add the message to the buffer and return true
        AddToBuffer(message);
        return true;
    }

    static inline bool FlushBuffer()
    {
        if (strlen(eventsMessagesBuffer) == 0) {
            return false; // Buffer is empty, no need to flush
        }
        // Send the serial communication with the format: MagicNumber|BoardName|message1|message2|...|messageN (Assuming the separator is '|')
        print(ArduDeb_MagicNumber); print(MessageSeparator);
        print(BoardName); print(MessageSeparator);
        println(eventsMessagesBuffer);
        ClearBuffer();
        return true;
    }
};



class Event {
public:
    Event(const char* name) : eventName(name) {}

    virtual const void flag() = 0;

    const char* getName() const {
        return eventName;
    }
    

protected:

    bool inline Log(const char* content) {
        ArduDebMessage message(getName(), content);
        return ArduDeb::LogMessage(message);
    }

private:
    const char* eventName;
};

class TickEvent : public Event {
public:
    TickEvent() : Event("TickEvent") {}

    const void flag() override {
        Log("Tick");
    }
};