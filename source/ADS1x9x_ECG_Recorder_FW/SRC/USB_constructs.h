BYTE hid_sendDataWaitTilDone(BYTE* dataBuf, WORD size, BYTE intfNum, ULONG ulTimeout);
BYTE hid_sendDataInBackground(BYTE* dataBuf, WORD size, BYTE intfNum, ULONG ulTimeout);
WORD hid_receiveDataInBuffer(BYTE*,WORD,BYTE);

BYTE cdc_sendDataWaitTilDone(BYTE* dataBuf, WORD size, BYTE intfNum, ULONG ulTimeout);
BYTE cdc_sendDataInBackground(BYTE* dataBuf, WORD size, BYTE intfNum, ULONG ulTimeout);
WORD cdc_receiveDataInBuffer(BYTE*,WORD,BYTE);
