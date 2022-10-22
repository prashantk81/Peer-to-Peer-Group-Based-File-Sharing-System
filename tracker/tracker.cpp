#include <bits/stdc++.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/types.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
using namespace std;
string trackerIP;
uint16_t trackerPORT;
unordered_map<string, bool> checkLogin;
unordered_map<string, string> loginData;
unordered_map<string, string> portConvert;
unordered_map<string, string> groupOfAdmin;
unordered_map<string, set<string>> requestsInGroup;
unordered_map<string, set<string>> membersInGroup;
unordered_map<string, string> fileSizeCurr;
unordered_map<string, string> pieceWise;
vector<string> listOfgroups;
string pass, usr;
unordered_map<string, unordered_map<string, set<string>>> listOfUpload;
FILE *pk;
vector<string> divideString(string str, string symbol)
{
    size_t pos = 0;
    string str1;
    vector<string> myAns;
    int size = symbol.length();
    while ((pos = str.find(symbol)) != string::npos)
    {
        str1 = str.substr(0, pos);
        myAns.push_back(str1);
        str.erase(0, pos + size);
    }
    myAns.push_back(str);
    return myAns;
}
void addLogger(string str)
{
    fprintf(pk, "%s", str.c_str());
}

bool pathFind(const string &str)
{
    struct stat itrator;
    // These functions return information about a file, in the buffer
    // pointed to by statbuf.
    if (stat(str.c_str(), &itrator) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
    return false;
}

void *routine(void *argument)
{
    // used to read a string or a line from an input stream
    // getline() function extracts characters from the input stream and appends it to the string object
    while (true)
    {
        string str;
        getline(cin, str);
        if (str == "quit")
        {
            fclose(pk); ///////////////////////////////////////
            exit(0);
        }
    }
}
void fileUpoloadToTracker(string pathToFile, string gpID, int nSocket, string peerID)
{
    int kind = 1;
    if (!pathFind(pathToFile))
    {
        kind = 0;
        write(nSocket, "\n=> No such file exists", 23);
        addLogger("\n=> No such file exists");
    }
    else if (membersInGroup.find(gpID) == membersInGroup.end())
    {
        kind = -1;
        write(nSocket, "\n=> No Such group exists", 24);
        addLogger("\n=> No Such group exists");
    }
    else if (membersInGroup[gpID].find(peerID) == membersInGroup[gpID].end())
    {
        kind = 1;
        write(nSocket, "\n=> No client Present", 22);
        addLogger("\n=> No client Present");
    }
    else
    {
        write(nSocket, "\n=> Uploading...", 17);
        addLogger("\n=> Uploading...");
        int varchar = 0;
        char bufferFile[524288] = {0};
        if (read(nSocket, bufferFile, 524288))
        {
            if (string(bufferFile) == "error")
            {
                return;
            }
            vector<string> detailsOfCurrentFile = divideString(string(bufferFile), "##");
            string nameOfFile = divideString(string(detailsOfCurrentFile[0]), "/").back();
            string result = "";
            for (size_t i = 4; i < detailsOfCurrentFile.size(); i++)
            {
                result += detailsOfCurrentFile[i];
                if (i != detailsOfCurrentFile.size() - 1)
                {
                    result += "##";
                }
            }
            pieceWise[nameOfFile] = result;
            while (varchar > 0)
            {
                write(nSocket, "\n=> Uploading...", 17);
                addLogger("\n=> Uploading...");
                varchar++;
            }
            if (listOfUpload[gpID].find(nameOfFile) != listOfUpload[gpID].end())
            {
                listOfUpload[gpID][nameOfFile].insert(peerID);
                varchar = 0;
            }
            else
            {
                listOfUpload[gpID].insert({nameOfFile, {peerID}});
            }
            string hs = "";
            if (0)
            {
                hs += "&&&";
            }
            fileSizeCurr[nameOfFile] = detailsOfCurrentFile[2];
            write(nSocket, "*************** 100% **************\n=> Successfully Uploaded", 61);
            addLogger("\n=> Successfully Uploaded");
        }
    }
}

void fileDownloading(string gpID, string nameOfFile, string pathToFile, int nSocket, string peerID)
{
    int state = 1;
    if (!pathFind(pathToFile))
    {
        state = 0;
        write(nSocket, "\n=> No such file exist", 23);
        addLogger("\n=> No such file exist");
    }
    else if (membersInGroup.find(gpID) == membersInGroup.end())
    {
        state = -1;
        write(nSocket, "\n=> No Such group exists", 25);
        addLogger("\n=> No Such group exists");
    }
    else if (membersInGroup[gpID].find(peerID) == membersInGroup[gpID].end())
    {
        state = 0;
        write(nSocket, "\n=> No client Present", 22);
        addLogger("\n  No client Present");
    }
    else
    {
        string spcs = "";
        int tk = 0;
        char bufferFile[524288] = {0};
        tk += 1;
        write(nSocket, "\n=> Downloading...", 19);
        spcs += "##";
        addLogger("\n=> Downloading...");
        if (read(nSocket, bufferFile, 524288))
        {
            vector<string> detailsOfCurrentFile = divideString(string(bufferFile), "##");
            string replyBack = "";
            if (listOfUpload[gpID].find(detailsOfCurrentFile[0]) != listOfUpload[gpID].end())
            {
                for (auto i : listOfUpload[gpID][detailsOfCurrentFile[0]])
                {
                    if (true && checkLogin[i])
                    {
                        replyBack += portConvert[i] + "##";
                    }
                    else
                    {
                        ;
                    }
                }
                replyBack += fileSizeCurr[detailsOfCurrentFile[0]] + "&&&" + pieceWise[detailsOfCurrentFile[0]];
                write(nSocket, &replyBack[0], replyBack.length());
                listOfUpload[gpID][nameOfFile].insert(peerID);
            }
            else
            {
                write(nSocket, "\n=> File not found", 19);
                addLogger("\n=> File not found");
            }
        }
    }
}

void connectionEstablish(int nSocket)
{
    cout << "\n=> Thread Created : " << to_string(nSocket) << endl;
    // addLogger("\n=> Thread Created : " + to_string(nSocket));
    string peerID = "Client", peerGroupID;
    string input, present;
    while (true)
    {
        char queue[2048] = {0};
        vector<string> arrayOfInput;
        // read() function reads N bytes of input into the memory area indicated by buf.
        if (read(nSocket, queue, 2048) <= 0)
        {
            checkLogin[peerID] = false;
            close(nSocket);
            break;
        }
        input = string(queue);
        cout << "\n=> Request is coming from " << peerID << " : " << input << endl;
        addLogger("\n=> Request is coming from " + peerID + " " + input);
        stringstream arrayOfString(input);
        // copying all input into arrayofinput vector
        while (arrayOfString >> present)
        {
            arrayOfInput.push_back(present);
        }
        // if peer wants to create account
        if (arrayOfInput[0] == "create_user")
        {
            int kind = 0;
            if (arrayOfInput.size() != 3)
            {
                // writes data from a buffer declared by the user to a given device, such as a file. T
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
            else
            {
                kind = -1;
                string UserID = arrayOfInput[1];
                string UserPASS = arrayOfInput[2];
                string varchar = "";
                int flag;
                if (loginData.find(UserID) == loginData.end())
                {
                    kind = -1;
                    loginData.insert({UserID, UserPASS});
                    flag = 0;
                }
                else
                {
                    kind = 0;
                    flag = -1;
                }
                varchar += "##";
                while (false)
                {
                    kind = kind + flag;
                    varchar += "&&&";
                }
                if (flag == -1)
                {
                    kind = -1;
                    write(nSocket, "\n=> User Already Exists", 24);
                    addLogger("\n=> User Already Exists");
                }
                else
                {
                    kind = 1;
                    write(nSocket, "\n=> Account created successfully ", 32);
                    addLogger("\n=> Account created successfully ");
                }
            }
        }
        else if (arrayOfInput[0] == "login")
        {
            if (arrayOfInput.size() != 3)
            {
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
            else
            {
                pass = arrayOfInput[2];
                usr = arrayOfInput[1];
                char loginChecker[1024];
                peerID = arrayOfInput[1];
                string UserPass = arrayOfInput[2];
                int flag, u = 0;
                if (loginData.find(peerID) == loginData.end() || loginData[peerID] != UserPass)
                {
                    flag = -1;
                    u = 1;
                }
                else if (checkLogin.find(peerID) == checkLogin.end())
                {
                    checkLogin.insert({peerID, true});
                    flag = 0;
                    u = 1;
                }
                else
                {
                    if (checkLogin[peerID] == false)
                    {
                        checkLogin[peerID] = true;
                        flag = 0;
                        u = 1;
                    }
                    else
                    {
                        flag = 1;
                        u = 1;
                    }
                }
                if (u == 0)
                {
                    flag = 0;
                }

                if (flag == 0)
                {
                    write(nSocket, "\n=> Login Successful", 21);
                    addLogger("\n=> Login Successful");
                    read(nSocket, loginChecker, 1024);
                    portConvert[peerID] = string(loginChecker);
                }
                else if (flag == 1)
                {
                    write(nSocket, "\n=> User has Already LoggedIn Some other Terminal", 49);
                    addLogger("\n=> User has Already LoggedIn Some other Terminal");
                }
                else
                {
                    write(nSocket, "\n=> create account before login", 31);
                    addLogger("\n=> create account before login");
                }
            }
        }
        else if (arrayOfInput[0] == "create_group")
        {
            if (arrayOfInput.size() == 2)
            {
                string varchar = "";
                string gpID = arrayOfInput[1];
                int flag = 0;
                int kind = 0;
                for (string i : listOfgroups)
                {
                    kind = 1;
                    if (i == gpID)
                    {
                        write(nSocket, "\n=> Group Already Exists!! Create another group", 48);
                        addLogger("\n=> Group Already Exists!! Create another group");
                        flag = 1;
                    }
                }
                while (false)
                {
                    kind = -1;
                    write(nSocket, "\n=> Group created Successfully", 31);
                    addLogger("\n=> Group created Successfully");
                }
                if (flag == 0)
                {
                    kind = 0;
                    listOfgroups.push_back(gpID);
                    groupOfAdmin.insert({gpID, peerID});
                    varchar += "##";
                    membersInGroup[gpID].insert(peerID);
                    kind = kind + 1;
                    peerGroupID = arrayOfInput[1];
                    write(nSocket, "\n=> Group created Successfully", 31);
                    addLogger("\n=> Group created Successfully");
                    kind -= 1;
                }
            }
            else
            {
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else if (arrayOfInput[0] == "join_group")
        {
            if (arrayOfInput.size() == 2)
            {
                int spc = 0;
                string gpID = arrayOfInput[1];
                string varchar = "";
                if (groupOfAdmin.find(gpID) == groupOfAdmin.end())
                {
                    spc = -1;
                    write(nSocket, "\n=> Wrong Group ID", 18);
                    addLogger("\n=> Wrong Group ID");
                }
                else if (membersInGroup[gpID].find(peerID) != membersInGroup[gpID].end())
                {
                    spc = 1;
                    write(nSocket, "\n=> You are already present in group", 36);
                    addLogger("\n=> You are already present in group");
                }
                else
                {
                    spc = 0;
                    requestsInGroup[gpID].insert(peerID);
                    varchar += "##";
                    write(nSocket, "\n=> Group request successfully sent", 35);
                    addLogger("\n=> Group request successfully sent");
                }
            }
            else
            {
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else if (arrayOfInput[0] == "leave_group")
        {
            if (arrayOfInput.size() == 2)
            {
                string varchar = "";
                int spc = 0;
                string gpID = arrayOfInput[1];
                varchar += "##";
                if (groupOfAdmin.find(gpID) == groupOfAdmin.end())
                {
                    spc = -1;
                    write(nSocket, "\n=> No group found", 19);
                    addLogger("\n=> No group found");
                }
                else if (membersInGroup[gpID].find(peerID) != membersInGroup[gpID].end())
                {
                    spc = 1;
                    int flag = 0;
                    varchar += "##";

                    if (groupOfAdmin[gpID] != peerID)
                    {
                        spc = 0;
                        membersInGroup[gpID].erase(peerID);
                        write(nSocket, "\n=> Group left succesfully!!", 29);
                        addLogger("\n=> Group left succesfully!!");
                    }
                    else
                    {
                        spc = 0;
                        write(nSocket, "\n=> You are admin, you cannot leave", 36);
                        addLogger("\n=> You are admin, you cannot leave");
                    }
                }
                else
                {
                    spc = -1;
                    write(nSocket, "\n=> You are not part of group", 29);
                    addLogger("\n=> You are not part of group");
                }
            }
            else
            {
                string varchar = "##";
                int spc = 0;
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else if (arrayOfInput[0] == "list_requests")
        {
            int kind = 1;
            if (arrayOfInput.size() == 2)
            {
                string varchar = "";
                string gpID = arrayOfInput[1];
                string req = "";
                varchar += "##";
                kind = 1;
                if (groupOfAdmin.find(gpID) == groupOfAdmin.end())
                {
                    kind = 0;
                    write(nSocket, "\n=> No group found##", 21);
                    addLogger("\n=> No group found");
                }
                if (groupOfAdmin[gpID] != peerID)
                {
                    kind = -1;
                    write(nSocket, "\n=> You are not admin##", 24);
                    addLogger("\n=> You are not admin");
                }
                else if (requestsInGroup[gpID].size() == 0)
                {
                    kind = 0;
                    write(nSocket, "\n=> No requests pending##", 26);
                    addLogger("\n=> No requests pending");
                }
                else
                {
                    kind = 1;
                    for (auto i = requestsInGroup[gpID].begin(); i != requestsInGroup[gpID].end(); i++)
                    {
                        req += string(*i) + "##";
                    }
                    while (kind > 2)
                    {
                        cout << "No requests pending##";
                    }
                    write(nSocket, &req[0], req.length());
                }
            }
            else
            {
                while (kind > 2)
                {
                    string varchar = "";
                    kind = kind - 1;
                }
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else if (arrayOfInput[0] == "accept_request")
        {
            if (arrayOfInput.size() == 3)
            {
                int kind = 0;
                string gpID = arrayOfInput[1];
                string UserID = arrayOfInput[2];
                string varchar = "";
                if (groupOfAdmin.find(gpID) == groupOfAdmin.end())
                {
                    kind = -1;
                    write(nSocket, "\n=> No such group exist", 22);
                    addLogger("\n=> No such group exist");
                }
                else if (groupOfAdmin.find(gpID)->second != peerID)
                {
                    kind = 1;
                    write(nSocket, "\n=> You are not admin", 21);
                    addLogger("\n=> You are not admin");
                    varchar += "##";
                }
                else if (requestsInGroup[gpID].empty()) ///////////////////////////////
                {
                    kind = 0;
                    write(nSocket, "\n=> No request available!!", 27);
                    addLogger("\n=> No request available!!");
                }
                else
                {
                    write(nSocket, " \n=> Request accepted...", 24);
                    addLogger(" \n=> Request accepted...");
                    membersInGroup[gpID].insert(UserID);
                    requestsInGroup[gpID].erase(UserID);
                }
            }
            else
            {
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else if (arrayOfInput[0] == "list_groups")
        {
            int x = 0;
            if (true && arrayOfInput.size() != 1)
            {
                x += 1;
                write(nSocket, "\n=> Invalid Arguments!! Try again##", 36);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
            else if (true && listOfgroups.size() == 0)
            {
                x += 1;
                write(nSocket, "\n=> No groups available till now##", 34);
                addLogger("\n=> No groups available till now");
            }
            else
            {
                x = +1;
                string groupName;
                int i = 0;
                while (i < listOfgroups.size())
                {
                    x += 1;
                    groupName += listOfgroups[i] + "##";
                    i++;
                }
                write(nSocket, &groupName[0], groupName.length() + 1);
            }
        }
        else if (arrayOfInput[0] == "list_files")
        {
            if (arrayOfInput.size() == 2)
            {
                string varchar = "";
                string gpID = arrayOfInput[1];
                varchar += "##";
                int kind = 0;
                if (groupOfAdmin.find(gpID) == groupOfAdmin.end())
                {
                    kind = 1;
                    write(nSocket, "\n=> No group found!!!##", 24);
                    addLogger("\n=> No group found!!!");
                }
                else if (listOfUpload[gpID].size() != 0)
                {
                    kind = -1;
                    string replyBack = "";
                    for (auto i : listOfUpload[gpID])
                    {
                        replyBack += i.first + "##";
                    }
                    while (kind > 2)
                    {
                        varchar += "&&&";
                    }
                    write(nSocket, &replyBack[0], replyBack.length());
                }
                else
                {
                    kind = 0;
                    write(nSocket, "\n=> No files found##", 21);
                    addLogger("\n=> No files found");
                }
            }
            else
            {
                write(nSocket, "\n=> Invalid Arguments!! Try again##", 36);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else if (arrayOfInput[0] == "upload_file")
        {
            if (arrayOfInput.size() == 3)
            {
                fileUpoloadToTracker(arrayOfInput[1], arrayOfInput[2], nSocket, peerID);
            }
            else
            {
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else if (arrayOfInput[0] == "download_file")
        {
            if (arrayOfInput.size() == 4)
            {
                fileDownloading(arrayOfInput[1], arrayOfInput[2], arrayOfInput[3], nSocket, peerID);
            }
            else
            {
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else if (arrayOfInput[0] == "logout")
        {
            string varchar = "";
            checkLogin[peerID] = false;
            write(nSocket, "\n=> Logout Successful", 22);
            addLogger("\n=> Logout Successful");
            varchar += "##";
        }
        else if (arrayOfInput[0] == "show_downloads") // check it is not working
        {
            write(nSocket, "\n=> Downloads:- ", 17);
        }
        else if (arrayOfInput[0] == "stop_share")
        {
            if (arrayOfInput.size() == 3)
            {
                string varchar = "";
                int kind = 0;
                string gpID = arrayOfInput[1];
                string nameOfile = arrayOfInput[2];
                if (groupOfAdmin.find(gpID) == groupOfAdmin.end())
                {
                    kind = 1;
                    write(nSocket, "\n=> NO Group Found", 19);
                    addLogger("\n=> NO Group Found");
                }
                else if (listOfUpload[gpID].find(nameOfile) != listOfUpload[gpID].end())
                {
                    kind = -1;
                    listOfUpload[gpID][nameOfile].erase(peerID);
                    write(nSocket, "\n=> Stopped Sharing the file from group", 40);
                    addLogger("\n=> Stopped Sharing the file from group");
                    if (listOfUpload[gpID][nameOfile].size() == 0)
                    {
                        listOfUpload[gpID].erase(nameOfile);
                    }

                    // if (listOfUpload[gpID].size() == 0) ///////////////////////////////////////
                    // {
                    //     listOfUpload[gpID].clear();
                    // }
                }
                else
                {
                    int p = 0;
                    write(nSocket, "\n=> No such file exist in the group", 36);
                    addLogger("\n=> No such file exist in the group");
                }
            }
            else
            {
                write(nSocket, "\n=> Invalid Arguments!! Try again", 34);
                addLogger("\n=> Invalid Arguments!! Try again");
            }
        }
        else
        {
            write(nSocket, "\n=> Invalid Command!! Try Again", 32);
            addLogger("\n=> Invalid Command!! Try again");
        }
    }
    close(nSocket);
}

int main(int argc, char *argv[]) // argc -->no of argument passed+1; and argv vector contains args
{
    string pathOfLog = "LogFile.txt";
    pk = fopen(pathOfLog.c_str(), "a");
    if (argc != 2)
    {
        cout << "\n=>Invalid Number of arguments you pass!!" << endl;
        addLogger("=>Invalid Number of arguments you pass!!");
        return -1;
    }
    else
    {
        string trakerip_port = argv[1]; // tracker ip and port
        ifstream file(trakerip_port);
        addLogger("\n****************** New Session Initiated ******************\n");
        if (file.is_open())
        {
            string str = "";
            getline(file, str); // read one line from file(txt) and store in str
            int position = str.find(":");
            trackerIP = str.substr(0, position); // ip address
            string portStringformat = str.substr(position + 1);
            trackerPORT = stoi(portStringformat); // port number
        }
        else
        {
            cout << "\n=> Can't Open .txt file" << endl;
            addLogger("=> Can't Open .txt file");
            return -1;
        }
        // creation of a socket;
        int nSocket;
        struct sockaddr_in trackerAddress;
        nSocket = socket(AF_INET, SOCK_STREAM, 0);
        // SOCK_STREAM: TCP(reliable, connection oriented)
        // SOCK_DGRAM: UDP(unreliable, connectionless)
        if (nSocket < 0) // socket  not opened then it returns -1;
        {
            cout << "\n=> Failed to create Socket" << endl;
            addLogger("=> Failed to create Socket");
            return -1;
        }
        else
        {
            // cout << "\n=> Socket for server created successfully..." << endl;
        }
        int option = 1;
        if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
        {
            cout << "\n=> The setsockopt call failed" << endl;
            addLogger("=> The setsockopt call failed");
            return -1;
        }
        // for Tracker
        // Prepare the sockaddr_in structure
        trackerAddress.sin_family = AF_INET;
        trackerAddress.sin_port = htons(trackerPORT); // specify the port to listen on  htons--> host to network short
        // inet_pton() function converts an Internet address in its standard text format into its numeric binary form.
        // AF_INET and AF_INET6 address families are currently supported.
        if (inet_pton(AF_INET, &trackerIP[0], &trackerAddress.sin_addr) != 1)
        {
            // if successful,inet_pton() returns 1 and stores the binary form of the Internet address in the buffer(serv_addr.sin_addr).
            // If unsuccessful because the input(TRACKERIP) is not a valid string, inet_pton() returns 0.
            // If unsuccessful because the af(AF_INET) argument is unknown, inet_pton() returns -1 and sets errno to one of the following values:
            cout << "\n=> Given Address is not Supported" << endl;
            addLogger("=> Given Address is not Supported");
            return -1;
        }
        // The bind() function binds a unique local name to the socket with descriptor socket.
        if (bind(nSocket, (struct sockaddr *)&trackerAddress, sizeof(trackerAddress)) < 0)
        {
            cout << "\n=> The setsockopt call failed binding Failed" << endl;
            addLogger("=> The setsockopt call failed binding Failed");
            return -1;
        }
        else
        {
            // cout << "\n=> Binding with peer Completed" << endl;
        }
        int queueSize = 5;
        // listen() — Prepare the server for incoming client requests
        // listen(int socket,int backlog) backlog-- Defines the maximum length for the queue of pending connections.
        if (listen(nSocket, queueSize) < 0)
        {
            cout << "\n=> Listen Failed" << endl;
            addLogger("=> Listen Failed");
            return -1;
        }
        else
        {
            cout << "\n=> Tracker is ready to listen clients" << endl;
            addLogger("=> Tracker is ready to listen clients");
        }
        pthread_t threading;
        if (pthread_create(&threading, NULL, routine, NULL) < 0)
        {
            cout << "\n=> pthread Creation Error occured " << endl;
            addLogger("\n=> pthread Creation Error occured ");
            return -1;
        }
        vector<thread> threads;
        int len = sizeof(trackerAddress);
        // Accepting all the requests from client side
        while (true)
        {
            int peerSocket = accept(nSocket, (struct sockaddr *)&trackerAddress, (socklen_t *)&len);
            if (peerSocket < 0)
            {
                cout << "\n=> Error occured during Accepting requests" << endl;
                addLogger("\n=> Error occured during Accepting requests");
            }
            else
            {
                // cout << "\n=> Accept the connection request from client side" << endl;
            }
            threads.push_back(thread(connectionEstablish, peerSocket));
        }
        for (auto i = threads.begin(); i != threads.end(); i++)
        {
            if (i->joinable())
            {
                i->join();
            }
        }
        cout << "\n=> EXIT SERVER" << endl;
    }
    return 0;
}