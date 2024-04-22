#ifndef STATES_HPP
#define STATES_HPP
#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include "HttpMessage.hpp"

class HttpRequest;

struct StateHandler;

typedef void (*StateHandlerFunc)(char c, HttpMessage& message, StateHandler& nextState);

struct StateHandler {
    StateHandlerFunc func;
};

namespace States
{
    void method(char c, HttpMessage& message, StateHandler& nextState);

    // URI
    void targetStart(char c, HttpMessage& message, StateHandler& nextState);
    void scheme(char c, HttpMessage& message, StateHandler& nextState);
    void colonSlashSlash(char c, HttpMessage& message, StateHandler& nextState);
    void authority(char c, HttpMessage& message, StateHandler& nextState);
    void port(char c, HttpMessage& message, StateHandler& nextState);
    void path(char c, HttpMessage& message, StateHandler& nextState);
    void query(char c, HttpMessage& message, StateHandler& nextState);
    void fragment(char c, HttpMessage& message, StateHandler& nextState);

    // HTTP version
    void httpVersion(char c, HttpMessage& message, StateHandler& nextState);
    void httpSlash(char c, HttpMessage& message, StateHandler& nextState);
    void ht(char c, HttpMessage& message, StateHandler& nextState);
    void htt(char c, HttpMessage& message, StateHandler& nextState);
    void http(char c, HttpMessage& message, StateHandler& nextState);
    void majorDigit(char c, HttpMessage& message, StateHandler& nextState);
    void minorDigit(char c, HttpMessage& message, StateHandler& nextState);
    void dot(char c, HttpMessage& message, StateHandler& nextState);

    // Headers
    void NL(char c, HttpMessage& message, StateHandler& nextState);
    void CR(char c, HttpMessage& message, StateHandler& nextState);
    void fieldName(char c, HttpMessage& message, StateHandler& nextState);
    void OWS(char c, HttpMessage& message, StateHandler& nextState);
    void SP(char c, HttpMessage& message, StateHandler& nextState);
    void fieldValue(char c, HttpMessage& message, StateHandler& nextState);
    void headerAlmostFinished(char c, HttpMessage& message, StateHandler& nextState);
    void headerFinished(char c, HttpMessage& message, StateHandler& nextState);

    // Body
    void finished(char c, HttpMessage& message, StateHandler& nextState);
} // namespace States

#endif // STATES_HPP