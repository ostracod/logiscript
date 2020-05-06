
let fs = require("fs");
let net = require("net");
let pathUtils = require("path");
let childProcess = require("child_process");

if (process.argv.length !== 3) {
    console.log("Please provide a path to a LogiScript interpreter.");
    process.exit(1);
}

let interpreterPath = process.argv[2];
let socketPath = pathUtils.join(__dirname, "testSocket");

let receivedSocketData = Buffer.alloc(0);
let interpreterClient = null;

if (fs.existsSync(socketPath)) {
    fs.unlinkSync(socketPath);
}

function handleReceivedPacket(data) {
    console.log(`Received packet: "${data.toString()}"`);
    sendPacket(Buffer.concat([Buffer.from("Very cool!"), Buffer.from([0])]));
}

function sendPacket(data) {
    console.log(`Sending packet: "${data.toString()}"`);
    let tempBuffer = Buffer.alloc(4);
    tempBuffer.writeInt32LE(data.length, 0);
    interpreterClient.write(tempBuffer);
    interpreterClient.write(data);
}

function socketReceiveEvent() {
    let contentOffset = 4;
    if (receivedSocketData.length < contentOffset) {
        return;
    }
    let tempLength = receivedSocketData.readInt32LE(0);
    let endIndex = contentOffset + tempLength;
    if (receivedSocketData.length < endIndex) {
        return;
    }
    let packetData = receivedSocketData.slice(contentOffset, endIndex);
    receivedSocketData = receivedSocketData.slice(endIndex, receivedSocketData.length);
    handleReceivedPacket(packetData);
}

let socketServer = net.createServer(client => {
    console.log("Interpreter connected to socket.");
    interpreterClient = client;
    
    client.on("data", data => {
        receivedSocketData = Buffer.concat([receivedSocketData, data]);
        socketReceiveEvent();
    });
    
    client.on("end", () => {
        console.log("Interpreter disconnected from socket.");
    });
});

socketServer.listen(socketPath, () => {
    console.log("Listening to socket.");
    
    console.log("Launching interpreter....");
    let interpreterProcess = childProcess.spawn(interpreterPath, ["--socket", socketPath]);
    
    interpreterProcess.stdout.on("data", data => {
        console.log(`Received stdout message: "${data.toString()}"`);
    });
    
    interpreterProcess.on("close", code => {
        console.log("Interpreter finished running.");
        socketServer.close(() => {
            console.log("Server finished running.");
        });
    });
});


