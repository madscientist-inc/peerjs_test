#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <csignal>
#include <thread>
#include <chrono>

using json = nlohmann::json;

bool keepRunning = true;

// 受信したJSONデータを保持する変数
json receivedData;
// 送信するJSONデータの作成
json sendData = {
    // {"message", "Hello from C++"},
    // {"key", "new_value"},  // データを変更
    // 他のデータフィールドを追加できます
    {"call_stat", 0},
    {"call_req", 0},
    {"call_acc", 0},
    {"seat_alm", 0},
    {"signal", 0}
};

// Ctrl+C でプログラムを終了するためのシグナルハンドラ
void handleSignal(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+Cが押されました。プログラムを終了します。\n";
        keepRunning = false;
    }
}

// 新しいスレッドで実行される関数
void threadFunction() {
    const char *socketPath = "/tmp/node_cpp_socket.sock";

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("ソケットの作成に失敗しました");
        return;
    }

    sockaddr_un serverAddr{};
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, socketPath, sizeof(serverAddr.sun_path) - 1);

    if (connect(sockfd, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("サーバーへの接続に失敗しました");
        close(sockfd);
        return;
    }

    while (keepRunning) {
        static int cnt=0;
        // JSONデータを文字列に変換
        std::string jsonString = sendData.dump();

        // データをサーバーに送信
        if (write(sockfd, jsonString.c_str(), jsonString.length()) == -1) {
            perror("データの送信に失敗しました");
            close(sockfd);
            return;
        }

        char buffer[1024];
        ssize_t bytesRead = read(sockfd, buffer, sizeof(buffer) - 1);
        if (bytesRead == -1) {
            perror("データの受信に失敗しました");
            close(sockfd);
            return;
        }

        buffer[bytesRead] = '\0';
        std::cout << "サーバーからのレスポンス: " << buffer << std::endl;

        // 受信したJSONデータを保持
        receivedData = json::parse(buffer);

        // 受信したデータを次に送信するデータに使用（例：keyフィールドを変更）
        sendData = receivedData; 
        sendData["call_stat"] = cnt;
        sendData["seat_alm"] = cnt;
        // sendData[""] = cnt;

        // 一定の待機時間を挿入（必要に応じて調整）
        //sleep(2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cnt++;
    }

    close(sockfd);
}

int main() {
    // Ctrl+C でプログラムを終了するためのシグナルハンドラの設定
    signal(SIGINT, handleSignal);

    // 新しいスレッドを開始
    std::thread workerThread(threadFunction);

    // メインスレッドはここで待機（Ctrl+Cが押されるまで）
    workerThread.join();

    return 0;
}