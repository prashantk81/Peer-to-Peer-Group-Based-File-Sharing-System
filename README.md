###(Peer-to-Peer Group Based File Sharing System)

`GOAL`:- We built a group based file sharing system where users
can share, download files from the group they belong to.

`Note`:- we divide the whole file into 524288 Bytes logical pieces.

`Note`:- From 2022201058_a3 compile and run both the files

Client folder contain client.cpp and tracker folder contain tracker.cpp

## Compile and Run client and tracker file

### Tracker:

    Command to compile tracker : g++ tracker/tracker.cpp -o tracker/tracker -pthread

    Command to run tracker : ./tracker/tracker IPport.txt

    Close Tracker: quit

### Client:

    Command to compile client : g++ client/client.cpp -o client/client -pthread -lcrypto

    Command to run client : ./client/client <IP>:<PORT> IPport.txt

    (port>1024 and ip- loopback addresses)

`Note`:- IPport.txt file contains ip address and port address of tracker(server)

### Commands to run client:

    a. Create User Account: create_user <user_id> <password>

    b. Login: login <user_id> <password>

    c. Create Group: create_group <group_id>

    d. Join Group: join_group <group_id>

    e. Leave Group: leave_group <group_id>

    f. List pending join(requests): list_requests <group_id>

    g. Accept Group Joining Request: accept_request <group_id> <user_id>

    h. List All Group In Network: list_groups

    i. List All sharable Files In Group: list_files <group_id>

    j. Upload File: upload_file <file_path> <group_id>

    k. Download File: download_file <group_id> <file_name> <destination_path>

    l. Logout: logout

    m. Show_downloads: show_downloads

    o. Stop_sharing: stop_share <group_id> <file_name>

`Note`:- Support both absolute and relative path

:- For Absolute path => /home/prashant/Desktop/........

:- For Relative path => ../2022201058/........

### Log File

    I've also added a log file which will capture the logs of the tracker and also download .txt file in 2022201058_a3 directory
