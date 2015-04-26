void CALLBACK cmdRPCList(std::string);
void CALLBACK cmdLog(std::string);
void CALLBACK cmdReload(std::string);
void CALLBACK cmdDrawRecv(std::string);
void CALLBACK cmdDrawSend(std::string);
void CALLBACK cmdCmdList(std::string);
void CALLBACK cmdSaveJSON(std::string);

bool CALLBACK RakHookIncomingRPC(stRakNetHookParams *);
bool CALLBACK RakHookOutcomingRPC(stRakNetHookParams *);
bool CALLBACK RakHookIncomingPacket(stRakNetHookParams *);
bool CALLBACK RakHookOutcomingPacket(stRakNetHookParams *);

bool CALLBACK D3DHookPresent(const RECT*, const RECT*, HWND, const RGNDATA*);
void CALLBACK GameDestructor(void);