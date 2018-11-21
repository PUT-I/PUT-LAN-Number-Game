# TS_PROJEKT_L7_15
University project concerning web protocols.

Protocol field:
  • Operation
  • Answer
  • Session identificator
  • Data
  • Padding

Byte  : 0	             |1	                   |2
Bit	  : 0	1	2 3 4	5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
Field : Op   |Answ |Session Id|Data                   |Padding


Operation and answer codes:
  •	000 – message:
    o	000 – sender disconnected (receiving error)
    o	001 – opponent disconnected
    o	010 – waiting for opponent
  •	001 – game:
    o	000 – game start
    o	001 – draw
    o	010 – won
    o	011 – lost
    o	111 – game end
  •	010 – data:
    o	000 – session id
    o	001 – number
  •	011 – number:
    o	000 – number too small
    o	001 – number too big
  •	100 – time:
    o	000 – game duration
    o	001 – time to game start
    o	010 – time to game end
