# Client

## How to build (Mac & Linux)
1. Install dependencies
- ncurses (TUI)
- pthread (POSIX threads)
- [cpstd](https://github.com/ColinIndieDev/ColinDev-C-Projects/tree/main/cpstd) (my hashmap implementation)
> [!NOTE]
> Build cpstd to library with this comman inside the cpstd-Folder: \
> cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local && sudo cmake --install build
2. Use CMake to build project
3. Done :)

## How to use
### Preparation
1. Go to the folder where the executable is
2. Create a .txt file named `ip.txt`
3. Write inside `ip.txt` the public IP address of the server
4. Create another file named `chat_key.txt`
5. Write a key inside you want to use with the others to encrypt your usernames and messages (f.e. `12345` or `MyPassword!?123`)
### Usage
1. Execute program
2. Enter your username
3. If you followed the preparation above correctly and the program found and connected to the server you should see the chatroom interface
4. To write, just type inside the message box and enter to send, easy
5. Use available commands which you write inside the message box:
- `/id` -> show your ID only to you
- `/id_share` -> send a message of your id to everyone
- `/exit` -> exit the program and disconnect from the server
6. Leave with `/exit`
