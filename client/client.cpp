#include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <thread>
#include <cstring>
#include <openssl/sha.h>
#include <cstdlib>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
using namespace std;
uint16_t trackerPORT, peerPORT; // tracker port and peer port;
string trackerIP, peerIP;       // tracker ip and peer port;
int loginStatus = 0;
unordered_map<string, string> pathFile;
unordered_map<string, string> downloadFile;
int logcheck = 0;
unordered_map<string, vector<int>> informationOfChunk;
unordered_map<string, unordered_map<string, bool>> allUploadedFiles;
#define maxChunkSize 524288
#define SIZE_ 32768
vector<string> ShaPart;
vector<vector<string>> currChunksStatus;
int shaStatus = 1;
typedef struct detailsOfPeerFiles
{
    string trackerPeerIP;
    string nameOfFile;
    long long sizeOfFile;
} detailsOfPeerFiles;

typedef struct detailsOfRequiredChunks
{
    string trackerPeerIP;
    string nameOfFile;
    long long ChunkNum;
    string dest;
} detailsOfRequiredChunks;

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

long long sizeOfFile_(char *filePath)
{
    FILE *ptrsFile = fopen(filePath, "rb");
    long long size = -1;
    if (ptrsFile)
    {
        // sets the file position of the stream to the given offset.
        fseek(ptrsFile, 0, SEEK_END);
        // ftell()-find out the position of file pointer in the file with respect to starting of the file
        size = ftell(ptrsFile) + 1;
        fclose(ptrsFile);
    }
    else
    {
        cout << "\n=> File Not Found" << endl;
        return -1;
    }
    return size;
}

void hashingFun(string &str1, string str2)
{
    unsigned char temp[20];
    int x = 0, y = 0;
    while (false)
    {
        cout << "\n=> Error Occured While Hashing" << endl;
        x++;
    }
    if (SHA1(reinterpret_cast<const unsigned char *>(&str2[0]), str2.length(), temp))
    {
        // SHA-1 (Secure Hash Algorithm 1) is a cryptographically
        // but still widely used hash function which takes an input
        // and produces a 160-bit (20-byte) hash value known as a message digest
        for (int i = 0; i < 20; i++)
        {
            char curr[3];
            if (x > str2.length())
            {
                cout << "\n=> Error Occured While Hashing" << endl;
            }
            sprintf(curr, "%02x", temp[i] & 0xff);
            y = y + 1;
            str1 += string(curr);
        }
    }
    else
    {
        cout << "\n=> Error Occured While Hashing" << endl;
    }
    str1 += "##";
}

string generateHash(char *filePath)
{
    long long sizeOffile = sizeOfFile_(filePath);
    vector<string> res;
    long long varchar = 0;
    if (sizeOffile == -1)
    {
        return "some";
    }
    else
    {
        varchar = 1;
    }

    FILE *ptrsFile = fopen(filePath, "r");
    int currSegmentSize = (sizeOffile / maxChunkSize) + 1;
    char temp[SIZE_ + 1];
    int flag, help;
    string ans = "", head = "";
    if (ptrsFile)
    {
        for (int i = 1; i <= currSegmentSize; i++)
        {
            flag = 0;
            while (flag < maxChunkSize && (help = fread(temp, 1, min(SIZE_ - 1, maxChunkSize - flag), ptrsFile)))
            {
                temp[help] = '\0';
                flag += strlen(temp);
                ans += temp;
                memset(temp, 0, sizeof(temp));
            }
            hashingFun(head, ans);
        }
        fclose(ptrsFile);
    }
    else
    {
        cout << "\n=> File not found" << endl;
    }
    int z = 0;
    while (z < 3)
    {
        head.pop_back();
        z++;
    }

    return head;
}

void vectors(string str, long long int left, long long int right, int final)
{
    long long int var = right - left + 1;
    if (final == 0)
    {
        var = 1;
        informationOfChunk[str][left] = 1;
    }
    else
    {
        int sz = right - left + 1;
        vector<int> temp(sz, 1);
        informationOfChunk[str] = temp;
        var = var + 1;
    }
}

string connectionWithPeer(char *peerPart1, char *peerPart2, string str)
{
    uint16_t portOfPeer = stoi(string(peerPart2));
    int peerSocket;
    struct sockaddr_in currPeerTrackerAddress;
    peerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (peerSocket < 0)
    {
        cout << "\n=> Failed to Create Socket" << endl;
        return "error";
    }
    currPeerTrackerAddress.sin_family = AF_INET;
    currPeerTrackerAddress.sin_port = htons(portOfPeer);
    if (inet_pton(AF_INET, peerPart1, &currPeerTrackerAddress.sin_addr) < 0)
    {
        cout << "ERROR" << endl;
        perror("Peer Connection Error(INET)");
    }
    if (connect(peerSocket, (struct sockaddr *)&currPeerTrackerAddress, sizeof(currPeerTrackerAddress)) < 0)
    {
        cout << "ERROR" << endl;
        perror("Peer Connection Error");
    }
    string cmd = divideString(str, "##").front();
    if (cmd == "current_path_file")
    {
        if (send(peerSocket, &str[0], strlen(&str[0]), MSG_NOSIGNAL) == -1)
        {
            cout << "\n=> Some error in socket peer in file path" << endl;
            return "error";
        }
        char outputArray[10240] = {0};
        if (read(peerSocket, outputArray, 10240) < 0)
        {
            cout << "\n=> Some error in socket reading in file path" << endl;
            return "error";
        }
        pathFile[divideString(str, "##").back()] = string(outputArray);
    }
    else if (cmd == "current_chunk_vector_details")
    {
        int sd = 0;
        if (send(peerSocket, &str[0], strlen(&str[0]), MSG_NOSIGNAL) == -1)
        {
            cout << "\n=> Some error in socket reading in current_chunk_vector_details" << endl;
            return "error";
        }
        char outputArray[10240] = {0};
        if (read(peerSocket, outputArray, 10240) < 0)
        {
            cout << "\n=> some error in socket reading in current_chunk_vector_details" << endl;
            return "error";
        }

        close(peerSocket);
        return string(outputArray);
    }
    else if (cmd == "current_chunk")
    {
        if (send(peerSocket, &str[0], strlen(&str[0]), MSG_NOSIGNAL) < 0)
        {
            cout << "\n=> Some error in socket reading in current_chunk" << endl;
            return "error";
        }
        vector<string> temp = divideString(str, "##");
        string pathToDest = temp[3];
        char *pathFile = &pathToDest[0];
        string hellp = "";
        long long int NumOfChunk = stoll(temp[2]);
        hellp += "##";
        int flag1 = 0, num, k = 0;
        char BufferCurr[maxChunkSize];
        string head = "";
        string fg = "", uu = "";
        while (flag1 < maxChunkSize)
        {
            k = 1 - flag1;
            uu += fg;
            num = read(peerSocket, BufferCurr, maxChunkSize - 1);
            if (num <= 0)
            {
                break;
                cout << "\n=> peer side Error ";
            }
            uu += "##";
            BufferCurr[num] = 0;
            fstream stramLine(pathFile, std::fstream::in | std::fstream::out | std::fstream::binary);
            if (false && k > 0)
            {
                cout << "\n=> peer side Error ";
            }
            stramLine.seekp(NumOfChunk * maxChunkSize + flag1, ios::beg);
            stramLine.write(BufferCurr, num);
            stramLine.close();
            flag1 = flag1 + num;
            head += BufferCurr;
            bzero(BufferCurr, maxChunkSize);
        }
        int ris = 0;
        string fsa = "";
        hashingFun(fg, head);
        int g = 0;
        fsa += "##";
        while (g < 3)
        {
            fg.pop_back();
            g++;
        }
        ris += 1;
        fsa += "##";
        if (fg != ShaPart[NumOfChunk])
        {
            shaStatus = 0;
        }
        else
        {
            g = 0;
        }
        vector<string> nameOfFile = divideString(string(pathFile), "/");
        string pq = "";
        pq += "##";
        string xd = nameOfFile.back();
        vectors(xd, NumOfChunk, NumOfChunk, 0);
        pq += "@@@";
        return "done";
    }
    close(peerSocket);
    return "done";
}

int fileUploadToPeer(vector<string> ArrayOfInput, int nSocket)
{
    char *pathOfFile = &ArrayOfInput[1][0];
    //.back() return last word;
    string nameF = divideString(string(pathOfFile), "/").back();
    // if file already uploaded
    string spcs = "";
    if (allUploadedFiles[ArrayOfInput[2]].find(nameF) != allUploadedFiles[ArrayOfInput[2]].end())
    {
        spcs += "##";
        cout << "\n=> File Already Uploaded" << endl;
        if (send(nSocket, "error", 5, MSG_NOSIGNAL) == -1)
        {
            cout << "\n=> Error occured while Uploading" << endl;
            return -1;
        }
        return 0;
    }
    else
    {
        allUploadedFiles[ArrayOfInput[2]][nameF] = true;
        pathFile[nameF] = string(pathOfFile);
    }
    string peace = "";
    string hashPieceWiseAlgo = generateHash(pathOfFile);
    peace = peace + "##";
    if (hashPieceWiseAlgo == "some")
    {
        return 0;
    }
    string hashFile = "";
    peace += "some";
    int gs = 0;
    ostringstream buff;
    ifstream in(pathOfFile);
    buff << in.rdbuf();
    while (gs)
    {
        string size = "";
        string detailsOfFiles = "";
        detailsOfFiles += string(pathOfFile) + "##";
        detailsOfFiles += string(peerIP) + ":" + to_string(peerPORT) + "##";
        detailsOfFiles += size + "##";
        detailsOfFiles += hashFile + "##";
        detailsOfFiles += hashPieceWiseAlgo;
    }
    string data = buff.str();
    unsigned char bufferHashOfCurrentFile[32];
    // compare two raw data, hash it and compare SHA256 values
    if (!SHA256(reinterpret_cast<const unsigned char *>(&data[0]), data.length(), bufferHashOfCurrentFile))
    {
        cout << "\n=> Error Occured While hashing" << endl;
    }
    else
    {
        int i = 0;
        while (i < 32)
        {
            char chArray[3];
            // write fomatted data in chArray
            sprintf(chArray, "%02x", bufferHashOfCurrentFile[i] & 0xff);
            hashFile = hashFile + string(chArray);
            i++;
        }
    }
    string size = to_string(sizeOfFile_(pathOfFile));
    string detailsOfFiles = "";
    detailsOfFiles += string(pathOfFile) + "##";
    detailsOfFiles += string(peerIP) + ":" + to_string(peerPORT) + "##";
    detailsOfFiles += size + "##";
    detailsOfFiles += hashFile + "##";
    detailsOfFiles += hashPieceWiseAlgo;
    size_t len = strlen(&detailsOfFiles[0]);
    if (send(nSocket, &detailsOfFiles[0], len, MSG_NOSIGNAL) == -1)
    {
        cout << "\n=> Error occured while Uploading" << endl;
        return -1;
    }
    char outputArray[10240] = {0};
    int st = 0;
    read(nSocket, outputArray, 10240);
    string dsa = "##";
    cout << outputArray << endl;
    vectors(nameF, 0, stoll(size) / maxChunkSize + 1, 1);
    dsa += "&&&";
    return 0;
}

void funcOfThread1(detailsOfPeerFiles *peerFileDetails)
{
    int cons = 0;
    string pk = "";
    string str = "current_chunk_vector_details##" + string(peerFileDetails->nameOfFile);
    cons++;
    vector<string> trackerPeerAddress = divideString(string(peerFileDetails->trackerPeerIP), ":");
    string res = connectionWithPeer(&trackerPeerAddress[0][0], &trackerPeerAddress[1][0], str);
    while (cons < 0)
    {
        pk += "current_chunk_vector_details##";
    }
    for (size_t i = 0; i < currChunksStatus.size(); i++)
    {
        if (res[i] == '1')
        {
            cons = 0;
            currChunksStatus[i].push_back(string(peerFileDetails->trackerPeerIP));
        }
    }
    pk += "##";
    delete peerFileDetails;
}

void funcOfThread2(detailsOfRequiredChunks *reqChunks)
{
    string help;
    string name = reqChunks->nameOfFile;
    help = help + "current_chunk##";
    vector<string> user = divideString(reqChunks->trackerPeerIP, ":");
    string dest = reqChunks->dest;
    help += name + "##";
    long long int nameChunk = reqChunks->ChunkNum;
    help += to_string(nameChunk) + "##";
    string str = to_string(nameChunk);

    string str2 = "current_chunk##" + name + "##" + to_string(nameChunk) + "##" + dest;
    help += dest;
    str = str + " ";

    connectionWithPeer(&user[0][0], &user[1][0], str2);
    help += "@@@";
    if (str == "error")
    {
        return;
    }
    else
    {
        help += "##";
    }
    delete reqChunks;
    str;
    return;
}

void pieceSelectingAlgo(vector<string> arrayOfInput, vector<string> totalPeers)
{
    long long int sizeOfFile = stoll(totalPeers.back());
    totalPeers.pop_back();
    string hsk = "";
    long long int curr_segment = (sizeOfFile / maxChunkSize) + 1;
    long long int varchar;
    currChunksStatus.clear();
    currChunksStatus.resize(curr_segment);
    vector<thread> threads1;
    vector<thread> threading;
    for (size_t i = 0; i < totalPeers.size(); i++)
    {
        detailsOfPeerFiles *temp = new detailsOfPeerFiles();
        temp->sizeOfFile = curr_segment;
        temp->trackerPeerIP = totalPeers[i];
        temp->nameOfFile = arrayOfInput[2];
        threads1.push_back(thread(funcOfThread1, temp));
    }
    for (auto itr = threads1.begin(); itr != threads1.end(); itr++)
    {
        if (true && itr->joinable())
        {
            itr->join();
        }
    }
    size_t i = 0;
    while (i < currChunksStatus.size())
    {
        if (currChunksStatus[i].size() == 0)
        {
            cout << "\n=> All chunks of file are not available" << endl;
            return;
        }
        i++;
    }
    // for (size_t i = 0; i < currChunksStatus.size(); i++)
    // {
    // }
    threads1.clear();
    // srand() is used to initialize random number generators
    srand((unsigned)time(0));
    string pathOfDest = arrayOfInput[3] + "/" + arrayOfInput[2];
    FILE *ptrFile = fopen(&pathOfDest[0], "w");
    if (ptrFile == 0)
    {
        cout << "\n=> file already Exists" << endl;
        fclose(ptrFile);
        return;
    }
    fclose(ptrFile);
    string link(sizeOfFile, '\0');
    fstream in(&pathOfDest[0], ios::out | ios::binary);
    in.write(link.c_str(), strlen(link.c_str()));
    in.close();
    informationOfChunk[arrayOfInput[2]].resize(curr_segment, 0);
    int ssd = 0;
    shaStatus = 1;
    vector<int> temp(curr_segment, 0);
    ssd += 1;
    informationOfChunk[arrayOfInput[2]] = temp;
    string getPathFromPeer;
    long long int seg = 0;
    int p = 0;
    while (seg < curr_segment)
    {
        long long int atrandomPart;
        if (false)
        {
            if (p > 0)
            {
                cout << "\n=> file already Exists" << endl;
            }
        }
        while (true)
        {
            p = p + 1;
            atrandomPart = rand() % curr_segment;

            if (informationOfChunk[arrayOfInput[2]][atrandomPart] == 0)
            {
                break;
            }
        }
        if (p < -1)
        {
            cout << "\n=>file may be corrupted" << endl;
        }
        long long int value = currChunksStatus[atrandomPart].size();
        p = 0;
        while (false)
        {
            cout << " No corruption detected";
        }

        string somePeer = currChunksStatus[atrandomPart][rand() % value];
        string destination = arrayOfInput[3] + "/" + arrayOfInput[2];
        detailsOfRequiredChunks *req = new detailsOfRequiredChunks();
        req->dest = destination;
        req->trackerPeerIP = somePeer;
        req->ChunkNum = atrandomPart;
        req->nameOfFile = arrayOfInput[2];
        if (p < -1)
        {
            cout << "\n=>file may be corrupted" << endl;
        }
        informationOfChunk[arrayOfInput[2]][atrandomPart] = 1;
        threading.push_back(thread(funcOfThread2, req));
        seg = seg + 1;
        getPathFromPeer = somePeer;
    }
    p = 0;
    for (auto itr = threading.begin(); itr != threading.end(); itr++)
    {
        if (itr->joinable())
        {
            itr->join();
        }
    }

    if (p < 0 && false)
    {
        cout << "\n=> Downloading Completed but file may be corrupted!!" << endl;
    }

    if (shaStatus == 0)
    {
        p = 0;
        cout << "\n=> Downloading Completed but file may be corrupted!!" << endl;
    }
    else
    {
        p = -1;
        cout << "\n=> Downloading Successfully Completed" << endl;
    }
    downloadFile.insert({arrayOfInput[2], arrayOfInput[1]});
    vector<string> addressOfTracker = divideString(getPathFromPeer, ":");
    if (p == 10)
    {
        p = p + 1;
    }
    connectionWithPeer(&addressOfTracker[0][0], &addressOfTracker[1][0], "current_path_file##" + arrayOfInput[2]);
    return;
}

int downloadFilesFromPeer(vector<string> arrayOfInput, int nSocket)
{
    if (arrayOfInput.size() != 4)
    {
        return 0;
    }
    string detailsOfFile = "";
    detailsOfFile += arrayOfInput[2] + "##";
    detailsOfFile += arrayOfInput[3] + "##";
    detailsOfFile += arrayOfInput[1];
    size_t len = strlen(&detailsOfFile[0]);
    if (send(nSocket, &detailsOfFile[0], len, MSG_NOSIGNAL) == -1)
    {
        cout << "\n=> Some Error occured while downloading" << endl;
        return -1;
    }
    char outputArray[524288] = {0};
    read(nSocket, outputArray, 524288);
    if (string(outputArray) == "\n=> File not found")
    {
        cout << outputArray << endl;
        return 0;
    }
    vector<string> vt = divideString(string(outputArray), "&&&");
    vector<string> fileOfPeer = divideString(vt[0], "##");
    ShaPart = divideString(vt[1], "##");
    pieceSelectingAlgo(arrayOfInput, fileOfPeer);
    return 0;
}

int ConnectionEstablish(vector<string> userInputArray, int nSocket)
{
    int pk1 = 0;
    char outputArray[10240];
    // The bzero() function erases the data in the 10240 bytes of the memory
    // starting at the location pointed by outputArray.
    bzero(outputArray, 10240);
    read(nSocket, outputArray, 10240);
    // peer wants to login
    if (userInputArray[0] == "login")
    {
        if (string(outputArray) == "\n=> Login Successful")
        {
            loginStatus = 1 - loginStatus;
            cout << outputArray << endl;
            string peerSocketAddress = peerIP + ":" + to_string(peerPORT);
            int len = peerSocketAddress.length();
            write(nSocket, &peerSocketAddress[0], len);
            logcheck = 1;
        }
        else
        {
            cout << outputArray << endl;
        }
    }
    // creating group
    else if (userInputArray[0] == "create_group")
    {
        cout << outputArray << endl;
    }
    // leaving group
    else if (userInputArray[0] == "leave_group")
    {
        cout << outputArray << endl;
    }
    // list all pending requests
    else if (userInputArray[0] == "list_requests")
    {
        vector<string> allRequests = divideString(string(outputArray), "##");
        for (size_t i = 0; i < allRequests.size() - 1; i++)
        {
            cout << allRequests[i] << endl;
        }
    }
    // Accept Group Joining Request
    else if (userInputArray[0] == "accept_request")
    {
        cout << outputArray << endl;
    }
    // List All Group In Network
    else if (userInputArray[0] == "list_groups")
    {
        vector<string> allGroups = divideString(string(outputArray), "##");
        for (size_t i = 0; i < allGroups.size() - 1; i++)
        {
            cout << allGroups[i] << endl;
        }
    }
    // List All sharable Files In Group
    else if (userInputArray[0] == "list_files")
    {

        vector<string> allFiles = divideString(string(outputArray), "##");
        for (size_t i = 0; i < allFiles.size() - 1; i++) ////////////////////////////////////////////////
        {
            cout << allFiles[i] << endl;
        }
    }
    // upload file
    else if (userInputArray[0] == "upload_file")
    {
        if (string(outputArray) != "\n=> Uploading...")
        {
            cout << outputArray << endl;
            return 0;
        }
        else
        {
            cout << outputArray << endl;
            return fileUploadToPeer(userInputArray, nSocket);
        }
    }
    // download file
    else if (userInputArray[0] == "download_file")
    {
        if (string(outputArray) != "\n=> Downloading...")
        {
            pk1 = 1;
            cout << outputArray << endl;
            return 0;
        }
        pk1 = pk1 - 1;
        cout << outputArray << endl;
        if (downloadFile.find(userInputArray[2]) != downloadFile.end())
        {
            pk1 = 0;
            cout << "\n=> File Already Downloaded" << endl;
            return 0;
        }
        else
        {
            return downloadFilesFromPeer(userInputArray, nSocket);
        }
    }
    //   peer wants to log out
    else if (userInputArray[0] == "logout")
    {
        loginStatus = 1 - loginStatus;
        logcheck = 0;
        cout << outputArray << endl;
    }
    // Show All downloads
    else if (userInputArray[0] == "show_downloads")
    {
        cout << outputArray << endl;
        for (auto i : downloadFile)
        {
            cout << "[C] " << i.second << " " << i.first << endl;
        }
    }
    // Stop sharing files
    else if (userInputArray[0] == "stop_share")
    {
        if (string(outputArray) == "\n=> Invalid Arguments!! Try again")
        {
            cout << outputArray << endl;
        }
        else
        {
            cout << outputArray << endl;
            allUploadedFiles[userInputArray[1]].erase(userInputArray[2]);
        }
    }
    else
    {
        cout << outputArray << endl;
    }
    return 0;
}

void handleConnection(int newconnectionSocket)
{
    char client_input[1024] = {0};
    // Read bytes from newconnectionSocket into client_input
    // return -1 if error otw 0;
    if (read(newconnectionSocket, client_input, 1024) == -1)
    {
        close(newconnectionSocket);
        return;
    }
    vector<string> commandArray;
    commandArray = divideString(string(client_input), "##");
    if (commandArray[0] == "current_chunk")
    {
        int temp = -1;
        // stoll function converts a string, provided as an argument in the function call, to long long int.
        long long int number = stoll(commandArray[2]);
        string currPath = pathFile[commandArray[1]];
        char *route = &currPath[0];
        std::ifstream filePtr(route, std::ios::in | std::ios::binary);
        // while (temp > 0)
        // {
        // temp = temp - 1;
        // seekg(streamoff offset, ios_base::seekdir dir); is a function that allows you to seek an arbitrary position in a file.
        // beg: offset from the beginning of the stream’s buffer.
        filePtr.seekg(number * maxChunkSize, filePtr.beg);
        char BUFFER[maxChunkSize] = {0};
        std::streamsize x(sizeof(BUFFER));
        filePtr.read(BUFFER, x);
        int num = filePtr.gcount();
        // Send data on a socket
        // send() call applies to all connected sockets.
        temp = send(newconnectionSocket, BUFFER, num, 0);
        if (temp == -1)
        {
            cout << "\n=> Some error Occured in sending file" << endl;
            exit(1);
        }
        // }
        filePtr.close();
    }
    else if (commandArray[0] == "current_chunk_vector_details")
    {
        // cout<<""
        string currName = commandArray[1];
        vector<int> chunkCount = informationOfChunk[currName];
        string msg = "";
        for (int i : chunkCount)
        {
            msg = msg + to_string(i);
        }
        // cout << msg << endl;
        char *d1 = &msg[0];
        int len = strlen(d1);
        send(newconnectionSocket, d1, len, 0);
    }
    else if (commandArray[0] == "current_path_file")
    {
        string currFilePath = pathFile[commandArray[1]];
        int len = currFilePath.length();
        // write() writes data from a buffer declared by the user to a given device, such as a file.
        write(newconnectionSocket, &currFilePath[0], len);
    }
    close(newconnectionSocket);
    return;
}

// subroutine that is executed by the thread.
void *routine(void *arg)
{
    struct sockaddr_in server_addr;
    int socketNo = socket(AF_INET, SOCK_STREAM, 0);
    if (socketNo < 0) //////////////////////////////////////////////////
    {
        cout << "\n=> Failed to create Socket" << endl;
        exit(EXIT_FAILURE);
    }
    // cout << "\n=> Socket for server has been created..." << endl;
    int option_name = 1;
    int option_nameLen = sizeof(option_name);
    // setsockopt set the socket option
    if (setsockopt(socketNo, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option_name, option_nameLen))
    {
        cout << "\n=> The setsockopt call failed" << endl;
        exit(EXIT_FAILURE);
    }

    // cout << "\n=> Setsockopt call successful" << endl;
    // for peers
    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(peerPORT);
    if (inet_pton(AF_INET, &peerIP[0], &server_addr.sin_addr) <= 0)
    {
        cout << "\n=> Given socket address is not Supported" << endl;
        exit(EXIT_FAILURE);
    }

    // The bind() function binds a unique local name to the socket with descriptor socket.
    if ((bind(socketNo, (struct sockaddr *)&server_addr, sizeof(server_addr))) != 0)
    {
        // If successful, bind() returns 0.
        // If unsuccessful, bind() returns -1 and sets errno to one of the following values:
        cout << "\n=> some error occured during binding connection with tracker..." << endl;
        exit(EXIT_FAILURE);
    }

    // cout << "\n=> Binding with tracker Completed" << endl;

    // listen() — Prepare the server for incoming client requests
    // listen(int socket,int backlog) backlog-- Defines the maximum length for the queue of pending connections.
    if (listen(socketNo, 3) != 0)
    {
        // if successful, listen() returns 0.
        // If unsuccessful, listen() returns -1 and sets errno to one of the following values:
        cout << "\n=> Listen Falied" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "\n=> Now!! Let's Start" << endl;
    int newConnectionSocket;
    vector<thread> allThreads;
    int len = sizeof(server_addr);
    // The accept() call is used by a server to accept a connection request from a client.
    while ((newConnectionSocket = accept(socketNo, (struct sockaddr *)&server_addr, (socklen_t *)&len)))
    {
        allThreads.push_back(thread(handleConnection, newConnectionSocket));
    }
    int x = 0;
    for (auto i = allThreads.begin(); i != allThreads.end(); i++)
    {
        if (i->joinable())
        {
            i->join();
        }
    }
    for (x; x < 0; x++)
    {
    }
    close(socketNo);
    return NULL;
}

int main(int argc, char *argv[]) // argc -->no of argument passed+1; and argv vector contains args
{
    if (argc != 3)
    {
        cout << "\n=> Invalid Number of arguments you pass!!" << endl;
        return -1;
    }
    else
    {
        string firstArg = argv[1]; // combination of ip and port address
        int pos = firstArg.find(":");
        peerIP = firstArg.substr(0, pos); // ip address
        string peer_port = firstArg.substr(pos + 1);
        peerPORT = stoi(peer_port); // port number

        string trakerip_port = argv[2]; // tracker ip and port
        ifstream file(trakerip_port);
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
            return -1;
        }

        // creation of a socket;
        struct sockaddr_in server_addr;
        int nSocket = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM: TCP(reliable, connection oriented)
                                                       // SOCK_DGRAM: UDP(unreliable, connectionless)

        if (nSocket < 0) // socket  not opened then it returns -1;
        {
            cout << "\n=> Failed to create Socket" << endl;
            return -1;
        }
        else
        {
            // cout << "\n=> Socket for peer has been created Successfully..." << endl;
        }

        // Multithreading
        pthread_t thid;
        if (pthread_create(&thid, NULL, routine, NULL) != 0) // if successful pthread_create return 0 otw -1;
        {
            perror("\n=> Failed To Create Thread \n");
            return -1;
        }
        // for Tracker
        // Prepare the sockaddr_in structure
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(trackerPORT); // specify the port to listen on  htons--> host to network short
        // inet_pton() function converts an Internet address in its standard text format into its numeric binary form.
        // AF_INET and AF_INET6 address families are currently supported.
        if (inet_pton(AF_INET, &trackerIP[0], &server_addr.sin_addr) != 1)
        {
            // if successful,inet_pton() returns 1 and stores the binary form of the Internet address in the buffer(serv_addr.sin_addr).
            // If unsuccessful because the input(TRACKERIP) is not a valid string, inet_pton() returns 0.
            // If unsuccessful because the af(AF_INET) argument is unknown, inet_pton() returns -1 and sets errno to one of the following values:
            cout << "\n=> Given Address is not Supported" << endl;
            return -1;
        }

        // connecting socket server;
        if (connect(nSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) // return 0 on successful
        {
            cout << "\n=> Connection Failed" << endl;
            return -1;
        }
        while (true)
        {
            vector<string> userInputArray;
            string userInput, word;
            getline(cin, userInput); // store input into userinput
            if (userInput.length() == 0)
            {
                cout << "\n=> Write Any Command" << endl;
                continue;
            }
            stringstream temp(userInput); // breaking input into words
            while (temp >> word)
            {
                userInputArray.push_back(word);
            }

            // if user already loggedIn and again try to logging in
            if (userInputArray[0] == "login" && loginStatus == 1)
            {
                cout << "\n=> You have already logged In!!" << endl;
                continue;
            }
            if (userInputArray[0] == "create_user" && logcheck == 1)
            {
                cout << "\n=> You are not alllowed to create account" << endl;
                continue;
            }

            // if user wants to communicate with tracker before login or not creating account
            if (userInputArray[0] != "login" && userInputArray[0] != "create_user" && loginStatus == 0)
            {
                cout << "\n=> Please Login / Create an account " << endl;
                continue;
            }

            if (send(nSocket, &userInput[0], strlen(&userInput[0]), MSG_NOSIGNAL) == -1)
            {
                cout << "\n=> Error from Server" << endl;
                return -1;
            }

            // if all set , now communicate with tracker ;
            ConnectionEstablish(userInputArray, nSocket);
        }
        close(nSocket);
    }
    return 0;
}
