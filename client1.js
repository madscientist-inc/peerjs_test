const net = require('net');
const fs = require('fs');
const WebSocket = require('ws');


const socketPath = '/tmp/node_cpp_socket.sock';
let JsonData = {
            "call_stat": 0,
            "call_req": 0,
            "call_acc": 0,
            "seat_alm": 0,
            "signal": 0
        };
let responseJson = JSON.stringify(JsonData)

if (fs.existsSync(socketPath)) {
  fs.unlinkSync(socketPath);
}

const server = net.createServer((socket) => {
  console.log('C++ クライアントが接続しました。');

  socket.on('data', (data) => {
    try {
      const receivedData = JSON.parse(data.toString());
      console.log('C++側から受信したデータ:', receivedData);

    //   const responseData = { message: 'Hello from Node.js' };
      const responseData = receivedData;
      responseJson = JSON.stringify(responseData);

      socket.write(responseJson);
    } catch (error) {
      console.error('データのパースに失敗しました:', error.message);
    }
  });

  socket.on('close', () => {
    console.log('クライアントが切断されました。');
  });

  socket.on('error', (err) => {
    console.error('エラーが発生しました:', err.message);
  });
});

server.listen(socketPath, () => {
  console.log(`Node.js サーバーが ${socketPath} で待機中...`);
});

//websocketServer側
const wss = new WebSocket.Server({ port:5002 });
wss.on('connection', (ws) => {
    // WebSocket接続が確立されたときの処理
    console.log('WebSocket connected');
  
    // サンプルJSONデータ
    // const jsonData = { message: 'Hello from the Nodejs server!' };
    // const jsonData = responseData;
  
    // JSONデータを文字列に変換してWebSocketへ送信
    // ws.send(JSON.stringify(jsonData));
    // ws.send(JSON.stringify(responseData));
    ws.send(responseJson)//サーバーから一発目送信
  // WebSocketからメッセージを受信したときの処理
  ws.on('message', (message) => {
    // 受信したJSONデータをパース
    const receivedData = JSON.parse(message);
    
    // データを加工または変更
    receivedData.message = 'Server modified: ' + receivedData.message;

    // 加工したデータを文字列に変換してクライアントに送り返す
    // ws.send(JSON.stringify(receivedData));
    setTimeout(() => {
      ws.send(responseJson);
    }, 100);
  });
});