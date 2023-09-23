#include "GrblInterface.h"
#include <Regexp.h>
constexpr auto STATUS_REPORT_REGEX = "<(\w+)\|MPos:([\d\.,-]+),([\d\.,-]+),([\d\.,-]+)(.+)>";
constexpr auto TEST_STR = "<Idle|MPos:151.000,149.000,-1.000|Pn:XP|FS:0,0|WCO:12.000,28.000,78.000>";

GrblInterface grblInterface;

void matchCallback(const char *match, const unsigned int length, const MatchState &ms);

void setup() {
  Serial.begin(9600);
  MatchState ms(TEST_STR);
  auto count = ms.GlobalMatch(STATUS_REPORT_REGEX, matchCallback);
}

void loop() {
}

void matchCallback(const char *match,          // matching string (not null-terminated)
                   const unsigned int length,  // length of matching string
                   const MatchState &ms)       // MatchState in use (to get captures)
{
  char cap[40];

  Serial.print("Matched: ");
  Serial.write((byte *)match, length);
  Serial.println();

  for (byte i = 0; i < ms.level; i++) {
    Serial.print("Capture ");
    Serial.print(i, DEC);
    Serial.print(" = ");
    ms.GetCapture(cap, i);
    Serial.println(cap);
  }
}
