# TS_PROJEKT_L7_15
University project concerning web protocols.

## Protocol Description
### Protocol Fields:
  - Operation (3 bit)
  - Answer (3 bit)
  - Session identificator (5 bit)
  - Data (8 bit)
  - Padding (5 bit)

### Protocol Structure:
<pre>
Byte  : 0                      |1                      |2
Bit   : 00 01 02|03 04 05|06 07 08 09 10|11 12 13 14 15 16 17 18|19 20 21 22 23
Field : Operat  |Answer  |Session Id    |Data                   |Padding
</pre>
### Operation And Answer Codes:
  -	000 – message:
    -	000 – sender disconnected (receiving error)
    -	001 – opponent disconnected
    -	010 – waiting for opponent
  -	001 – game:
    -	000 – game start
    -	001 – draw
    -	010 – won
    -	011 – lost
    -	111 – game end
  -	010 – data:
    -	000 – session id
    -	001 – number
  -	011 – number:
    -	000 – number too small
    -	001 – number too big
  -	100 – time:
    -	000 – game duration
    -	001 – time to game start
    -	010 – time to game end
