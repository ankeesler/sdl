// A grammar for the sdl log output.
grammar SdlLog;

capture : 'CAPTURE' '{' packet+ '}' ;
packet : TIMESTAMP DIRECTION DATA ;

TIMESTAMP : '(' FLOAT ')' ;
FLOAT: [0-9]+ '.' [0-9]+ ;

DIRECTION : 'TX' | 'RX' ;

DATA : '[' HEXBYTES ']' | '[]' ;
HEXBYTES : (HEXBYTE ', ')* HEXBYTE ;
HEXBYTE : '0x' [0-9A-F]+ ;

WS : [ \r\n\t] -> skip ;
